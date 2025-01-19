#include "SymbolSpace.h"

bool SymbolSpace::is_object() const
{
    throw std::runtime_error("[*SymbolSpace::is_object*]: Not implemented yet!");
}

Datatype<> SymbolSpace::get_datatype() const
{
    throw std::runtime_error("[*SymbolSpace::get_datatype*]: Not implemented yet!");
}

std::string SymbolSpace::get_digraph_impl_text(bool verbose) const
{
    std::string res = _SymbolDeclDef::get_digraph_impl_text(verbose);
    // nothing to do;
    return res;
}

std::string SymbolSpace::get_symbol_type_str() const
{
    return "SPACE";
}

void SymbolSpace::set_definition(const qrawlr::Position& definition_position)
{
    set_defined(definition_position);
}

void SymbolSpace::append_expr(Expression<> expr)
{
    m_body.expressions.push_back(expr);
}