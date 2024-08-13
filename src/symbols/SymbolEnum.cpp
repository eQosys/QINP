#include "SymbolEnum.h"

SymbolEnum::SymbolEnum(const std::string& name, const qrawlr::Position& declaration_position)
    : _SymbolDeclDef(name, declaration_position),
    m_last_value(-1)
{}

bool SymbolEnum::is_object() const
{
    return false;
}

Datatype<> SymbolEnum::get_datatype() const
{
    return DT_NAMED(get_symbol_path().to_string(), true);
}

std::size_t SymbolEnum::next_value()
{
    return ++m_last_value;
}

void SymbolEnum::set_last_value(std::size_t value)
{
    m_last_value = value;
}

SymbolEnumMember::SymbolEnumMember(const std::string& name, std::size_t value, const qrawlr::Position& position)
    : _Symbol(name, position),
    m_value(value)
{}

bool SymbolEnumMember::is_object() const
{
    return true;
}

Datatype<> SymbolEnumMember::get_datatype() const
{
    return DT_NAMED(get_symbol_path().to_string(), true);
}

std::size_t SymbolEnumMember::get_value() const
{
    return m_value;
}