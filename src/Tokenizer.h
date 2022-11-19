#pragma once

#include <string>

#include "Token.h"

std::string readTextFile(const std::string& filename);

std::pair<TokenListRef, TokenListRef> tokenize(const std::string& code, std::string name);
