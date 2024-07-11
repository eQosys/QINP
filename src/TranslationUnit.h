#pragma once

#include <vector>

#include "symbols/SymbolSpace.h"

class TranslationUnit
{
public:
    TranslationUnit(const std::string& path, SymbolRef root_sym);
public:
    void enter_symbol(SymbolRef sym);
    void leave_symbol();
    SymbolRef curr_symbol() const;
    SymbolRef get_symbol(const std::string& path, bool local_only, bool from_root = false) const;
public:
    const std::string& path() const;
private:
    std::string m_path;
    SymbolRef m_root_sym;
    std::vector<SymbolRef> m_sym_stack;
};