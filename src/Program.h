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
    void import_source_file(std::string path_str, bool skip_duplicate);
    void import_source_code(const std::string& code_str);
private:
    bool m_verbose;
    std::set<std::filesystem::path> m_import_dirs;
    std::set<std::string> m_imported_files;
};