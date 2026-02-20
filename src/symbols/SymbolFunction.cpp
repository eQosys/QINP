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

bool SymbolFunction::is_variadic() const
{
    auto& np = m_params.named_parameters;
    return np.size() > 0 && np.back().datatype->is_variadic();
}

std::pair<size_t, size_t> SymbolFunction::get_param_count_range() const
{
    size_t minCount = m_params.named_parameters.size();
    size_t maxCount = is_variadic() ? -1 : minCount;

    return { minCount, maxCount };
}

int SymbolFunction::get_conversion_score(const std::vector<Expression<>>& params) const
{
    auto paramsCountRange = get_param_count_range();
    if (params.size() < paramsCountRange.first || paramsCountRange.second < params.size())
        return -1; // parameter count does not match function specification

    int cumConvScore = 0;
    size_t variadicIndex = m_params.named_parameters.size() - 1;
    
    for (size_t i = 0; i < params.size(); ++i)
    {
        auto& targetParam = m_params.named_parameters[std::min(i, variadicIndex)];

        int convScore = -1;

        if (targetParam.datatype->is_variadic())
            convScore = 1;
        else if (targetParam.datatype->is_macro())
            convScore = 1; // TODO: Proper macro handling
        else
            convScore = 0; // TODO: Proper conversion handling

        if (convScore < 0)
            return -1;

        cumConvScore += convScore;
    }

    return cumConvScore;
}

std::string SymbolFunction::get_digraph_impl_text(bool verbose) const
{
    std::string res = SymbolSpace::get_digraph_impl_text(verbose);
    res += "ReturnType: " + escape_string(m_return_type->get_symbol_name()) + "\n";
    res += "Params: " + gen_params_digraph_text() + "\n";
    res += "NoDiscard: ";
    res += m_nodiscard ? "YES\n" : "no\n";
    return res;
}

std::string SymbolFunction::get_symbol_type_str() const
{
    return "FUNCTION";
}

std::string SymbolFunction::gen_params_digraph_text() const
{
    std::string res;

    auto it = m_params.named_parameters.begin();
    while (it != m_params.named_parameters.end())
    {
        res += it->datatype->get_symbol_name();
        res += " ";
        res += it->name;

        ++it;

        if (it != m_params.named_parameters.end())
            res += ", ";
    }

    return res;
}

bool SymbolFunctionSpecialization::is_object() const
{
    throw std::runtime_error("[*SymbolFunctionSpecification::is_object*]: Not implemented yet!");
}

Datatype<> SymbolFunctionSpecialization::get_datatype() const
{
    throw std::runtime_error("[*SymbolFunctionSpecification::get_datatype*]: Not implemented yet!");
}

std::string SymbolFunctionSpecialization::get_digraph_impl_text(bool verbose) const
{
    std::string res = SymbolFunction::get_digraph_impl_text(verbose);
    // nothing to do
    return res;
}

std::string SymbolFunctionSpecialization::get_symbol_type_str() const
{
    return "FUNCTION SPECIFICATION";
}

CodeBlock& SymbolFunctionSpecialization::set_definition(const qrawlr::Position& definition_position)
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