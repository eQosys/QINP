#include "ExpressionUnaryOp.h"

ExpressionUnaryOperator::ExpressionUnaryOperator(UnaryOperatorType type, Expression<> sub_expr, Datatype<> datatype, const qrawlr::Position& position)
    : _Expression(datatype, position),
    m_type(type),
    m_sub_expr(sub_expr)
{}

bool ExpressionUnaryOperator::is_const_expr() const
{
    return m_sub_expr->is_const_expr();
}

bool ExpressionUnaryOperator::results_in_object() const
{
    return m_sub_expr->results_in_object();
}