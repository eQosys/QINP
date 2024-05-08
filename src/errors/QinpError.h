#pragma once

#include <stdexcept>

class QinpError : public std::runtime_error
{
public:
    QinpError(const std::string& what)
        : std::runtime_error(what)
    {}
};