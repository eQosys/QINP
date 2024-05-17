#pragma once

#include "libQrawlr.h"

#include "symbols/SymbolSpace.h"

class TranslationUnit
{
public:
    TranslationUnit(const std::string& path)
        : m_path(path), m_root_sym(Symbol::make<SymbolSpace>("<global>", Location(path))), m_sym_stack()
    {
        m_sym_stack.push(m_root_sym);
    }
public:
    void enter_symbol(SymbolRef sym) { m_sym_stack.push(sym); }
    void leave_symbol() { m_sym_stack.pop(); }
    SymbolRef curr_sym() const { return m_sym_stack.top(); }
public:
    const std::string& path() const { return m_path; }
private:
    std::string m_path;
    SymbolRef m_root_sym;
    std::stack<SymbolRef> m_sym_stack;
};