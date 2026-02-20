#include "ExpressionSubscript.h"

ExpressionSubscript::ExpressionSubscript(Expression<> mainExpr, Expression<> subExpr, const qrawlr::Position &position)
    : _Expression(mainExpr->get_datatype()->get_dereferenced(), position),
    m_mainExpr(mainExpr), m_subExpr(subExpr)
{
}

bool ExpressionSubscript::is_const_expr() const
{
    return m_mainExpr->is_const_expr() && m_subExpr->is_const_expr();
}

bool ExpressionSubscript::results_in_object() const
{
    return true;
}
