#pragma once

#include "Expression.h"
#include "symbols/SymbolFunction.h"

class ExpressionFunctionCall : public _Expression
{
public:
    class MatchLevel;
public:
    ExpressionFunctionCall(Symbol<SymbolFunction> funcSym, const std::vector<Expression<>>& params);
public:
    virtual bool is_const_expr() const override;
    virtual bool results_in_object() const override;
public:
    const MatchLevel& get_match_level() const;
private:
    Symbol<> m_funcSym;
    const std::vector<Expression<>> m_params;
    class MatchLevel
    {
    public:
        bool operator<(const MatchLevel& other) const;
        bool operator>(const MatchLevel& other) const;
        bool operator==(const MatchLevel& other) const;
    public:
        enum class Category
        {
            Specialization = -2,
            BlueprintGenerated = -1,
            NoMatch = 0
        };
    public:
        Category category = Category::NoMatch;
        int score = 0;
    } m_matchLevel;
};