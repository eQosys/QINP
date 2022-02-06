#pragma once

#include <string>

class QinpError
{
public:
    QinpError(const std::string& what)
        : m_what(what)
    {}
public:
    const std::string& what() const { return m_what; }
private:
    std::string m_what;
};