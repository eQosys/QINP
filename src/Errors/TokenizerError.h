#pragma once

#include "QinpError.h"

#include "Token.h"

class TokenizerError : public QinpError
{
public:
    TokenizerError(const Token::Position& pos, const std::string& what, const std::string& srcFile, int srcLine)
        : QinpError(pos.file + ":" + std::to_string(pos.line) + ":" + std::to_string(pos.column) + ": " + what, srcFile, srcLine)
    {}
};

#define THROW_TOKENIZER_ERROR(pos, what) throw TokenizerError(pos, what, __FILE__, __LINE__)
