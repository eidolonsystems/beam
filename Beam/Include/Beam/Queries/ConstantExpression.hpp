#ifndef BEAM_QUERIES_CONSTANT_EXPRESSION_HPP
#define BEAM_QUERIES_CONSTANT_EXPRESSION_HPP
#include <utility>
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/NativeValue.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/Value.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam::Queries {

  /** An Expression that evaluates to a constant. */
  class ConstantExpression : public VirtualExpression,
      public CloneableMixin<ConstantExpression> {
    public:

      /**
       * Constructs a ConstantExpression.
       * @param value The value to evaluate to.
       */
      explicit ConstantExpression(Value value);

      /**
       * Constructs a ConstantExpression representing a native value.
       * @param value The native value to evaluate to.
       */
      template<typename T,
        typename = std::enable_if_t<!std::is_constructible_v<Value, T>>>
      explicit ConstantExpression(T value);

      /**
       * Copies a ConstantExpression.
       * @param expression The ConstantExpression to copy.
       */
      ConstantExpression(const ConstantExpression& expression) = default;

      /** Returns the Value to evaluate to. */
      const Value& GetValue() const;

      const DataType& GetType() const override;

      void Apply(ExpressionVisitor& visitor) const override;

    protected:
      std::ostream& ToStream(std::ostream& out) const override;

    private:
      friend struct Serialization::DataShuttle;
      Value m_value;

      ConstantExpression();
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  inline ConstantExpression::ConstantExpression(Value value)
    : m_value(std::move(value)) {}

  template<typename T, typename>
  ConstantExpression::ConstantExpression(T value)
    : ConstantExpression(NativeValue(std::move(value))) {}

  inline const Value& ConstantExpression::GetValue() const {
    return m_value;
  }

  inline const DataType& ConstantExpression::GetType() const {
    return m_value->GetType();
  }

  inline void ConstantExpression::Apply(ExpressionVisitor& visitor) const {
    visitor.Visit(*this);
  }

  inline std::ostream& ConstantExpression::ToStream(std::ostream& out) const {
    return out << *m_value;
  }

  inline ConstantExpression::ConstantExpression()
    : m_value(NativeValue(0)) {}

  template<typename Shuttler>
  void ConstantExpression::Shuttle(Shuttler& shuttle, unsigned int version) {
    VirtualExpression::Shuttle(shuttle, version);
    shuttle.Shuttle("value", m_value);
  }

  inline void ExpressionVisitor::Visit(const ConstantExpression& expression) {
    Visit(static_cast<const VirtualExpression&>(expression));
  }
}

#endif
