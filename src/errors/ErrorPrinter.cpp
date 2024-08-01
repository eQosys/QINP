#include "ErrorPrinter.h"

#include <cstdio>

void print_error(const std::runtime_error& err)
{ printf("[ ERR ]: %s\n", err.what()); }

void print_warning(const std::runtime_error& warn)
{ printf("[ WARN ]: %s\n", warn.what()); }

void print_generic_error(const std::string& what)
{ printf("[ ERR ]: %s\n", what.c_str()); }
void print_generic_warning(const std::string& what)
{ printf("[ WARN ]: %s\n", what.c_str()); }