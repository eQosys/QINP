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