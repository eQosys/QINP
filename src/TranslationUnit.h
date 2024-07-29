#pragma once

#include <vector>

#include "utility/SymbolPath.h"
#include "symbols/SymbolSpace.h"

class TranslationUnit
{
public:
    TranslationUnit(const std::string& path, const SymbolRef root_sym);
public:
    void enter_symbol(SymbolRef sym);
    void leave_symbol();
    SymbolRef curr_symbol() const;
    SymbolRef get_symbol_by_path(const std::string& path_str, bool local_only, bool from_root = false) const;
    SymbolRef get_symbol_by_path(const SymbolPath& path, SymbolRef root_sym) const;
public:
    const std::string& get_path() const;
private:
    const std::string m_path;
    const SymbolRef m_root_sym;
    std::vector<SymbolRef> m_sym_stack;
};