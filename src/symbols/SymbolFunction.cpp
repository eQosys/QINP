#include "SymbolFunction.h"

#include "utility/StringEscape.h"

bool SymbolFunctionName::is_object() const
{
    return false;
}

Datatype<> SymbolFunctionName::get_datatype() const
{
    return DT_FUNCTION_NAME();
}

std::string SymbolFunctionName::get_digraph_impl_text(bool verbose) const
{
    std::string res = _Symbol::get_digraph_impl_text(verbose);
    // nothing to do
    return res;
}

std::string SymbolFunctionName::get_symbol_type_str() const
{
    return "FUNCTION NAME";
}

SymbolFunction::SymbolFunction(Datatype<> return_type, const Parameter_Decl& parameters, bool is_nodiscard, const qrawlr::Position& position)
    : SymbolSpace(parameters.get_symbol_path_str(), position),
    m_return_type(return_type),
    m_params(parameters), m_nodiscard(is_nodiscard)
{}

std::string SymbolFunction::get_digraph_impl_text(bool verbose) const
{
    std::string res = SymbolSpace::get_digraph_impl_text(verbose);
    res += "ReturnType: " + escape_string(m_return_type->get_symbol_name()) + "\n";
    res += "Params: <to-implement>\n";
    res += "NoDiscard: ";
    res += m_nodiscard ? "YES\n" : "no\n";
    return res;
}

std::string SymbolFunction::get_symbol_type_str() const
{
    return "FUNCTION";
}

bool SymbolFunctionSpecification::is_object() const
{
    throw std::runtime_error("[*SymbolFunctionSpecification::is_object*]: Not implemented yet!");
}

Datatype<> SymbolFunctionSpecification::get_datatype() const
{
    throw std::runtime_error("[*SymbolFunctionSpecification::get_datatype*]: Not implemented yet!");
}

std::string SymbolFunctionSpecification::get_digraph_impl_text(bool verbose) const
{
    std::string res = SymbolFunction::get_digraph_impl_text(verbose);
    // nothing to do
    return res;
}

std::string SymbolFunctionSpecification::get_symbol_type_str() const
{
    return "FUNCTION SPECIFICATION";
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

std::string SymbolFunctionBlueprint::get_digraph_impl_text(bool verbose) const
{
    std::string res = SymbolFunction::get_digraph_impl_text(verbose);
    // nothing to do
    return res;
}

std::string SymbolFunctionBlueprint::get_symbol_type_str() const
{
    return "FUNCTION BLUEPRINT";
}

void SymbolFunctionBlueprint::set_definition(qrawlr::ParseTreeRef body_tree)
{
    set_defined(body_tree->get_pos_begin());
    m_body_tree = body_tree;
}