#pragma once

#include "Symbol.h"

#include "utility/Datatype.h"
#include "utility/CodeBlock.h"
#include "utility/Parameters.h"

class SymbolFunctionName : public _Symbol
{
    using _Symbol::_Symbol;
};

class SymbolFunction : public _Symbol
{
    using _Symbol::_Symbol;
public:
    bool is_defined() const;
private:
    bool m_is_defined;

    Datatype m_return_type;
    Parameter_Decl m_params;
    bool m_nodiscard;
    CodeBlock m_body;
};