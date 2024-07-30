#pragma once

#include <string>
#include <vector>

class SymbolPath
{
public:
    SymbolPath() = default;
    SymbolPath(const std::string& path);
    SymbolPath(const std::vector<std::string>& parts, bool is_from_root);
public:
    std::string to_string() const;
    bool is_from_root() const;
    const std::vector<std::string>& get_parts() const;
public:
    std::string get_name() const;
    SymbolPath get_parent_path() const;
private:
    bool m_from_root;
    std::vector<std::string> m_parts;
};