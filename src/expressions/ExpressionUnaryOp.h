#pragma once

#include "Expression.h"

enum class UnaryOperatorType
{
    AccessFromGlobal,
    PrefixIncrement, PrefixDecrement,
    PostfixIncrement, PostfixDecrement,
    PrefixPlus, PrefixMinus,
    LogicalNot, BitwiseNot,
    Dereference, PointerTo,
    TypeCast,
    SizeOf,
};

class ExpressionUnaryOperator : public _Expression
{
public:
    ExpressionUnaryOperator(UnaryOperatorType type, Expression<> expr, Datatype<> datatype, const qrawlr::Position& position);
private:
    UnaryOperatorType m_type;
};