#pragma once

#include "symbols/Symbol.h"

class SymbolString : public _Symbol
{
public:
    SymbolString(const std::string& name, const std::string& value, const qrawlr::Position& position);
public:
    virtual bool is_object() const override;
    virtual Datatype<> get_datatype() const override;
protected:
    virtual std::string get_digraph_impl_text(bool verbose) const override;
    virtual std::string get_symbol_type_str() const override;
private:
    std::string m_value;
};