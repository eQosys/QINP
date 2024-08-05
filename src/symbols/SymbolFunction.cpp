#include "SymbolFunction.h"

SymbolFunction::SymbolFunction(Datatype<> return_type, const Parameter_Decl& parameters, bool is_nodiscard, const qrawlr::Position& position)
    : _SymbolDeclDef(parameters.get_symbol_path_str(), position),
    m_return_type(return_type),
    m_params(parameters), m_nodiscard(is_nodiscard)
{}

CodeBlock& SymbolFunctionSpecification::set_definition(const qrawlr::Position& definition_position)
{
    set_defined(definition_position);
    m_body = {};
    return m_body;
}

void SymbolFunctionBlueprint::set_definition(qrawlr::ParseTreeRef body_tree)
{
    set_defined(body_tree->get_pos_begin());
    m_body_tree = body_tree;
}