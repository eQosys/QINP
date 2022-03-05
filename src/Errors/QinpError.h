#pragma once

#include <string>

class QinpError
{
public:
    QinpError(const std::string& what, const std::string& srcFile, int srcLine)
        : m_what(what), m_where(srcFile + ":" + std::to_string(srcLine))
    {}
public:
    const std::string& what() const { return m_what; }
    const std::string where() const { return m_where;}
private:
    std::string m_what;
    std::string m_where;
};

#define THROW_QINP_ERROR(what) throw QinpError(what, __FILE__, __LINE__)