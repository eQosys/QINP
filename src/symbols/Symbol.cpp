#include "Symbol.h"

#include "errors/SymbolError.h"

_Symbol::_Symbol(const std::string& name, const qrawlr::Position& position)
    : m_name(name), m_position(position), m_parent()
{}

void _Symbol::add_child(Symbol<> sym, Symbol<> this_sym)
{
    if (m_children.find(sym->get_name()) != m_children.end())
        throw SymbolError(sym, "Duplicate symbol: " + sym->get_name());

    m_children[sym->get_name()] = sym;
    m_parent = this_sym;
}

Symbol<> _Symbol::get_child_by_name(const std::string& name) const
{
    auto it = m_children.find(name);
    if (it == m_children.end())
        return nullptr;
    return it->second;
}

const std::string& _Symbol::get_name() const
{
    return m_name;
}

Symbol<> _Symbol::get_parent() const
{
    auto p = m_parent.lock();
    return *(Symbol<>*)(&p);
}

const qrawlr::Position& _Symbol::get_position() const
{
    return m_position;
}

SymbolPath _Symbol::get_symbol_path() const
{
    auto p = get_parent();
    if (!p)
        return SymbolPath("." + get_name());

    auto path = p->get_symbol_path();
    path.enter(get_name());
    return path;
}