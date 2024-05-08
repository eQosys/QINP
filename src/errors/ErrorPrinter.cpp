#include "ErrorPrinter.h"

#include <cstdio>

void print_error(const QinpError& err)
{ printf("[ ERR ]: QNP: %s\n", err.what()); }
void print_error(const qrawlr::GrammarException& err)
{ printf(" [ ERR ]: Qrawlr: %s\n", err.what()); }
void print_error(const std::runtime_error& err)
{ printf("[ ERR ]: STD: %s\n", err.what()); }

void print_warning(const QinpError& warn)
{ printf("[ WARN ]: QNP: %s\n", warn.what()); }
void print_warning(const qrawlr::GrammarException& err)
{ printf(" [ WARN ]: Qrawlr: %s\n", err.what()); }
void print_warning(const std::runtime_error& warn)
{ printf("[ WARN ]: STD: %s\n", warn.what()); }

void print_generic_error(const std::string& what)
{ printf("[ ERR ]: %s\n", what.c_str()); }
void print_generic_warning(const std::string& what)
{ printf("[ WARN ]: %s\n", what.c_str()); }