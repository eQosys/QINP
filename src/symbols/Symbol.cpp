#include "Symbol.h"

#include "errors/SymbolError.h"

_Symbol::_Symbol(const std::string& name, const qrawlr::Position& position)
    : m_name(name), m_position(position), m_parent()
{}

Symbol<> _Symbol::add_child(Symbol<> this_sym, Symbol<> child_sym, DuplicateHandling dupHandling, std::function<std::string(int)> tree_id_to_name)
{
    if (child_sym->get_parent() != nullptr)
        throw SymbolError(
            "Cannot add symbol '" + child_sym->get_name() +
            "' as a child to '" + this_sym->get_symbol_path().to_string() +
            "', it already has a parent (" +
            child_sym->get_parent()->get_symbol_path().to_string() + ")",
            child_sym,
            tree_id_to_name
        );

    auto it = m_children.find(child_sym->get_name());
    if (it == m_children.end())
    {
        m_children[child_sym->get_name()] = child_sym;
        child_sym->m_parent = this_sym;
        return child_sym;
    }

    auto old_child_sym = it->second;

    switch (dupHandling)
    {
    case DuplicateHandling::Throw:
        throw SymbolError("Duplicate symbol: '" + child_sym->get_name() + "' in '" + this_sym->get_symbol_path().to_string() + "'", child_sym, tree_id_to_name);
    case DuplicateHandling::Keep:
        return old_child_sym;
    case DuplicateHandling::Replace:
        m_children[child_sym->get_name()] = child_sym;
        child_sym->m_parent = this_sym;
        return child_sym;
    default:
        std::runtime_error("[*_Symbol::add_child*]: Unhandled value of DuplicateHandling!");
    }

    return nullptr;
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
        return SymbolPath(get_name());

    auto path = p->get_symbol_path();
    path.enter(get_name());
    return path;
}