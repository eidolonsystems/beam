#ifndef BEAM_SCHEDULEDROUTINEMSVC_HPP
#define BEAM_SCHEDULEDROUTINEMSVC_HPP
#include <windows.h>
#include "Beam/Routines/Routine.hpp"
#include "Beam/Routines/RoutineException.hpp"
#include "Beam/Routines/Routines.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Threading/LockRelease.hpp"
#include "Beam/Utilities/ReportException.hpp"
#include "Beam/Utilities/StackPrint.hpp"

namespace Beam {
namespace Routines {

  /*! \class ScheduledRoutine
      \brief A Routine that executes within a Scheduler.
   */
  class ScheduledRoutine : public Routine {
    public:

      //! Returns the Scheduler this Routine runs through.
      Details::Scheduler& GetScheduler() const;

      //! Continues execution of this Routine from its last defer point or from
      //! the beginning if it has not yet executed.
      void Continue();

      //! Completes execution of this Routine.
      void Complete();

      //! Waits for this Routine to complete execution.
      void Wait();

      //! Begins running this Routine.
      virtual void Execute() = 0;

      virtual void Defer() override;

      virtual void PendingSuspend() override;

      virtual void Suspend() override;

      virtual void Resume() override;

    protected:

      //! Constructs a ScheduledRoutine.
      /*!
        \param stackSize The size of the stack to allocate.
        \param scheduler The Scheduler this Routine will execute through.
      */
      ScheduledRoutine(std::size_t stackSize,
        RefType<Details::Scheduler> scheduler);

    private:
      friend class Details::Scheduler;
      mutable boost::mutex m_mutex;
      std::size_t m_stackSize;
      Details::Scheduler* m_scheduler;
      LPVOID m_parentFiber;
      LPVOID m_fiber;
      std::vector<Routine*> m_suspendedRoutines;
      std::shared_ptr<ScheduledRoutine> m_self;

      static void __stdcall Win32InitializeRoutine(LPVOID r);
      void Bind(std::shared_ptr<ScheduledRoutine> self);
  };

  inline void ScheduledRoutine::Continue() {
    Details::CurrentRoutineGlobal<void>::GetInstance() = this;
    if(GetState() == State::PENDING) {
      if(m_self == nullptr) {
        Details::CurrentRoutineGlobal<void>::GetInstance() = nullptr;
        throw RoutineException{"Routine aborted."};
      }
      SetState(State::RUNNING);
      m_fiber = CreateFiber(m_stackSize,
        ScheduledRoutine::Win32InitializeRoutine, this);
      if(m_fiber == nullptr) {
        auto errorCode = GetLastError();
        std::cout << errorCode << std::endl;
      }
      m_parentFiber = GetCurrentFiber();
      SwitchToFiber(m_fiber);
    } else {
      SetState(State::RUNNING);
      m_parentFiber = GetCurrentFiber();
      SwitchToFiber(m_fiber);
    }
    Details::CurrentRoutineGlobal<void>::GetInstance() = nullptr;
  }

  inline void ScheduledRoutine::Complete() {
    assert(GetState() == State::COMPLETE);
    for(auto& suspendedRoutine : m_suspendedRoutines) {
      suspendedRoutine->Resume();
    }
    DeleteFiber(m_fiber);
    auto self = std::move(m_self);
  }

  inline void ScheduledRoutine::Defer() {
    assert(GetState() == State::RUNNING || GetState() == State::COMPLETE);
    Details::CurrentRoutineGlobal<void>::GetInstance() = nullptr;
    SwitchToFiber(m_parentFiber);
  }

  inline void ScheduledRoutine::Suspend() {
    Details::CurrentRoutineGlobal<void>::GetInstance() = nullptr;
    assert(GetState() == State::PENDING_SUSPEND);
    SwitchToFiber(m_parentFiber);
  }

  inline ScheduledRoutine::ScheduledRoutine(std::size_t stackSize,
      RefType<Details::Scheduler> scheduler)
      : m_stackSize{stackSize},
        m_scheduler{scheduler.Get()} {}

  inline void ScheduledRoutine::Win32InitializeRoutine(LPVOID r) {
    auto routine = reinterpret_cast<ScheduledRoutine*>(r);
    try {
      routine->Execute();
    } catch(...) {
      std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
    }
    {
      boost::lock_guard<boost::mutex> lock{routine->m_mutex};
      routine->SetState(State::COMPLETE);
    }
    routine->Defer();
  }
}
}

#endif
