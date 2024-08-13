#pragma once

#include "SymbolDeclDef.h"

class SymbolEnum : public _SymbolDeclDef
{
public:
    SymbolEnum(const std::string& name, const qrawlr::Position& declaration_position);
public:
    virtual bool is_object() const override;
    virtual Datatype<> get_datatype() const override;
public:
    std::size_t next_value();
    void set_last_value(std::size_t value);
private:
    std::size_t m_last_value;
};

class SymbolEnumMember : public _Symbol
{
public:
    SymbolEnumMember(const std::string& name, std::size_t value, const qrawlr::Position& position);
public:
    virtual bool is_object() const override;
    virtual Datatype<> get_datatype() const override;
public:
    std::size_t get_value() const;
private:
    std::size_t m_value;
};