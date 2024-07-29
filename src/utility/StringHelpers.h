#pragma once

#include <set>
#include <string>

bool startswith(std::string str, std::string start);
bool endswith(std::string str, std::string end);

template <class InputIt>
std::string join(InputIt begin, InputIt end, const std::string& delimiter)
{
    std::string result;

    while (begin != end)
    {
        result += *begin;

        if (++begin != end)
            result += delimiter;
    }

    return result;
}

std::string replace_all(std::string str, const std::string& from, const std::string& to);