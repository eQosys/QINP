#pragma once

#include <string>

#include "TokenizerTypes.h"

std::string readTextFile(const std::string& filename);

TokenListRef tokenize(const std::string& code, const std::string& name);
