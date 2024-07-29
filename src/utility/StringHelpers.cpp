#include "StringHelpers.h"

bool startswith(std::string str, std::string start)
{
    return str.find(start) == 0;
}
bool endswith(std::string str, std::string end)
{
    return str.rfind(end) == str.size() - end.size();
}

std::string replace_all(std::string str, const std::string& from, const std::string& to)
{
    std::size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }

    return str;
}