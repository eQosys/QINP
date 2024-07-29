#include "TranslationUnit.h"

TranslationUnit::TranslationUnit(const std::string& path, const SymbolRef root_sym)
    : m_path(path), m_root_sym(root_sym), m_sym_stack()
{
    m_sym_stack.push_back(m_root_sym);
}

void TranslationUnit::enter_symbol(SymbolRef sym)
{
    m_sym_stack.push_back(sym);
}

void TranslationUnit::leave_symbol()
{
    m_sym_stack.pop_back();
}

SymbolRef TranslationUnit::curr_symbol() const
{
    return m_sym_stack.back();
}

SymbolRef TranslationUnit::get_symbol_by_path(const std::string& path_str, bool local_only, bool from_root) const
{
    SymbolPath path(path_str);

    if (from_root)
        return get_symbol_by_path(path, m_root_sym);

    for (auto it = m_sym_stack.rbegin(); it != m_sym_stack.rend(); ++it)
    {
        auto sym = get_symbol_by_path(path, *it);
        if (sym)
            return sym;
        if (local_only)
            break;
    }

    return nullptr;
}

SymbolRef TranslationUnit::get_symbol_by_path(const SymbolPath& path, SymbolRef root_sym) const
{
    for (auto& part : path.get_parts())
    {
        root_sym = root_sym->get_child_by_name(part);
        if (!root_sym)
            return nullptr;
    }
    
    return root_sym;
}

const std::string& TranslationUnit::get_path() const
{
    return m_path;
}