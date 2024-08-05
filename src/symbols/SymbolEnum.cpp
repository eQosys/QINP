#include "SymbolEnum.h"

SymbolEnum::SymbolEnum(const std::string& name, const qrawlr::Position& declaration_position)
    : _SymbolDeclDef(name, declaration_position)
{}

void SymbolEnum::set_definition(const qrawlr::Position& definition_position)
{
    set_defined(definition_position);
}