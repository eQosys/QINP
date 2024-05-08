#include "Program.h"

#include "FileReader.h"

Program::Program(bool verbose)
    : m_verbose(verbose)
{
    ;
}

void Program::add_import_directory(const std::string& path_str)
{
    auto path = std::filesystem::path(path_str);
    if (!std::filesystem::is_directory(path))
        throw 1; // TODO: throw proper exception when path is not a directory
    m_import_dirs.insert(path);
}

void Program::import_source_file(std::string path_str, bool skip_duplicate)
{
    path_str = std::filesystem::canonical(path_str).generic_string();
    if (skip_duplicate && m_imported_files.find(path_str) != m_imported_files.end())
        return; // skip duplicate import
    
    std::string code_str = read_file(path_str);

    m_imported_files.insert(path_str);
    import_source_code(code_str);
}

void Program::import_source_code(const std::string& code_str)
{
    ;
}