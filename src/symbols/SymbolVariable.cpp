#include "SymbolVariable.h"

SymbolVariable::SymbolVariable(const std::string& name, const VariableDeclarators& declarators, Datatype<> datatype, const qrawlr::Position& position)
    : _SymbolDeclDef(name, position),
    m_is_static(declarators.is_static),
    m_datatype(declarators.is_const ? datatype->make_const(), datatype : datatype)
{
    set_defined(position);
}

bool SymbolVariable::is_object() const
{
    throw std::runtime_error("[*SymbolVariable::is_object*]: Not implemented yet!");
}

Datatype<> SymbolVariable::get_datatype() const
{
    throw std::runtime_error("[*SymbolVariable::get_datatype*]: Not implemented yet!");
}