#include "StringHelpers.h"

bool startswith(std::string str, std::string start)
{
    return str.find(start) == 0;
}
bool endswith(std::string str, std::string end)
{
    return str.rfind(end) == str.size() - end.size();
}