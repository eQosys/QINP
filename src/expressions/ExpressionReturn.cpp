#include "ExpressionReturn.h"

ExpressionReturn::ExpressionReturn(Expression<> sub_expr, const qrawlr::Position& position)
    : _Expression(DT_NAMED("void", false), position),
    m_sub_expr(sub_expr)
{}

bool ExpressionReturn::is_const_expr() const
{
    return m_sub_expr->is_const_expr();
}

bool ExpressionReturn::results_in_object() const
{
    return false;
}