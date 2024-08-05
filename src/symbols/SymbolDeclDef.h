#pragma once

#include "Symbol.h"

class _SymbolDeclDef : public _Symbol
{
public:
    _SymbolDeclDef(const std::string& name, const qrawlr::Position& declaration_position);
public:
    bool is_defined() const;
    const qrawlr::Position& get_definition_position() const;
protected:
    void set_defined(const qrawlr::Position& definition_position);
private:
    bool m_is_defined;
    qrawlr::Position m_definition_position;
};
