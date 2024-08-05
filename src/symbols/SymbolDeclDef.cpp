#include "SymbolDeclDef.h"

_SymbolDeclDef::_SymbolDeclDef(const std::string& name, const qrawlr::Position& declaration_position)
    : _Symbol(name, declaration_position),
    m_is_defined(false), m_definition_position()
{}

bool _SymbolDeclDef::is_defined() const
{
    return m_is_defined;
}

const qrawlr::Position& _SymbolDeclDef::get_definition_position() const
{
    return m_definition_position;
}

void _SymbolDeclDef::set_defined(const qrawlr::Position& definition_position)
{
    if (is_defined())
        throw std::runtime_error("Function '" + get_symbol_path().to_string() + "' has already been defined!");
    
    m_is_defined = true;
    m_definition_position = definition_position;
}