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

#define MAKE_TOKENIZER_ERROR(pos, what) TokenizerError(pos, what, __FILE__, __LINE__)
#define THROW_TOKENIZER_ERROR(pos, what) throw MAKE_TOKENIZER_ERROR(pos, what)
