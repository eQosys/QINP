#include "TranslationUnit.h"

TranslationUnit::TranslationUnit(const std::string& path, SymbolRef root_sym)
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

SymbolRef TranslationUnit::get_symbol(const std::string& name, bool local_only, bool from_root) const
{
    if (from_root)
        return m_root_sym->get_child(name);

    for (auto it = m_sym_stack.rbegin(); it != m_sym_stack.rend(); ++it)
    {
        auto sym = (*it)->get_child(name);
        if (sym)
            return sym;
        if (local_only)
            break;
    }

    return nullptr;
}

const std::string& TranslationUnit::path() const
{
    return m_path;
}