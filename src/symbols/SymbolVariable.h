#pragma once

#include "SymbolDeclDef.h"
#include "utility/Datatype.h"
#include "expressions/Expression.h"

struct VariableDeclarators
{
    bool is_static;
    bool is_const;
};

class SymbolVariable : public _SymbolDeclDef
{
public:
    SymbolVariable(const std::string& name, const VariableDeclarators& declarators, Datatype<> datatype, Expression<> initializer, const qrawlr::Position& position);
private:
    bool m_is_static;
    Datatype<> m_datatype;
    Expression<> m_initializer;
};