#ifndef BEAM_QUERIES_GLOBAL_VARIABLE_DECLARATION_EXPRESSION_HPP
#define BEAM_QUERIES_GLOBAL_VARIABLE_DECLARATION_EXPRESSION_HPP
#include <string>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/DataType.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam::Queries {

  /** Declares a global variable and evaluates an expression. */
  class GlobalVariableDeclarationExpression : public VirtualExpression,
      public CloneableMixin<GlobalVariableDeclarationExpression> {
    public:

      /**
       * Constructs a GlobalVariableDeclarationExpression.
       * @param name The name of the variable.
       * @param initialValue The variable's initial value.
       * @param body The body to evaluate.
       */
      GlobalVariableDeclarationExpression(std::string name,
        Expression initialValue, Expression body);

      /**
       * Copies a GlobalVariableDeclarationExpression.
       * @param expression The GlobalVariableDeclarationExpression to copy.
       */
      GlobalVariableDeclarationExpression(
        const GlobalVariableDeclarationExpression& expression) = default;

      /** Returns the name of the variable. */
      const std::string& GetName() const;

      /** Returns the variable's initial value. */
      const Expression& GetInitialValue() const;

      /** Returns the body to evaluate. */
      const Expression& GetBody() const;

      const DataType& GetType() const override;

      void Apply(ExpressionVisitor& visitor) const override;

    protected:
      std::ostream& ToStream(std::ostream& out) const override;

    private:
      friend struct Serialization::DataShuttle;
      std::string m_name;
      Expression m_initialValue;
      Expression m_body;

      GlobalVariableDeclarationExpression();
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  inline GlobalVariableDeclarationExpression::
    GlobalVariableDeclarationExpression(std::string name,
      Expression initialValue, Expression body)
    : m_name(std::move(name)),
      m_initialValue(std::move(initialValue)),
      m_body(std::move(body)) {}

  inline const std::string&
      GlobalVariableDeclarationExpression::GetName() const {
    return m_name;
  }

  inline const Expression&
      GlobalVariableDeclarationExpression::GetInitialValue() const {
    return m_initialValue;
  }

  inline const Expression&
      GlobalVariableDeclarationExpression::GetBody() const {
    return m_body;
  }

  inline const DataType& GlobalVariableDeclarationExpression::GetType() const {
    return m_body->GetType();
  }

  inline void GlobalVariableDeclarationExpression::Apply(
      ExpressionVisitor& visitor) const {
    visitor.Visit(*this);
  }

  inline std::ostream& GlobalVariableDeclarationExpression::ToStream(
      std::ostream& out) const {
    return out << "(global (" << m_name << " " << *m_initialValue << ") " <<
      *m_body << ")";
  }

  inline GlobalVariableDeclarationExpression::
    GlobalVariableDeclarationExpression()
    : GlobalVariableDeclarationExpression(
        "", ConstantExpression(false), ConstantExpression(false)) {}

  template<typename Shuttler>
  void GlobalVariableDeclarationExpression::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    VirtualExpression::Shuttle(shuttle, version);
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("initial_value", m_initialValue);
    shuttle.Shuttle("body", m_body);
  }

  inline void ExpressionVisitor::Visit(
      const GlobalVariableDeclarationExpression& expression) {
    Visit(static_cast<const VirtualExpression&>(expression));
  }
}

#endif
