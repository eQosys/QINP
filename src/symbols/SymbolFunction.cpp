#include "SymbolFunction.h"

bool SymbolFunctionName::is_object() const
{
    throw std::runtime_error("[*SymbolFunctionName::is_object*]: Not implemented yet!");
}

Datatype<> SymbolFunctionName::get_datatype() const
{
    throw std::runtime_error("[*SymbolFunctionName::get_datatype*]: Not implemented yet!");
}

SymbolFunction::SymbolFunction(Datatype<> return_type, const Parameter_Decl& parameters, bool is_nodiscard, const qrawlr::Position& position)
    : SymbolSpace(parameters.get_symbol_path_str(), position),
    m_return_type(return_type),
    m_params(parameters), m_nodiscard(is_nodiscard)
{}

bool SymbolFunctionSpecification::is_object() const
{
    throw std::runtime_error("[*SymbolFunctionSpecification::is_object*]: Not implemented yet!");
}

Datatype<> SymbolFunctionSpecification::get_datatype() const
{
    throw std::runtime_error("[*SymbolFunctionSpecification::get_datatype*]: Not implemented yet!");
}

CodeBlock& SymbolFunctionSpecification::set_definition(const qrawlr::Position& definition_position)
{
    set_defined(definition_position);
    m_body = {};
    return m_body;
}

bool SymbolFunctionBlueprint::is_object() const
{
    throw std::runtime_error("[*SymbolFunctionBlueprint::is_object*]: Not implemented yet!");
}

Datatype<> SymbolFunctionBlueprint::get_datatype() const
{
    throw std::runtime_error("[*SymbolFunctionBlueprint::get_datatype*]: Not implemented yet!");
}

void SymbolFunctionBlueprint::set_definition(qrawlr::ParseTreeRef body_tree)
{
    set_defined(body_tree->get_pos_begin());
    m_body_tree = body_tree;
}