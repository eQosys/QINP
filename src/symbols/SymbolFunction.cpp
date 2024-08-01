#include "SymbolFunction.h"

SymbolFunction::SymbolFunction(Datatype<> return_type, const Parameter_Decl& parameters, bool is_nodiscard, const qrawlr::Position& position)
    : _Symbol(parameters.get_symbol_path_str(), position),
    m_is_defined(false), m_return_type(return_type),
    m_params(parameters), m_nodiscard(is_nodiscard)
{}

bool SymbolFunction::is_defined() const
{
    return m_is_defined;
}