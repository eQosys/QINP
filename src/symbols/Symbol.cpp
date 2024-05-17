#include "Symbol.h"

Symbol::Symbol(const std::string& name, const Location& location)
    : m_name(name), m_location(location)
{}

void Symbol::add_child(SymbolRef sym)
{
    // TODO: check for duplicates
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