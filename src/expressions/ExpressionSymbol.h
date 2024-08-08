#pragma once

#include "Expression.h"
#include "symbols/Symbol.h"

class ExpressionSymbol : public _Expression
{
public:
    ExpressionSymbol(Symbol<> symbol, const qrawlr::Position& position);
public:
    virtual bool is_const_expr() const override;
    virtual bool results_in_object() const override;
private:
    Symbol<> m_symbol;
};