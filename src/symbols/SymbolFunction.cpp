#include "SymbolFunction.h"

SymbolFunction::SymbolFunction(Datatype<> return_type, const Parameter_Decl& parameters, bool is_nodiscard, const qrawlr::Position& position)
    : _Symbol(parameters.get_symbol_path_str(), position),
    m_is_defined(false), m_return_type(return_type),
    m_params(parameters), m_nodiscard(is_nodiscard)
{}

bool SymbolFunction::is_defined() const
{
    return m_is_defined;
}

const qrawlr::Position& SymbolFunction::get_definition_position() const
{
    return m_definition_position;
}

void SymbolFunction::set_defined(const qrawlr::Position& definition_position)
{
    if (is_defined())
        throw std::runtime_error("Function '" + get_symbol_path().to_string() + "' has already been defined!");
    
    m_is_defined = true;
    m_definition_position = definition_position;
}

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