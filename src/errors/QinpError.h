#pragma once

#include <stdexcept>
#include "libQrawlr.h"

class QinpError : public std::runtime_error
{
public:
    QinpError(const std::string& what)
        : std::runtime_error("QNP: " + what)
    {}
public:
    static QinpError from_pos(const std::string& message, const qrawlr::Position& position);
    static QinpError from_node(const std::string& message, qrawlr::ParseTreeRef elem);
    static QinpError from_expr(const std::string& message, class Expression<> expr);
};