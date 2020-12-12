#ifndef BEAM_FILTERED_QUERY_HPP
#define BEAM_FILTERED_QUERY_HPP
#include <ostream>
#include <boost/throw_exception.hpp>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/Evaluator.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/NativeValue.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/TypeCompatibilityException.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/SerializationException.hpp"

namespace Beam::Queries {

  /** Filters what values should be returned in a Query. */
  class FilteredQuery {
    public:

      /** Constructs a FilteredQuery that returns all values. */
      FilteredQuery();

      /**
       * Constructs a FilteredQuery with a specified filter.
       * @param filter The Expression used as the filter.
       */
      FilteredQuery(Expression filter);

      /** Returns the filter. */
      const Expression& GetFilter() const;

      /** Sets the filter. */
      void SetFilter(const Expression& filter);

    private:
      friend struct Serialization::Shuttle<FilteredQuery>;
      Expression m_filter;
  };

  /**
   * Uses an Evaluator to test whether a value passes a filter.
   * @param evaluator The Evaluator used as the filter.
   * @param value The value to filter.
   * @return <code>true</code> iff the <i>value</i> passes the filter.
   */
  template<typename T>
  bool TestFilter(Evaluator& evaluator, const T& value) {
    try {
      return evaluator.Eval<bool>(value);
    } catch(const std::exception&) {
      return false;
    }
  }

  inline std::ostream& operator <<(std::ostream& out,
      const FilteredQuery& query) {
    return out << query.GetFilter();
  }

  inline FilteredQuery::FilteredQuery()
    : FilteredQuery(ConstantExpression(true)) {}

  inline FilteredQuery::FilteredQuery(Expression filter)
      : m_filter(std::move(filter)) {
    if(m_filter->GetType()->GetNativeType() != typeid(bool)) {
      BOOST_THROW_EXCEPTION(
        TypeCompatibilityException("Filter is not boolean."));
    }
  }

  inline const Expression& FilteredQuery::GetFilter() const {
    return m_filter;
  }

  inline void FilteredQuery::SetFilter(const Expression& filter) {
    if(filter->GetType()->GetNativeType() != typeid(bool)) {
      BOOST_THROW_EXCEPTION(
        TypeCompatibilityException("Filter is not boolean."));
    }
    m_filter = filter;
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Queries::FilteredQuery> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Queries::FilteredQuery& value,
        unsigned int version) {
      shuttle.Shuttle("filter", value.m_filter);
      if(IsReceiver<Shuttler>::value) {
        if(value.m_filter->GetType()->GetNativeType() != typeid(bool)) {
          value.m_filter = Queries::ConstantExpression(false);
          BOOST_THROW_EXCEPTION(
            SerializationException("Filter is not boolean."));
        }
      }
    }
  };
}

#endif
