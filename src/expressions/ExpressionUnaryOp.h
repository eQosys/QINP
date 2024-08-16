#pragma once

#include "Expression.h"

enum class UnaryOperatorType
{
    AccessFromGlobal,
    PrefixIncrement, PrefixDecrement,
    PostfixIncrement, PostfixDecrement,
    PrefixPlus, PrefixMinus,
    LogicalNot, BitwiseNot,
    Dereference, AddressOf,
    TypeCast,
    SizeOf,
};

class ExpressionUnaryOperator : public _Expression
{
public:
    ExpressionUnaryOperator(UnaryOperatorType type, Expression<> sub_expr, Datatype<> datatype, const qrawlr::Position& position);
public:
    virtual bool is_const_expr() const override;
    virtual bool results_in_object() const override;
private:
    UnaryOperatorType m_type;
    Expression<> m_sub_expr;
};