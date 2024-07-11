#pragma once

#include "symbols/Symbol.h"
#include "QinpError.h"

class SymbolError : public QinpError
{
public:
    SymbolError(SymbolRef sym, const std::string& what)
        : QinpError(sym->location().to_string() + ": " + what)
    {}
public:
    using QinpError::QinpError;
};