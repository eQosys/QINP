#include "ExpressionFunctionCall.h"

ExpressionFunctionCall::ExpressionFunctionCall()
{
    ;
}

bool ExpressionFunctionCall::is_const_expr() const
{
    // TODO: Implementation
    return false;
}

bool ExpressionFunctionCall::results_in_object() const
{
    return m_datatype->is_void();
}