#include "Expression.h"

#include "errors/QinpError.h"

_Expression::_Expression(Datatype<> datatype, const qrawlr::Position& position)
    : m_position(position), m_datatype(datatype)
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
    return false;
}

CEObject _Expression::eval_const_expr() const
{
    throw QinpError::from_pos("Constexpr not implemented for this type", get_position());
}

bool _Expression::results_in_object() const
{
    return !m_datatype->is_void();
}