#include "ExpressionFunctionCall.h"

ExpressionFunctionCall::ExpressionFunctionCall(Symbol<SymbolFunction> funcSym, const std::vector<Expression<>>& params)
{
    m_matchLevel.category = MatchLevel::Category::NoMatch;
    
    if ((m_matchLevel.score = funcSym->get_conversion_score(params)) < 0)
        return;
    
    if (funcSym.is_of_type<SymbolFunctionSpecialization>())
        m_matchLevel.category = MatchLevel::Category::Specialization;
    else if (funcSym.is_of_type<SymbolFunctionBlueprint>())
        m_matchLevel.category = MatchLevel::Category::BlueprintGenerated;
    else // Unknown symbol type
        m_matchLevel.category = MatchLevel::Category::NoMatch;
}

bool ExpressionFunctionCall::is_const_expr() const
{
    // TODO: Implementation
    return false;
}

const ExpressionFunctionCall::MatchLevel& ExpressionFunctionCall::get_match_level() const
{
    return m_matchLevel;
}

bool ExpressionFunctionCall::MatchLevel::operator<(const MatchLevel& other) const
{
    if ((int)category != (int)other.category)
        return (int)category < (int)other.category;
    return score < other.score;
}

bool ExpressionFunctionCall::MatchLevel::operator>(const MatchLevel& other) const
{
    if ((int)category != (int)other.category)
        return (int)category > (int)other.category;
    return score > other.score;
}

bool ExpressionFunctionCall::MatchLevel::operator==(const MatchLevel& other) const
{
    if (category != other.category)
        return false;
    if (score != other.score)
        return false;
    return true;
}