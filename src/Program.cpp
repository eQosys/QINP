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
    m_import_dirs.push_back(path);
}

void Program::import_source_file(std::string path_str, bool skip_duplicate, bool ignore_import_dirs)
{
    std::filesystem::path path(path_str);
    if (!path.is_absolute())
    {
        bool found_base = false;

        try {
            path_str = std::filesystem::canonical(path).generic_string();
            found_base = true;
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            ;
        }

        if (!ignore_import_dirs)
        {
            for (auto it = m_import_dirs.rbegin(); !found_base && it != m_import_dirs.rend(); ++it)
            {
                auto base = *it;
                try {
                    path_str = std::filesystem::canonical(base / path).generic_string();
                    found_base = true;
                }
                catch (const std::filesystem::filesystem_error& e)
                {
                    continue;
                }
            }
        }

        if (!found_base)
            throw QinpError("Could not locate source file '" + path_str + "'");
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