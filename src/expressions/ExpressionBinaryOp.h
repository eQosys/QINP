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
private:
    BinaryOperatorType m_type;
};