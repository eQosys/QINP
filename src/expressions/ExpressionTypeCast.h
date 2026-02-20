#pragma once

#include "Expression.h"

class ExpressionTypeCast : public _Expression
{
public:
    ExpressionTypeCast(Datatype<> dtTarget, Expression<> expr);
public:
    virtual bool is_const_expr() const override;
private:
    Expression<> m_sub_expr;
};