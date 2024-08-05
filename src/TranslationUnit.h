#pragma once

#include <vector>

#include "errors/SymbolError.h"
#include "utility/SymbolPath.h"
#include "symbols/SymbolSpace.h"

typedef std::shared_ptr<class TranslationUnit> TranslationUnitRef;

class TranslationUnit
{
public:
    TranslationUnit(const std::string& path, const Symbol<> root_sym);
public:
    void enter_symbol(Symbol<> sym);
    void leave_symbol();
    Symbol<> curr_symbol() const;
    Symbol<> get_symbol_from_path(const SymbolPath& path) const;
    Symbol<> get_symbol_from_path(const SymbolPath& path, Symbol<> root_sym) const;
public:
    const std::string& get_file_path() const;
private:
    const std::string m_file_path;
    const Symbol<> m_root_sym;
    std::vector<Symbol<>> m_sym_stack;
};