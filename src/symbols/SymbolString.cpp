#include "SymbolString.h"

#include "utility/StringEscape.h"

SymbolString::SymbolString(const std::string &name, const std::string &value, const qrawlr::Position &position)
    : _Symbol(name, position),
    m_value(value)
{
}

bool SymbolString::is_object() const
{
    return true;
}

Datatype<> SymbolString::get_datatype() const
{
    return DT_ARRAY(m_value.size(), DT_NAMED("u8", true), true);
}

std::string SymbolString::get_digraph_impl_text(bool verbose) const
{
    std::string res = _Symbol::get_digraph_impl_text(verbose);
    res += "Text: ";
    res += escape_string(m_value);
    return res;
}

std::string SymbolString::get_symbol_type_str() const
{
    return "STRING";
}
