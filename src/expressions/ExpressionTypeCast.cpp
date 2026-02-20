#include "ExpressionTypeCast.h"

ExpressionTypeCast::ExpressionTypeCast(Datatype<> dtTarget, Expression<> expr)
    : _Expression(dtTarget, expr->get_position()),
    m_sub_expr(expr)
{}

bool ExpressionTypeCast::is_const_expr() const
{
    return m_sub_expr->is_const_expr();
}
