#pragma once

#include <string>

#include "Symbol.h"

class SymbolSpace : public _Symbol
{
    using _Symbol::_Symbol;
public:
    virtual bool is_object() const override;
    virtual Datatype<> get_datatype() const override;
};