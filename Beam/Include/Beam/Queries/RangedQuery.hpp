#ifndef BEAM_RANGED_QUERY_HPP
#define BEAM_RANGED_QUERY_HPP
#include <ostream>
#include <utility>
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/Range.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam::Queries {

  /** Queries for data over a specified Range. */
  class RangedQuery {
    public:

      /** Constructs a RangedQuery over an empty Range. */
      RangedQuery() = default;

      /**
       * Constructs a RangedQuery over a specified Range.
       * @param range The Range to Query over.
       */
      RangedQuery(Range range);

      /** Returns the Range to Query. */
      const Range& GetRange() const;

      /** Sets the Range to Query. */
      void SetRange(const Range& range);

      /**
       * Sets the Range to Query.
       * @param start The start point.
       * @param end The end point.
       */
      template<typename Start, typename End>
      void SetRange(Start&& start, End&& end);

    private:
      friend struct Serialization::Shuttle<RangedQuery>;
      Range m_range;
  };

  inline std::ostream& operator <<(std::ostream& out,
      const RangedQuery& query) {
    return out << query.GetRange();
  }

  inline RangedQuery::RangedQuery(Range range)
    : m_range(std::move(range)) {}

  inline const Range& RangedQuery::GetRange() const {
    return m_range;
  }

  inline void RangedQuery::SetRange(const Range& range) {
    m_range = range;
  }

  template<typename Start, typename End>
  inline void RangedQuery::SetRange(Start&& start, End&& end) {
    m_range = Range(std::forward<Start>(start), std::forward<End>(end));
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Queries::RangedQuery> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Queries::RangedQuery& value,
        unsigned int version) {
      shuttle.Shuttle("range", value.m_range);
    }
  };
}

#endif
