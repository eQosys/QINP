#pragma once

#include "Expression.h"

class ExpressionIf : public _Expression
{
public:
    ExpressionIf(Expression<> condition, const qrawlr::Position& position);
public:
    virtual bool is_const_expr() const override;
    virtual bool results_in_object() const override;
private:
    Expression<> m_condition;
};