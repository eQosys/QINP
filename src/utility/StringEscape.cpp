#include "StringEscape.h"

std::string escape_string(const std::string& str)
{
    std::string res;

    for (char c : str)
    {
        switch (c)
        {
        case '\a': res += "\\a"; break;
        case '\b': res += "\\b"; break;
        case '\e': res += "\\e"; break;
        case '\f': res += "\\f"; break;
        case '\n': res += "\\n"; break;
        case '\r': res += "\\r"; break;
        case '\t': res += "\\t"; break;
        case '\v': res += "\\v"; break;
        case '\\': res += "\\\\"; break;
        case '\'': res += "\\'"; break;
        case '\"': res += "\\\""; break;
        default: res += c; break;
        }
    }

    return res;
}