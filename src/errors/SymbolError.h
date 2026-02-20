#pragma once

#include "symbols/Symbol.h"
#include "QinpError.h"

class SymbolError : public QinpError
{
public:
    SymbolError(const std::string& what, Symbol<> symbol)
        : QinpError(symbol->get_position().to_string() + ": " + what)
    {}
public:
    using QinpError::QinpError;
};