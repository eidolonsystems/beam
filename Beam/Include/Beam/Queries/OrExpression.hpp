#ifndef BEAM_QUERIES_OR_EXPRESSION_HPP
#define BEAM_QUERIES_OR_EXPRESSION_HPP
#include <utility>
#include <boost/throw_exception.hpp>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"
#include "Beam/Queries/TypeCompatibilityException.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam::Queries {

  /** Represents a logical or expression. */
  class OrExpression :
      public VirtualExpression, public CloneableMixin<OrExpression> {
    public:

      /**
       * Constructs an OrExpression.
       * @param lhs The left hand side of the Expression.
       * @param rhs The right hand side of the Expression.
       */
      OrExpression(Expression lhs, Expression rhs);

      /**
       * Copies an OrExpression.
       * @param expression The OrExpression to copy.
       */
      OrExpression(const OrExpression& expression) = default;

      /** Returns the left hand side of the Expression. */
      const Expression& GetLeftExpression() const;

      /** Returns the right hand side of the Expression. */
      const Expression& GetRightExpression() const;

      const DataType& GetType() const override;

      void Apply(ExpressionVisitor& visitor) const override;

    protected:
      std::ostream& ToStream(std::ostream& out) const override;

    private:
      friend struct Serialization::DataShuttle;
      Expression m_left;
      Expression m_right;

      OrExpression();
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  /**
   * Makes an Expression that represents the logical or over a sequence of
   * sub-Expressions.
   * @param first An iterator to the first Expression.
   * @param last An iterator to one past the last Expression.
   * @return An Expression that represents the logical or over the sequence of
   *         sub-Expressions.
   */
  template<typename ForwardIterator>
  inline Expression MakeOrExpression(
      ForwardIterator first, ForwardIterator last) {
    if(first == last) {
      return ConstantExpression(false);
    }
    if((*first)->GetType()->GetNativeType() != typeid(bool)) {
      BOOST_THROW_EXCEPTION(
        TypeCompatibilityException("Expression must be bool."));
    }
    if(first + 1 == last) {
      return *first;
    } else {
      auto right = MakeOrExpression(first + 1, last);
      return OrExpression(*first, right);
    }
  }

  inline OrExpression::OrExpression(Expression lhs, Expression rhs)
      : m_left(std::move(lhs)),
        m_right(std::move(rhs)) {
    if(m_left->GetType()->GetNativeType() != typeid(bool)) {
      BOOST_THROW_EXCEPTION(
        TypeCompatibilityException("Expression must be bool."));
    }
    if(m_right->GetType()->GetNativeType() != typeid(bool)) {
      BOOST_THROW_EXCEPTION(
        TypeCompatibilityException("Expression must be bool."));
    }
  }

  inline const Expression& OrExpression::GetLeftExpression() const {
    return m_left;
  }

  inline const Expression& OrExpression::GetRightExpression() const {
    return m_right;
  }

  inline const DataType& OrExpression::GetType() const {
    static auto value = DataType(BoolType::GetInstance());
    return value;
  }

  inline void OrExpression::Apply(ExpressionVisitor& visitor) const {
    visitor.Visit(*this);
  }

  inline std::ostream& OrExpression::ToStream(std::ostream& out) const {
    return out << "(or " << GetLeftExpression() << " " <<
      GetRightExpression() << ")";
  }

  inline OrExpression::OrExpression()
    : m_left(ConstantExpression(false)),
      m_right(ConstantExpression(false)) {}

  template<typename Shuttler>
  void OrExpression::Shuttle(Shuttler& shuttle, unsigned int version) {
    VirtualExpression::Shuttle(shuttle, version);
    shuttle.Shuttle("left", m_left);
    shuttle.Shuttle("right", m_right);
    if(Serialization::IsReceiver<Shuttler>::value) {
      if(m_left->GetType()->GetNativeType() != typeid(bool)) {
        BOOST_THROW_EXCEPTION(
          Serialization::SerializationException("Incompatible types."));
      }
      if(m_right->GetType()->GetNativeType() != typeid(bool)) {
        BOOST_THROW_EXCEPTION(
          Serialization::SerializationException("Incompatible types."));
      }
    }
  }

  inline void ExpressionVisitor::Visit(const OrExpression& expression) {
    Visit(static_cast<const VirtualExpression&>(expression));
  }
}

#endif
