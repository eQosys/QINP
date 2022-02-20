#pragma once

#include "QinpError.h"

#include "TokenizerTypes.h"

class NasmGenError : public QinpError
{
public:
	NasmGenError(const Token::Position& pos, const std::string& what)
		: QinpError(pos.file + ":" + std::to_string(pos.line) + ":" + std::to_string(pos.column) + ": " + what)
	{}
};