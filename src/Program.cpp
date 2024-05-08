#include "Program.h"

#include "errors/QinpError.h"
#include "utility/FileReader.h"

Program::Program(bool verbose)
    : m_verbose(verbose)
{
    ;
}

void Program::add_import_directory(const std::string& path_str)
{
    auto path = std::filesystem::path(path_str);
    if (!std::filesystem::is_directory(path))
        throw QinpError("Import directory '" + path_str + "' does not exist");
    m_import_dirs.insert(path);
}

void Program::import_source_file(std::string path_str, bool skip_duplicate)
{
    try {
        path_str = std::filesystem::canonical(path_str).generic_string();
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        throw 1; // throw proper exception when file could not be found
    }

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