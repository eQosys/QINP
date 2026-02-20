#pragma once

#include "Expression.h"

#include "symbols/SymbolDeclDef.h"

class ExpressionIf : public _Expression
{
public:
    ExpressionIf(Expression<> condition, Symbol<const class SymbolSpace> body_true, const qrawlr::Position& position);
public:
    virtual bool is_const_expr() const override;
    virtual bool results_in_object() const override;
public:
    void set_body_false(Symbol<const SymbolSpace> body_false);
private:
    Expression<> m_condition;
    Symbol<const SymbolSpace> m_body_true = nullptr;
    Symbol<const SymbolSpace> m_body_false = nullptr;
};