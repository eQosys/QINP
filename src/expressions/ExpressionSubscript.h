#pragma once

#include "Expression.h"

class ExpressionSubscript : public _Expression
{
public:
    ExpressionSubscript(Expression<> mainExpr, Expression<> subExpr);
public:
    virtual bool is_const_expr() const override;
private:
    Expression<> m_mainExpr;
    Expression<> m_subExpr;
};