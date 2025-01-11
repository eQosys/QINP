#pragma once

#include "Expression.h"

class ExpressionFunctionCall : public _Expression
{
public:
    ExpressionFunctionCall();
public:
    virtual bool is_const_expr() const override;
    virtual bool results_in_object() const override;
private:
    ;
};