#include "Symbol.h"

#include "errors/SymbolError.h"

Symbol::Symbol(const std::string& name, const Location& location, SymbolRef parent)
    : m_name(name), m_location(location), m_parent(parent)
{}

void Symbol::add_child(SymbolRef sym)
{
    if (m_children.find(sym->name()) != m_children.end())
        throw SymbolError(sym, "Duplicate symbol: " + sym->name());

    m_children[sym->name()] = sym;
}

SymbolRef Symbol::get_child(const std::string& name) const
{
    auto it = m_children.find(name);
    if (it == m_children.end())
        return nullptr;
    return it->second;
}

const std::string& Symbol::name() const
{
    return m_name;
}

const Location& Symbol::location() const
{
    return m_location;
}