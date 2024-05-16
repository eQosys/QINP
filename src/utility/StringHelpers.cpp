#include "StringHelpers.h"

bool startswith(std::string str, std::string start)
{
    return str.find(start) == 0;
}
bool endswith(std::string str, std::string end)
{
    return str.rfind(end) == str.size() - end.size();
}

std::string join(const std::set<std::string>& elements, const std::string& delimiter)
{
    std::string result;

    size_t counter = elements.size();
    for (auto elem : elements)
    {
        result += elem;

        if (--counter != 0)
            result += delimiter;
    }

    return result;
}

std::string replace_all(std::string str, const std::string& from, const std::string& to)
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }

    return str;
}