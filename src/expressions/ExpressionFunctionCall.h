#pragma once

#include "Expression.h"

class ExpressionFunctionCall : public _Expression
{
public:
    ExpressionFunctionCall();
public:
    virtual bool is_const_expr() const override;
    virtual bool results_in_object() const override;
public:
    class MatchLevel get_match_level() const;
private:
    enum class MatchLevel
    {
        NoMatch,
        Exact,
    } m_match_level = MatchLevel::NoMatch;
};