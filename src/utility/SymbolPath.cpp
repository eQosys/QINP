#include "SymbolPath.h"

#include "StringHelpers.h"
#include "errors/QinpError.h"

SymbolPath::SymbolPath(const std::string& path)
{
    // TODO: Proper parsing

    if (path.empty())
    {
        return;
    }
    if (path == ".")
    {
        m_from_root = true;
        return;
    }
    
    m_from_root = (path.find('.') == 0);

    std::size_t start_pos = m_from_root ? 1 : 0;
    std::size_t sep = path.find('.', start_pos);
    do
    {
        m_parts.push_back(path.substr(start_pos, sep - start_pos));
        start_pos = sep + 1;
        sep = path.find('.', start_pos);
    } while (start_pos != 0);
}

SymbolPath::SymbolPath(const std::vector<std::string>& parts, bool is_from_root)
    : m_from_root(is_from_root), m_parts(parts)
{}

SymbolPath& SymbolPath::enter(const SymbolPath& to_enter)
{
    if (to_enter.is_from_root())
        *this = to_enter;
    else
        m_parts.insert(m_parts.end(), to_enter.m_parts.begin(), to_enter.m_parts.end());

    return *this;
}

SymbolPath& SymbolPath::leave()
{
    if (m_parts.empty())
        throw QinpError("Cannot get parent path of (relative) root!");

    m_parts.pop_back();

    return *this;
}

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
    auto other = *this;
    return other.leave();
}