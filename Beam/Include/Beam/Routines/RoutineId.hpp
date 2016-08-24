#ifndef BEAM_ROUTINEID_HPP
#define BEAM_ROUTINEID_HPP
#include <cstdint>
#include <memory>
#include "Beam/Routines/Routines.hpp"

namespace Beam {
namespace Routines {

  /*! \class RoutineId
      \brief Represents a handle to a Routine.
   */
  class RoutineId {
    public:

      //! Constructs a RoutineId referring to no Routine.
      RoutineId();

      //! Copies a RoutineId.
      /*!
        \param id The RoutineId to move.
      */
      RoutineId(const RoutineId& id) = default;

      //! Moves a RoutineId.
      /*!
        \param id The RoutineId to move.
      */
      RoutineId(RoutineId&& id);

      //! Copies a RoutineId.
      /*!
        \param rhs The RoutineId to copy.
        \return <code>*this</code>.
      */
      RoutineId& operator =(const RoutineId& rhs);

      //! Moves a RoutineId.
      /*!
        \param rhs The RoutineId to move.
        \return <code>*this</code>.
      */
      RoutineId& operator =(RoutineId&& rhs);

      //! Tests a RoutineId for equality.
      /*!
        \param rhs The right hand side of the equality.
        \return <code>true</code> iff <code>*this</code> refers to the same
                Routine as <i>rhs</i>.
      */
      bool operator ==(const RoutineId& rhs) const;

      //! Tests a RoutineId for inequality.
      /*!
        \param rhs The right hand side of the inequality.
        \return <code>true</code> iff <code>*this</code> refers to a different
                Routine from <i>rhs</i>.
      */
      bool operator !=(const RoutineId& rhs) const;

    private:
      friend class Details::Scheduler;
      friend void Wait(const RoutineId& id);
      std::uint64_t m_id;
      std::weak_ptr<ScheduledRoutine> m_routine;

      RoutineId(std::uint64_t id,
        const std::shared_ptr<ScheduledRoutine>& routine);
      std::shared_ptr<ScheduledRoutine> GetRoutine() const;
  };

  inline RoutineId::RoutineId()
      : m_id{0} {}

  inline RoutineId::RoutineId(RoutineId&& id)
      : m_id{id.m_id},
        m_routine{std::move(id.m_routine)} {
    id.m_id = 0;
  }

  inline RoutineId& RoutineId::operator =(const RoutineId& rhs) {
    m_id = rhs.m_id;
    m_routine = rhs.m_routine;
    return *this;
  }

  inline RoutineId& RoutineId::operator =(RoutineId&& rhs) {
    m_id = rhs.m_id;
    m_routine = std::move(rhs.m_routine);
    rhs.m_id = 0;
    return *this;
  }

  inline bool RoutineId::operator ==(const RoutineId& rhs) const {
    return m_id == rhs.m_id;
  }

  inline bool RoutineId::operator !=(const RoutineId& rhs) const {
    return !(*this == rhs);
  }

  inline RoutineId::RoutineId(std::uint64_t id,
      const std::shared_ptr<ScheduledRoutine>& routine)
      : m_id{id},
        m_routine{routine} {}

  inline std::shared_ptr<ScheduledRoutine> RoutineId::GetRoutine() const {
    return m_routine.lock();
  }
}
}

#endif
