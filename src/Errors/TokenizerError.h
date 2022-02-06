#pragma once

#include "Errors/QinpError.h"

#include "TokenizerTypes.h"

class TokenizerError : public QinpError
{
public:
    TokenizerError(const std::string& file, int line, int column, const std::string& what)
        : QinpError(file + ":" + std::to_string(line) + ":" + std::to_string(column) + ": " + what)
    {}
    TokenizerError(const Token::Position& pos, const std::string& what)
        : TokenizerError(pos.file, pos.line, pos.column, what)
    {}
};
