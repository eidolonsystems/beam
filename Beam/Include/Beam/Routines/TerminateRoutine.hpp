#ifndef BEAM_TERMINATEROUTINE_HPP
#define BEAM_TERMINATEROUTINE_HPP
#include "Beam/Routines/Routines.hpp"
#include "Beam/Routines/ScheduledRoutine.hpp"

namespace Beam {
namespace Routines {

  /*! \class TerminateRoutine
      \brief Sentinel object used to terminate the Scheduler.
   */
  class TerminateRoutine : public ScheduledRoutine {
    public:

      //! Constructs a TerminateRoutine.
      /*!
        \param scheduler The Scheduler to terminate.
      */
      TerminateRoutine(Ref<Details::Scheduler> scheduler);

      void Execute() override;
  };

  inline TerminateRoutine::TerminateRoutine(
    Ref<Details::Scheduler> scheduler)
    : ScheduledRoutine(64 * 1024, Ref(scheduler)) {}

  inline void TerminateRoutine::Execute() {
    throw RoutineException("Routine aborted.");
  }
}
}

#endif
