#pragma once

#include "SymbolSpace.h"

#include "utility/Datatype.h"
#include "utility/CodeBlock.h"
#include "utility/Parameters.h"

class SymbolFunctionName : public _Symbol
{
    using _Symbol::_Symbol;
public:
    virtual bool is_object() const override;
    virtual Datatype<> get_datatype() const override;
};

class SymbolFunction : public SymbolSpace
{
public:
    SymbolFunction(Datatype<> return_type, const Parameter_Decl& parameters, bool is_nodiscard, const qrawlr::Position& position);
private:
    Datatype<> m_return_type;
    Parameter_Decl m_params;
    bool m_nodiscard;
};

class SymbolFunctionSpecification : public SymbolFunction
{
public:
    using SymbolFunction::SymbolFunction;
public:
    virtual bool is_object() const override;
    virtual Datatype<> get_datatype() const override;
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
    virtual bool is_object() const override;
    virtual Datatype<> get_datatype() const override;
public:
    void set_definition(qrawlr::ParseTreeRef body_tree);
private:
    qrawlr::ParseTreeRef m_body_tree;
};