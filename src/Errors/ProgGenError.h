#pragma once

#include "QinpError.h"

#include "Symbols.h"

class ProgGenError : public QinpError
{
public:
	ProgGenError(const Token::Position& pos, const std::string& what, const std::string& srcFile, int srcLine)
		: QinpError(getPosStr(pos) + ": " + what, srcFile, srcLine)
	{}
};

#define MAKE_PROG_GEN_ERROR(pos, what) ProgGenError(pos, what, __FILE__, __LINE__)
#define THROW_PROG_GEN_ERROR(pos, what) throw MAKE_PROG_GEN_ERROR(pos, what)