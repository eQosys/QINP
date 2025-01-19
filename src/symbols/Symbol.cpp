#include "Symbol.h"

#include "errors/SymbolError.h"

#include "utility/StringEscape.h"

size_t _Symbol::s_next_id = 1;

_Symbol::_Symbol()
    : _Symbol("<unnamed>", {})
{}

_Symbol::_Symbol(const std::string& name, const qrawlr::Position& position)
    : m_id(s_next_id++), m_name(name), m_position(position), m_parent()
{}

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

Symbol<> _Symbol::add_child(Symbol<> this_sym, Symbol<> child_sym, std::function<std::string(int)> tree_id_to_name, DuplicateHandling dupHandling)
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

std::string _Symbol::to_digraph_str(bool verbose) const
{
    std::string graph;

    graph += "digraph {\n\tgraph [rankdir=LR]\n";
    to_digraph_internal(graph, verbose);
    graph += "}\n";

    return graph;
}

std::string _Symbol::get_digraph_impl_text(bool verbose) const
{
    std::string res;
    res += "Name: " + escape_string(m_name) + "\n";
    return res;
}

std::string _Symbol::get_symbol_type_str() const
{
    return "<unspecified>";
}

void _Symbol::to_digraph_internal(std::string& graph, bool verbose) const
{
    graph += "\t";
    graph += std::to_string(m_id);
    graph += " [label=\"";
    graph += get_symbol_type_str() + "\n";
    graph += escape_string(get_digraph_impl_text(verbose));
    graph += "\" shape=box]\n";

    // add all sub-symbols to digraph
    for (auto& pair : m_children)
    {
        auto& child = pair.second;

        child->to_digraph_internal(graph, verbose);
        graph += "\t";
        graph += std::to_string(m_id);
        graph += " -> ";
        graph += std::to_string(child->m_id);
        graph += "\n";
    }
}