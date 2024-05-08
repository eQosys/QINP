#include "EnvironmentParser.h"

EnvironmentMap parse_environment(const char** env)
{
    EnvironmentMap environ;

    while (*env)
    {
        std::string entry = *env++;
        size_t sep = entry.find('=');
        std::string name = entry.substr(0, sep);
        std::string value = entry.substr(sep + 1);

        environ.insert({ name, value });
    }

    return environ;
}