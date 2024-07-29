#pragma once

#include "Symbol.h"

#include "utility/Datatype.h"
#include "utility/CodeBlock.h"
#include "utility/Parameters.h"

class SymbolFunctionName : public Symbol
{
    using Symbol::Symbol;
};

class SymbolFunction : public Symbol
{
    using Symbol::Symbol;
private:
    Datatype m_return_type;
    Parameter_Decl m_params;
    CodeBlock m_body;
};