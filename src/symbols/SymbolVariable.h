#pragma once

#include "SymbolDeclDef.h"
#include "utility/Datatype.h"
#include "expressions/Expression.h"

struct VariableDeclarators
{
    bool is_static = false;
    bool is_const = false;
};

class SymbolVariable : public _SymbolDeclDef
{
public:
    SymbolVariable(const std::string& name, const VariableDeclarators& declarators, Datatype<> datatype, const qrawlr::Position& position);
public:
    virtual bool is_object() const override;
    virtual Datatype<> get_datatype() const override;
private:
    bool m_is_static;
    Datatype<> m_datatype;
};