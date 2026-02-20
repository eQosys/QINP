#include "ExpressionIf.h"

#include "errors/QinpError.h"
#include "utility/SymbolPath.h"

ExpressionIf::ExpressionIf(Expression<> condition, const SymbolPath& spacePath, const qrawlr::Position& position)
    : _Expression(DT_VOID(), position)
{
    m_condition = condition;
    if (!condition->results_in_object() || condition->get_datatype()->is_void())
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