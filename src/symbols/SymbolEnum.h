#pragma once

#include "SymbolDeclDef.h"

class SymbolEnum : public _SymbolDeclDef
{
public:
    SymbolEnum(const std::string& name, const qrawlr::Position& declaration_position);
public:
    void set_definition(const qrawlr::Position& definition_position);
};

class SymbolEnumMember : public _Symbol
{
    ;
};