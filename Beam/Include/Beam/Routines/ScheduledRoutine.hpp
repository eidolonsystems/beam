#ifndef BEAM_SCHEDULED_ROUTINE_HPP
#define BEAM_SCHEDULED_ROUTINE_HPP
#include <iostream>
#if defined _MSC_VER
#define BEAM_DISABLE_OPTIMIZATIONS __pragma(optimize( "", off ))
#define other beam_other
#define BOOST_USE_WINFIB
#include <boost/context/continuation.hpp>
#undef BOOST_USE_WINFIB
#undef other
#else
#define BEAM_DISABLE_OPTIMIZATIONS
#include <boost/context/continuation.hpp>
#endif
#include "Beam/Routines/Routine.hpp"
#include "Beam/Routines/RoutineException.hpp"
#include "Beam/Routines/Routines.hpp"
#include "Beam/Pointers/Ref.hpp"
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

      virtual void Defer() override final;

      virtual void PendingSuspend() override final;

      virtual void Suspend() override final;

      virtual void Resume() override final;

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
      Details::Scheduler* m_scheduler;
      boost::context::continuation m_continuation;
      boost::context::continuation m_parent;
      std::vector<Routine*> m_suspendedRoutines;
      std::shared_ptr<ScheduledRoutine> m_self;

      boost::context::continuation InitializeRoutine(
        boost::context::continuation&& parent);
      void Bind(std::shared_ptr<ScheduledRoutine> self);
  };

  //! Waits for a Routine to complete.
  /*!
    \param id The id of the Routine to wait for.
  */
  inline void Wait(const Routine::Id& id) {
    auto routine = id.GetRoutine();
    if(routine != nullptr) {
      routine->Wait();
    }
  }

  inline Details::Scheduler& ScheduledRoutine::GetScheduler() const {
    return *m_scheduler;
  }

  inline void ScheduledRoutine::Continue() {
    Details::CurrentRoutineGlobal<void>::GetInstance() = this;
    if(GetState() == State::PENDING) {
      if(m_self == nullptr) {
        Details::CurrentRoutineGlobal<void>::GetInstance() = nullptr;
        throw RoutineException{"Routine aborted."};
      }
      SetState(State::RUNNING);
      m_continuation = boost::context::callcc(
        [=] (boost::context::continuation&& parent) {
          return InitializeRoutine(std::move(parent));
        });
    } else {
      SetState(State::RUNNING);
      m_continuation = m_continuation.resume();
    }
    Details::CurrentRoutineGlobal<void>::GetInstance() = nullptr;
  }

  inline void ScheduledRoutine::Complete() {
    assert(GetState() == State::COMPLETE);
    for(auto& suspendedRoutine : m_suspendedRoutines) {
      suspendedRoutine->Resume();
    }
    auto self = std::move(m_self);
  }

  inline void ScheduledRoutine::Wait() {
    auto currentRoutine = &GetCurrentRoutine();
    boost::unique_lock<boost::mutex> lock{m_mutex};
    if(GetState() == State::COMPLETE) {
      return;
    }
    m_suspendedRoutines.push_back(currentRoutine);
    while(GetState() != State::COMPLETE) {
      currentRoutine->PendingSuspend();
      auto release = Threading::Release(lock);
      currentRoutine->Suspend();
    }
  }

  BEAM_DISABLE_OPTIMIZATIONS
  inline void ScheduledRoutine::Defer() {
    assert(GetState() == State::RUNNING || GetState() == State::COMPLETE);
    Details::CurrentRoutineGlobal<void>::GetInstance() = nullptr;
    #ifdef BEAM_ENABLE_STACK_PRINT
    #ifndef NDEBUG
    m_stackPrint = CaptureStackPrint();
    #endif
    #endif
    m_parent = m_parent.resume();
  }

  inline void ScheduledRoutine::PendingSuspend() {
    m_mutex.lock();
    assert(GetState() == State::RUNNING);
    SetState(State::PENDING_SUSPEND);
  }

  inline void ScheduledRoutine::Suspend() {
    Details::CurrentRoutineGlobal<void>::GetInstance() = nullptr;
    SetState(State::PENDING_SUSPEND);
    #ifdef BEAM_ENABLE_STACK_PRINT
    #ifndef NDEBUG
    m_stackPrint = CaptureStackPrint();
    #endif
    #endif
    m_parent = m_parent.resume();
  }

  inline ScheduledRoutine::ScheduledRoutine(std::size_t stackSize,
      RefType<Details::Scheduler> scheduler)
      : m_scheduler{scheduler.Get()} {}

  inline boost::context::continuation ScheduledRoutine::InitializeRoutine(
      boost::context::continuation&& parent) {
    m_parent = std::move(parent);
    try {
      Execute();
    } catch(const boost::context::detail::forced_unwind&) {
      throw;
    } catch(...) {
      std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
    }
    boost::lock_guard<boost::mutex> lock{m_mutex};
    SetState(State::COMPLETE);
    return std::move(m_parent);
  }

  inline void ScheduledRoutine::Bind(std::shared_ptr<ScheduledRoutine> self) {
    m_self = std::move(self);
  }
}
}

#endif
