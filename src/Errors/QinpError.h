#pragma once

#include <string>

class QinpError
{
public:
    QinpError(const std::string& what, const std::string& srcPos)
        : m_what(what), m_where(srcPos)
    {}
    QinpError(const std::string& what, const std::string& srcFile, int srcLine)
        : QinpError(what, srcFile + ":" + std::to_string(srcLine))
    {}
public:
    const std::string& what() const { return m_what; }
    const std::string where() const { return m_where;}
private:
    std::string m_what;
    std::string m_where;
};

#define MAKE_QINP_ERROR(what) QinpError(what, __FILE__, __LINE__)
#define THROW_QINP_ERROR(what) throw MAKE_QINP_ERROR(what)