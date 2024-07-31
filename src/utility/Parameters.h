#pragma once

#include "Datatype.h"

struct Parameter
{
    Datatype<> datatype;
    std::string name;
};

struct Parameter_Decl
{
    std::vector<Parameter> named_parameters;
    bool is_blueprint = false;

    std::string get_symbol_path_str() const;
};