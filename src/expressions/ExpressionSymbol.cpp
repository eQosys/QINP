#include "ExpressionSymbol.h"

ExpressionSymbol::ExpressionSymbol(Symbol<> symbol, const qrawlr::Position& position)
    : _Expression(symbol->get_datatype(), position),
    m_symbol(symbol)
{}

bool ExpressionSymbol::is_const_expr() const
{
    return get_symbol()->is_const_expr();
}

CEObject ExpressionSymbol::eval_const_expr() const
{
    return get_symbol()->eval_const_expr();
}

bool ExpressionSymbol::results_in_object() const
{
    return m_symbol->is_object();
}

Symbol<> ExpressionSymbol::get_symbol() const
{
    return m_symbol;
}