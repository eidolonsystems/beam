#ifndef BEAM_SCHEDULER_HPP
#define BEAM_SCHEDULER_HPP
#include <boost/atomic/atomic.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include "Beam/Routines/FunctionRoutine.hpp"
#include "Beam/Routines/Routines.hpp"
#include "Beam/Routines/TerminateRoutine.hpp"
#include "Beam/Utilities/Singleton.hpp"

namespace Beam {
namespace Routines {
namespace Details {

  /*! \class Scheduler
      \brief Schedules the execution of Routines across multiple threads.
   */
  class Scheduler : public Singleton<Scheduler> {
    public:

      //! The default size of a Routine's stack.
      static const std::size_t DEFAULT_STACK_SIZE = 64 * 1024;

      //! Constructs a Scheduler with a number of threads equal to the system's
      //! concurrency.
      Scheduler();

      ~Scheduler();

      //! Spawns a Routine from a callable object.
      /*!
        \param f The callable object to run within the Routine.
        \return A unique ID used to identify the Routine.
      */
      template<typename F>
      Routine::Id Spawn(F&& f);

      //! Spawns a Routine from a callable object.
      /*!
        \param f The callable object to run within the Routine.
        \param stackSize The size of the stack to allocate for the Routine.
        \return A unique ID used to identify the Routine.
      */
      template<typename F>
      Routine::Id Spawn(F&& f, std::size_t stackSize);

      //! Waits for any currently executing Routines to COMPLETE and stops
      //! executing any new ones.
      void Stop();

    private:
      friend class Beam::Routines::ScheduledRoutine;
      mutable boost::mutex m_mutex;
      std::vector<boost::thread> m_threads;
      boost::atomic_uint64_t m_nextId;
      boost::lockfree::queue<ScheduledRoutine*> m_pendingRoutines;
      mutable boost::condition_variable m_pendingRoutinesAvailableCondition;

      void Queue(ScheduledRoutine& routine);
      void Suspend(ScheduledRoutine& routine);
      void Run();
  };

  inline Scheduler::Scheduler()
      : m_nextId{0},
        m_pendingRoutines{boost::thread::hardware_concurrency()} {
    for(std::size_t i = 0; i < boost::thread::hardware_concurrency(); ++i) {
      m_threads.emplace_back(
        [=] {
#ifdef _MSC_VER
          auto scheduler = ConvertThreadToFiber(nullptr);
#endif
          Run();
        });
    }
  }

  inline Scheduler::~Scheduler() {
    Stop();
  }

  template<typename F>
  Routine::Id Scheduler::Spawn(F&& f) {
    return Spawn(std::forward<F>(f), DEFAULT_STACK_SIZE);
  }

  template<typename F>
  Routine::Id Scheduler::Spawn(F&& f, std::size_t stackSize) {
    auto routine = std::make_shared<FunctionRoutine<F>>(std::forward<F>(f),
      stackSize, Ref(*this));
    routine->Bind(routine);
    Queue(*routine);
    Routine::Id id{++m_nextId, std::move(routine)};
    return id;
  }

  inline void Scheduler::Queue(ScheduledRoutine& routine) {
    m_pendingRoutines.push(&routine);
    m_pendingRoutinesAvailableCondition.notify_one();
  }

  inline void Scheduler::Suspend(ScheduledRoutine& routine) {
    boost::lock_guard<boost::mutex> lock{routine.m_mutex, boost::adopt_lock};
    routine.SetState(Routine::State::SUSPENDED);
  }

  inline void Scheduler::Stop() {
    TerminateRoutine terminateRoutine{Ref(*this)};
    Queue(terminateRoutine);
    for(auto& thread : m_threads) {
      thread.join();
    }
  }

  inline void Scheduler::Run() {
    ScheduledRoutine* routine;
    auto& pendingRoutines = m_pendingRoutines;
    auto& mutex = m_mutex;
    auto& pendingRoutinesAvailableCondition =
      m_pendingRoutinesAvailableCondition;
    try {
      while(true) {
        if(!pendingRoutines.pop(routine)) {
          boost::unique_lock<boost::mutex> lock{mutex};
          while(!pendingRoutines.pop(routine)) {
            pendingRoutinesAvailableCondition.wait(lock);
          }
        }
        routine->Continue();
        if(routine->GetState() == Routine::State::COMPLETE) {
          routine->Complete();
        } else if(routine->GetState() == Routine::State::PENDING_SUSPEND) {
          Suspend(*routine);
        } else {
          Queue(*routine);
        }
      }
    } catch(const RoutineException&) {
      Queue(*routine);
    }
  }
}

  template<typename F>
  Routine::Id Spawn(F&& f) {
    return Details::Scheduler::GetInstance().Spawn(std::forward<F>(f));
  }

  template<typename F>
  Routine::Id Spawn(F&& f, std::size_t stackSize) {
    return Details::Scheduler::GetInstance().Spawn(std::forward<F>(f),
      stackSize);
  }

  inline void ScheduledRoutine::Resume() {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    assert(GetState() == State::SUSPENDED);
    m_scheduler->Queue(*this);
  }
}
}

#endif
