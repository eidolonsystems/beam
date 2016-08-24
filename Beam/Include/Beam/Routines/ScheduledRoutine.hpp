#ifndef BEAM_SCHEDULEDROUTINE_HPP
#define BEAM_SCHEDULEDROUTINE_HPP
#include <iostream>
#ifdef _MSC_VER
  #include "Beam/Routines/ScheduledRoutineMsvc.hpp"
#else
  #include "Beam/Routines/ScheduledRoutinePosix.hpp"
#endif

namespace Beam {
namespace Routines {

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

  inline void ScheduledRoutine::PendingSuspend() {
    m_mutex.lock();
    assert(GetState() == State::RUNNING);
    SetState(State::PENDING_SUSPEND);
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

  inline void ScheduledRoutine::Bind(std::shared_ptr<ScheduledRoutine> self) {
    m_self = std::move(self);
  }
}
}

#endif
