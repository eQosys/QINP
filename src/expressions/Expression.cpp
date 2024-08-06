#include "Expression.h"

#include "errors/QinpError.h"

_Expression::_Expression(Datatype<> datatype, const qrawlr::Position& position)
    : m_datatype(datatype), m_position(position)
{}

Datatype<> _Expression::get_datatype() const
{
    return m_datatype;
}

const qrawlr::Position& _Expression::get_position() const
{
    return m_position;
}

bool _Expression::is_const_expr() const
{
    throw QinpError("[*_Expression::is_const_expr*]: Non-overloaded version not callable!");
    return true;
}