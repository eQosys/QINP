#pragma once

#include "Datatype.h"

struct Parameter
{
    Datatype datatype;
    std::string name;
};

struct Parameter_Decl
{
    std::vector<Parameter> named_parameters;
    bool has_variadic_parameters = false;
};