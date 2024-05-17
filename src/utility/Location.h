#pragma once

#include <map>
#include <string>

#include "libQrawlr.h"

class Location
{
public:
    Location() = default;
    Location(const std::string& path)
        : m_path(path), m_line(0), m_col(0)
    {}
    Location(const std::string& path, int line, int col)
        : m_path(path), m_line(line), m_col(col)
    {}
public:
    const std::string& path() const { return m_path; }
    int line() const { return m_line; }
    int col() const { return m_col; }
public:
    std::string to_string() const { return m_path + ":" + std::to_string(m_line) + ":" + std::to_string(m_col); }
public:
    static Location from_qrawlr(const std::string& path, const qrawlr::Position& pos)
    {
        return Location(path, pos.line, pos.column);
    }
private:
    std::string m_path;
    int m_line;
    int m_col;
};