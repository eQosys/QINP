#include "SymbolVariable.h"

SymbolVariable::SymbolVariable(const std::string& name, const VariableDeclarators& declarators, Datatype<> datatype, Expression<> initializer, const qrawlr::Position& position)
    : _SymbolDeclDef(name, position),
    m_is_static(declarators.is_static),
    m_datatype(declarators.is_const ? datatype->make_const(), datatype : datatype),
    m_initializer(initializer)
{
    set_defined(position);
}