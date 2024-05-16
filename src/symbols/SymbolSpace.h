#pragma once

#include <string>

#include "Symbol.h"

class SymbolSpace : public Symbol
{
public:
    SymbolSpace() = default;
public:
    virtual ~SymbolSpace() = default;
private:
    std::string m_name;
};