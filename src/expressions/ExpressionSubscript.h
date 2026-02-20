#pragma once

#include "Expression.h"

class ExpressionSubscript : public _Expression
{
public:
    ExpressionSubscript(Expression<> mainExpr, Expression<> subExpr, const qrawlr::Position& position);
public:
    virtual bool is_const_expr() const override;
    virtual bool results_in_object() const override;
private:
    Expression<> m_mainExpr;
    Expression<> m_subExpr;
};