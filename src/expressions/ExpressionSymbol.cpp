#include "ExpressionSymbol.h"

ExpressionSymbol::ExpressionSymbol(Symbol<> symbol, const qrawlr::Position& position)
    : _Expression(symbol->get_datatype(), position),
    m_symbol(symbol)
{}

bool ExpressionSymbol::is_const_expr() const
{
    // TODO: Implementation
    return false;
}

bool ExpressionSymbol::results_in_object() const
{
    return m_symbol->is_object();
}