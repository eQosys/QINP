#include "SymbolSpace.h"

bool SymbolSpace::is_object() const
{
    throw std::runtime_error("[*SymbolSpace::is_object*]: Not implemented yet!");
}

Datatype<> SymbolSpace::get_datatype() const
{
    throw std::runtime_error("[*SymbolSpace::get_datatype*]: Not implemented yet!");
}

void SymbolSpace::set_definition(const qrawlr::Position& definition_position)
{
    set_defined(definition_position);
}