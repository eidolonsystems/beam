#ifndef BEAM_THREADING_HPP
#define BEAM_THREADING_HPP

namespace Beam::Threading {
  template<typename MutexType> class CallOnce;
  class ConditionVariable;
  class LiveTimer;
  template<typename LockType> class LockRelease;
  class Mutex;
  template<bool Acquire, typename MutexType> class OptionalLock;
  template<typename MutexType> struct PreferredConditionVariable;
  class RecursiveMutex;
  template<typename T, typename M> class Sync;
  class TaskRunner;
  class ThreadPool;
  class TimedConditionVariable;
  class TimeoutException;
  struct Timer;
  class TimerThreadPool;
  class TriggerTimer;
  class VirtualTimer;
  template<typename TimerType> class WrapperTimer;
}

#endif
