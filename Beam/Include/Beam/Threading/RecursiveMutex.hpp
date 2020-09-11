#ifndef BEAM_RECURSIVE_MUTEX_HPP
#define BEAM_RECURSIVE_MUTEX_HPP
#include <cstdint>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Routines/Routine.hpp"
#include "Beam/Routines/SuspendedRoutineQueue.hpp"
#include "Beam/Threading/LockRelease.hpp"
#include "Beam/Threading/Threading.hpp"

namespace Beam::Threading {

  /** Implements a recursive_mutex that suspends the current Routine. */
  class RecursiveMutex {
    public:

      /** Constructs a RecursiveMutex. */
      RecursiveMutex();

      ~RecursiveMutex();

      /** Locks this Mutex. */
      void lock();

      /** Tries to locks this Mutex. */
      bool try_lock();

      /** Unlocks this Mutex. */
      void unlock();

    private:
      boost::mutex m_mutex;
      int m_counter;
      int m_depth;
      Routines::Routine* m_owner;
      Routines::SuspendedRoutineQueue m_suspendedRoutines;

      RecursiveMutex(const RecursiveMutex&) = delete;
      RecursiveMutex& operator =(const RecursiveMutex&) = delete;
  };

  inline RecursiveMutex::RecursiveMutex()
    : m_counter(0),
      m_depth(0),
      m_owner(nullptr) {}

  inline RecursiveMutex::~RecursiveMutex() {
    assert(m_counter == 0);
  }

  inline void RecursiveMutex::lock() {
    auto currentRoutine = Routines::SuspendedRoutineNode();
    auto lock = boost::unique_lock(m_mutex);
    ++m_counter;
    if(m_counter > 1) {
      if(currentRoutine.m_routine != m_owner) {
        m_suspendedRoutines.push_back(currentRoutine);
        currentRoutine.m_routine->PendingSuspend();
        auto release = Release(lock);
        Routines::Suspend();
      }
    }
    ++m_depth;
    m_owner = currentRoutine.m_routine;
  }

  inline bool RecursiveMutex::try_lock() {
    auto currentRoutine = &Routines::GetCurrentRoutine();
    auto lock = boost::lock_guard(m_mutex);
    ++m_counter;
    if(m_counter > 1) {
      if(currentRoutine != m_owner) {
        --m_counter;
        return false;
      }
    }
    ++m_depth;
    m_owner = currentRoutine;
    return true;
  }

  inline void RecursiveMutex::unlock() {
    auto lock = boost::lock_guard(m_mutex);
    --m_depth;
    if(m_depth == 0) {
      m_owner = nullptr;
    }
    --m_counter;
    if(m_counter > 0) {
      if(m_depth == 0) {
        if(m_suspendedRoutines.empty()) {
          return;
        }
        auto routine = m_suspendedRoutines.front().m_routine;
        m_suspendedRoutines.pop_front();
        Routines::Resume(routine);
      }
    }
  }
}

#endif
