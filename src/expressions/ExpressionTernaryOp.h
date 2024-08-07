#pragma once

#include "Expression.h"

enum class TernaryOperatorType
{
    TernaryIfElse
};

class ExpressionTernaryOperator : public _Expression
{
public:
    ExpressionTernaryOperator(TernaryOperatorType type, Expression<> exprLeft, Expression<> exprMid, Expression<> exprRight, Datatype<> datatype, const qrawlr::Position& position);
private:
    TernaryOperatorType m_type;
};