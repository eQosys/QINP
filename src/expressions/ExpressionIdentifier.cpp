#include "ExpressionIdentifier.h"

ExpressionIdentifier::ExpressionIdentifier(const std::string& name, const qrawlr::Position& position)
    : _Expression(DT_NAMED("void", false), position),
    m_name(name)
{}

const std::string& ExpressionIdentifier::get_name() const
{
    return m_name;
}

bool ExpressionIdentifier::is_const_expr() const
{
    return true;
}

bool ExpressionIdentifier::results_in_object() const
{
    return false;
}