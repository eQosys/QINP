#include "Parameters.h"

#include "StringHelpers.h"

std::string Parameter_Decl::get_symbol_name() const
{
    std::vector<std::string> params;
    for (auto& param : named_parameters)
        params.push_back(param.datatype->get_symbol_name());

    return std::string(is_blueprint ? "$__BLUEPRINT__$." : "") + "<" + join(params.begin(), params.end(), ",") + ">";
}