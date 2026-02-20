#include "ExpressionBinaryOp.h"

ExpressionBinaryOperator::ExpressionBinaryOperator(BinaryOperatorType type, Expression<> exprLeft, Expression<> exprRight, Datatype<> datatype, const qrawlr::Position& position)
    : _Expression(datatype, position),
    m_type(type),
    m_exprLeft(exprLeft),
    m_exprRight(exprRight)
{}

bool ExpressionBinaryOperator::is_const_expr() const
{
    return m_exprLeft->is_const_expr() && m_exprRight->is_const_expr();
}