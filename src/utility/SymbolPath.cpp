#include "SymbolPath.h"

#include "StringHelpers.h"

SymbolPath::SymbolPath(const std::string& path)
{
    // TODO: Proper parsing
    
    m_from_root = (path.find('.') == 0);

    std::size_t last_sep = m_from_root ? 1 : 0;
    std::size_t sep = path.find('.', last_sep);
    do
    {
        m_parts.push_back(path.substr(last_sep, sep - last_sep));
        last_sep = sep;
        sep = path.find(last_sep + 1);
    } while (sep != std::string::npos);
}

SymbolPath::SymbolPath(const std::vector<std::string>& parts, bool is_from_root)
    : m_from_root(is_from_root), m_parts(parts)
{}

std::string SymbolPath::to_string() const
{
    return (m_from_root ? "." : "") + join(m_parts.begin(), m_parts.end(), ".");
}

bool SymbolPath::is_from_root() const
{
    return m_from_root;
}

const std::vector<std::string>& SymbolPath::get_parts() const
{
    return m_parts;
}


std::string SymbolPath::get_name() const
{
    return m_parts.back();
}

SymbolPath SymbolPath::get_parent_path() const
{
    auto parts = m_parts;
    parts.pop_back();
    return SymbolPath(parts, m_from_root);
}