#include "FileReader.h"

#include <sstream>
#include <fstream>

std::string read_file(const std::string& path_str)
{
    std::ifstream file(path_str);   
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}