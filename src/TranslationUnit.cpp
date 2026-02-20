#include "TranslationUnit.h"

TranslationUnit::TranslationUnit(const std::string& path, const Symbol<> sym_root)
    : m_file_path(path), m_sym_root(sym_root), m_sym_stack()
{
    m_sym_stack.push_back(m_sym_root);
}

void TranslationUnit::enter_symbol(Symbol<> sym)
{
    m_sym_stack.push_back(sym);
}

void TranslationUnit::leave_symbol()
{
    m_sym_stack.pop_back();
}

Symbol<> TranslationUnit::curr_symbol() const
{
    return m_sym_stack.back();
}

Symbol<> TranslationUnit::get_symbol_from_path(const SymbolPath& path) const
{
    if (path.is_from_root())
        return get_symbol_from_path(path, m_sym_root);

    for (auto it = m_sym_stack.rbegin(); it != m_sym_stack.rend(); ++it)
    {
        auto sym = get_symbol_from_path(path, *it);
        if (sym)
            return sym;
    }

    return nullptr;
}

Symbol<> TranslationUnit::get_symbol_from_path(const SymbolPath& path, Symbol<> root_sym) const
{
    for (auto& part : path.get_parts())
    {
        root_sym = root_sym->get_child_by_name(part);
        if (!root_sym)
            return nullptr;
    }
    
    return root_sym;
}

const std::string& TranslationUnit::get_file_path() const
{
    return m_file_path;
}