#pragma once

#include "symbols/Symbol.h"
#include "QinpError.h"

class SymbolError : public QinpError
{
public:
    SymbolError(const std::string& what, Symbol<> symbol, std::function<std::string(int)> tree_id_to_name)
        : QinpError(symbol->get_position().to_string(tree_id_to_name) + ": " + what)
    {}
public:
    using QinpError::QinpError;
};