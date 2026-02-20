#include "QinpError.h"

#include "expressions/Expression.h"

QinpError QinpError::from_pos(const std::string& message, const qrawlr::Position& position)
{
    return QinpError(qrawlr::GrammarException(message, position).what());
}

QinpError QinpError::from_node(const std::string& message, qrawlr::ParseTreeRef elem)
{
    return from_pos(message, elem->get_pos_begin());
}

QinpError QinpError::from_expr(const std::string& message, Expression<> expr)
{
    return from_pos(message, expr->get_position());
}