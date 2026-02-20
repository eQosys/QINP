#include "SymbolVariable.h"

#include "utility/StringEscape.h"

SymbolVariable::SymbolVariable(const std::string& name, const VariableDeclarators& declarators, Datatype<> datatype, const qrawlr::Position& position)
    : _SymbolDeclDef(name, position),
    m_is_static(declarators.is_static),
    m_datatype(declarators.is_const ? datatype->make_const(), datatype : datatype)
{
    set_defined(position);
}

bool SymbolVariable::is_object() const
{
    return true;
}

Datatype<> SymbolVariable::get_datatype() const
{
    throw std::runtime_error("[*SymbolVariable::get_datatype*]: Not implemented yet!");
}

std::string SymbolVariable::get_digraph_impl_text(bool verbose) const
{
    std::string res = _SymbolDeclDef::get_digraph_impl_text(verbose);
    res += "IsStatic: ";
    res += m_is_static ? "YES\n" : "no\n";
    res += "Datatype: " + escape_string(m_datatype->get_symbol_name()) + "\n";
    return res;
}

std::string SymbolVariable::get_symbol_type_str() const
{
    return "VARIABLE";
}