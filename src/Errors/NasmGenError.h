#pragma once

#include "QinpError.h"

#include "Token.h"

class NasmGenError : public QinpError
{
public:
	NasmGenError(const Token::Position& pos, const std::string& what, const std::string& srcFile, int srcLine)
		: QinpError(pos.file + ":" + std::to_string(pos.line) + ":" + std::to_string(pos.column) + ": " + what, srcFile, srcLine)
	{}
};

#define MAKE_NASM_GEN_ERROR(pos, what) NasmGenError(pos, what, __FILE__, __LINE__)
#define THROW_NASM_GEN_ERROR(pos, what) throw MAKE_NASM_GEN_ERROR(pos, what)