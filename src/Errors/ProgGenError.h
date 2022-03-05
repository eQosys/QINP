#pragma once

#include "QinpError.h"

#include "Token.h"

class ProgGenError : public QinpError
{
public:
	ProgGenError(const Token::Position& pos, const std::string& what, const std::string& srcFile, int srcLine)
		: QinpError(pos.file + ":" + std::to_string(pos.line) + ":" + std::to_string(pos.column) + ": " + what, srcFile, srcLine)
	{}
};

#define THROW_PROG_GEN_ERROR(pos, what) throw ProgGenError(pos, what, __FILE__, __LINE__)