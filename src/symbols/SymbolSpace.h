#pragma once

#include <string>

#include "SymbolDeclDef.h"
#include "utility/CodeBlock.h"

class SymbolSpace : public _SymbolDeclDef
{
    using _SymbolDeclDef::_SymbolDeclDef;
public:
    virtual bool is_object() const override;
    virtual Datatype<> get_datatype() const override;
protected:
    virtual std::string get_digraph_impl_text(bool verbose) const override;
    virtual std::string get_symbol_type_str() const override;
public:
    void set_definition(const qrawlr::Position& definition_position);
public:
    void append_expr(Expression<> expr);
private:
    CodeBlock m_body;
public:
    static std::string gen_unique_space_name(const std::string& prefix);
};