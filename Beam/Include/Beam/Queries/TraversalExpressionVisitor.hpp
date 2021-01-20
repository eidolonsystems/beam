#ifndef BEAM_TRAVERSAL_EXPRESSION_VISITOR_HPP
#define BEAM_TRAVERSAL_EXPRESSION_VISITOR_HPP
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/FunctionExpression.hpp"
#include "Beam/Queries/GlobalVariableDeclarationExpression.hpp"
#include "Beam/Queries/MemberAccessExpression.hpp"
#include "Beam/Queries/NotExpression.hpp"
#include "Beam/Queries/OrExpression.hpp"
#include "Beam/Queries/ParameterExpression.hpp"
#include "Beam/Queries/ReduceExpression.hpp"
#include "Beam/Queries/SetVariableExpression.hpp"
#include "Beam/Queries/VariableExpression.hpp"
#include "Beam/Queries/Queries.hpp"

namespace Beam::Queries {

  /** An ExpressionVisitor that traverses all of its children. */
  class TraversalExpressionVisitor : public ExpressionVisitor {
    public:
      void Visit(const ConstantExpression& expression) override;

      void Visit(const FunctionExpression& expression) override;

      void Visit(
        const GlobalVariableDeclarationExpression& expression) override;

      void Visit(const MemberAccessExpression& expression) override;

      void Visit(const NotExpression& expression) override;

      void Visit(const OrExpression& expression) override;

      void Visit(const ParameterExpression& expression) override;

      void Visit(const ReduceExpression& expression) override;

      void Visit(const SetVariableExpression& expression) override;

      void Visit(const VariableExpression& expression) override;

      void Visit(const VirtualExpression& expression) override;
  };

  inline void TraversalExpressionVisitor::Visit(
    const ConstantExpression& expression) {}

  inline void TraversalExpressionVisitor::Visit(
      const FunctionExpression& expression) {
    for(auto& parameter : expression.GetParameters()) {
      parameter->Apply(*this);
    }
  }

  inline void TraversalExpressionVisitor::Visit(
      const GlobalVariableDeclarationExpression& expression) {
    expression.GetInitialValue()->Apply(*this);
    expression.GetBody()->Apply(*this);
  }

  inline void TraversalExpressionVisitor::Visit(
      const MemberAccessExpression& expression) {
    expression.GetExpression()->Apply(*this);
  }

  inline void TraversalExpressionVisitor::Visit(
      const NotExpression& expression) {
    expression.GetOperand()->Apply(*this);
  }

  inline void TraversalExpressionVisitor::Visit(
      const OrExpression& expression) {
    expression.GetLeftExpression()->Apply(*this);
    expression.GetRightExpression()->Apply(*this);
  }

  inline void TraversalExpressionVisitor::Visit(
      const ParameterExpression& expression) {}

  inline void TraversalExpressionVisitor::Visit(
      const ReduceExpression& expression) {
    expression.GetReduceExpression()->Apply(*this);
    expression.GetSeriesExpression()->Apply(*this);
  }

  inline void TraversalExpressionVisitor::Visit(
      const SetVariableExpression& expression) {
    expression.GetValue()->Apply(*this);
  }

  inline void TraversalExpressionVisitor::Visit(
    const VariableExpression& expression) {}

  inline void TraversalExpressionVisitor::Visit(
    const VirtualExpression& expression) {}
}

#endif
