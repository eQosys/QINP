#pragma once

#include "Expression.h"

enum class BinaryOperatorType
{
    Assign,
    AssignSum,
    AssignDifference,
    AssignProduct,
    AssignQuotient,
    AssignModulo,
    AssignLogicalAnd,
    AssignLogicalOr,
    AssignLogicalXor,
    AssignShiftLeft,
    AssignShiftRight,
    AssignBinaryAnd,
    AssignBinaryOr,
    AssignBinaryXor,
    LogicalOr, LogicalXor, LogicalAnd,
    BitwiseOr, BitwiseXor, BitwiseAnd,
    Equal, NotEqual,
    SmallerEquals, Smaller,
    GreaterEquals, Greater,
    ShiftLeft, ShiftRight,
    Sum, Difference,
    Product, Quotient,
    Modulo,
    AccessMember,
    AccessPointerMember,
    FunctionCall,
    Subscript
};

class ExpressionBinaryOperator : public _Expression
{
public:
    ExpressionBinaryOperator(BinaryOperatorType type, Expression<> exprLeft, Expression<> exprRight, Datatype<> datatype, const qrawlr::Position& position);
private:
    BinaryOperatorType m_type;
};