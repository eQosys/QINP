#pragma once

#include <string>

#include "Token.h"

std::string readTextFile(const std::string& filename);

TokenListRef tokenize(const std::string& code, std::string name, CommentTokenMapRef comments);
