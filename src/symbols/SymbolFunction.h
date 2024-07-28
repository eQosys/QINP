#pragma once

#include "Symbol.h"

#include "Datatype.h"

class SymbolFunctionName : public Symbol
{
    using Symbol::Symbol;
};

class SymbolFunction : public Symbol
{
    using Symbol::Symbol;
private:
    Datatype m_return_type;
    Parameters m_params;
    CodeBlock m_body;
};