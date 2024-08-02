#pragma once

#include "Symbol.h"

#include "utility/Datatype.h"
#include "utility/CodeBlock.h"
#include "utility/Parameters.h"

class SymbolFunctionName : public _Symbol
{
    using _Symbol::_Symbol;
};

class SymbolFunction : public _Symbol
{
public:
    SymbolFunction(Datatype<> return_type, const Parameter_Decl& parameters, bool is_nodiscard, const qrawlr::Position& position);
public:
    bool is_defined() const;
    const qrawlr::Position& get_definition_position() const;
protected:
    void set_defined(const qrawlr::Position& definition_position);
private:
    bool m_is_defined;
    qrawlr::Position m_definition_position;

    Datatype<> m_return_type;
    Parameter_Decl m_params;
    bool m_nodiscard;
};

class SymbolFunctionSpecification : public SymbolFunction
{
public:
    using SymbolFunction::SymbolFunction;
public:
    CodeBlock& set_definition(const qrawlr::Position& definition_position);
private:
    CodeBlock m_body;
};

class SymbolFunctionBlueprint : public SymbolFunction
{
public:
    using SymbolFunction::SymbolFunction;
public:
    void set_definition(qrawlr::ParseTreeRef body_tree);
private:
    qrawlr::ParseTreeRef m_body_tree;
};