#ifndef BEAM_SCHEDULEDROUTINEPOSIX_HPP
#define BEAM_SCHEDULEDROUTINEPOSIX_HPP
#include <iostream>
#include <boost/context/detail/fcontext.hpp>
#include "Beam/Routines/Routine.hpp"
#include "Beam/Routines/RoutineException.hpp"
#include "Beam/Routines/Routines.hpp"
#include "Beam/Routines/simple_stack_allocator.hpp"
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
      Details::Scheduler* m_scheduler;
      std::size_t m_stackSize;
      void* m_stackPointer;
      boost::context::detail::fcontext_t m_parentContext;
      boost::context::detail::fcontext_t m_context;
      std::vector<Routine*> m_suspendedRoutines;
      std::shared_ptr<ScheduledRoutine> m_self;

      static void InitializeRoutine(boost::context::detail::transfer_t r);
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
      m_context = boost::context::detail::make_fcontext(m_stackPointer,
        m_stackSize, ScheduledRoutine::InitializeRoutine);
      m_context = boost::context::detail::jump_fcontext(m_context, this).fctx;
    } else {
      SetState(State::RUNNING);
      m_context = boost::context::detail::jump_fcontext(
        m_context, nullptr).fctx;
    }
    Details::CurrentRoutineGlobal<void>::GetInstance() = nullptr;
  }

  inline void ScheduledRoutine::Complete() {
    assert(GetState() == State::COMPLETE);
    for(auto& suspendedRoutine : m_suspendedRoutines) {
      suspendedRoutine->Resume();
    }
    boost::context::simple_stack_allocator<8 * 1024 * 1024, 64 * 1024, 1024>
      allocator;
    allocator.deallocate(m_stackPointer, m_stackSize);
    auto self = std::move(m_self);
  }

  inline void ScheduledRoutine::Defer() {
    assert(GetState() == State::RUNNING || GetState() == State::COMPLETE);
    Details::CurrentRoutineGlobal<void>::GetInstance() = nullptr;
#ifdef _DEBUG
//    m_stackPrint = CaptureStackPrint();
#endif
    m_parentContext = boost::context::detail::jump_fcontext(
      m_parentContext, nullptr).fctx;
  }

  inline void ScheduledRoutine::Suspend() {
    Details::CurrentRoutineGlobal<void>::GetInstance() = nullptr;
    SetState(State::PENDING_SUSPEND);
#ifdef _DEBUG
//    m_stackPrint = CaptureStackPrint();
#endif
    m_parentContext = boost::context::detail::jump_fcontext(
      m_parentContext, nullptr).fctx;
  }

  inline ScheduledRoutine::ScheduledRoutine(std::size_t stackSize,
      RefType<Details::Scheduler> scheduler)
      : m_stackSize{stackSize},
        m_scheduler{scheduler.Get()} {
    boost::context::simple_stack_allocator<8 * 1024 * 1024, 64 * 1024, 1024>
      allocator;
    m_stackPointer = allocator.allocate(m_stackSize);
  }

  inline void ScheduledRoutine::InitializeRoutine(
      boost::context::detail::transfer_t r) {
    auto routine = reinterpret_cast<ScheduledRoutine*>(r.data);
    routine->m_parentContext = r.fctx;
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
