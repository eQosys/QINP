#pragma once

#include "Expression.h"

class ExpressionReturn : public _Expression
{
public:
    ExpressionReturn(Expression<> sub_expr, const qrawlr::Position& position);
public:
    virtual bool is_const_expr() const override;
    virtual bool results_in_object() const override;
private:
    Expression<> m_sub_expr;
};