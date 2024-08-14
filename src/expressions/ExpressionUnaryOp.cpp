#include "ExpressionUnaryOp.h"

ExpressionUnaryOperator::ExpressionUnaryOperator(UnaryOperatorType type, Expression<> sub_expr, Datatype<> datatype, const qrawlr::Position& position)
    : _Expression(datatype, position),
    m_type(type),
    m_sub_expr(sub_expr)
{}