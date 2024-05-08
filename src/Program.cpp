#include "Program.h"

Program::Program(bool verbose)
    : m_verbose(verbose)
{
    ;
}

void Program::add_import_directory(const std::string& path_str)
{
    auto path = std::filesystem::path(path_str);
    if (!std::filesystem::is_directory(path))
        throw 1; // TODO: throw proper exception if path is not a directory
    m_import_dirs.insert(path);
}

void Program::add_source_file(const std::string& path_str)
{
    ;
}