#pragma once

#include <set>
#include <filesystem>
#include <string>

class Program
{
public:
    Program() = delete;
    Program(bool verbose);
public:
    void add_import_directory(const std::string& path_str);
    void add_source_file(const std::string& path_str);
private:
    bool m_verbose;
    std::set<std::filesystem::path> m_import_dirs;
};