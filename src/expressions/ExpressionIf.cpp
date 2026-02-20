#include "ExpressionIf.h"

#include "errors/QinpError.h"
#include "symbols/SymbolSpace.h"

ExpressionIf::ExpressionIf(Expression<> condition, Symbol<const SymbolSpace> body_true, const qrawlr::Position& position)
    : _Expression(DT_VOID(), position),
    m_condition(condition), m_body_true(body_true)
{
    m_condition = condition;
    if (!condition->results_in_object())
        throw QinpError::from_pos("", position);
}

bool ExpressionIf::is_const_expr() const
{
    return m_condition->is_const_expr();
}

bool ExpressionIf::results_in_object() const
{
    return false;
}

void ExpressionIf::set_body_false(Symbol<const SymbolSpace> body_false)
{
    m_body_false = body_false;
}