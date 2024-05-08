#pragma once

#include "QinpError.h"

void print_error(const QinpError& err);
void print_error(const std::runtime_error& err);

void print_warning(const QinpError& warn);
void print_warning(const std::runtime_error& warn);

void print_generic_error(const std::string& what);
void print_generic_warning(const std::string& what);