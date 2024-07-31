#include "Parameters.h"

#include "StringHelpers.h"
#include "symbols/Symbol.h"

std::string Parameter_Decl::get_symbol_path_str() const
{
    std::vector<std::string> params;
    for (auto& param : named_parameters)
        params.push_back(param.datatype->get_symbol_name());

    return std::string(is_blueprint ? SYMBOL_NAME_BLUEPRINT "." : "") + "<" + join(params.begin(), params.end(), ",") + ">";
}