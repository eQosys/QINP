#pragma once

#include "Expression.h"

class ExpressionIdentifier : public _Expression
{
public:
    ExpressionIdentifier(const std::string& name, const qrawlr::Position& position);
public:
    const std::string& get_name() const;
public:
    virtual bool is_const_expr() const override;
private:
    std::string m_name;
};