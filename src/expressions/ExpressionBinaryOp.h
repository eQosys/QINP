#pragma once

#include "Expression.h"

enum class BinaryOperatorType
{
    Assign,
    AssignSum, AssignDifference,
    AssignProduct, AssignQuotient,
    AssignModulo,
    AssignLogicalAnd, AssignLogicalOr, AssignLogicalXor,
    AssignBitwiseAnd, AssignBitwiseOr, AssignBitwiseXor,
    AssignShiftLeft, AssignShiftRight,
    LogicalAnd, LogicalOr, LogicalXor,
    BitwiseAnd, BitwiseOr, BitwiseXor,
    Equal, NotEqual,
    SmallerEquals, Smaller,
    GreaterEquals, Greater,
    ShiftLeft, ShiftRight,
    Sum, Difference,
    Product, Quotient,
    Modulo,
    AccessMember, AccessPointerMember,
    FunctionCall,
    Subscript,
};

class ExpressionBinaryOperator : public _Expression
{
public:
    ExpressionBinaryOperator(BinaryOperatorType type, Expression<> exprLeft, Expression<> exprRight, Datatype<> datatype, const qrawlr::Position& position);
public:
    virtual bool is_const_expr() const override;
    virtual bool results_in_object() const override;
private:
    BinaryOperatorType m_type;
    Expression<> m_exprLeft;
    Expression<> m_exprRight;
};