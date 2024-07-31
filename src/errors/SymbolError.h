#pragma once

#include "symbols/Symbol.h"
#include "QinpError.h"

class SymbolError : public QinpError
{
public:
    SymbolError(Symbol<> sym, const std::string& what)
        : QinpError(sym->get_position().to_string() + ": " + what)
    {}
public:
    using QinpError::QinpError;
};