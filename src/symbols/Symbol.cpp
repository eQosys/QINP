#include "Symbol.h"

#include "errors/SymbolError.h"

Symbol::Symbol(const std::string& name, const Location& location, SymbolRef parent)
    : m_name(name), m_location(location), m_parent(parent)
{}

void Symbol::add_child(SymbolRef sym)
{
    if (m_children.find(sym->get_name()) != m_children.end())
        throw SymbolError(sym, "Duplicate symbol: " + sym->get_name());

    m_children[sym->get_name()] = sym;
}

SymbolRef Symbol::get_child_by_name(const std::string& name) const
{
    auto it = m_children.find(name);
    if (it == m_children.end())
        return nullptr;
    return it->second;
}

const std::string& Symbol::get_name() const
{
    return m_name;
}

const Location& Symbol::get_location() const
{
    return m_location;
}