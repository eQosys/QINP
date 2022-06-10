#pragma once

#include <sstream>

#include "QinpError.h"

#include "Symbols.h"

inline std::string getPosHistoryStr(const std::vector<Token::Position>& posHistory)
{
	std::stringstream ss;
	for (auto pos : posHistory)
		ss << "FROM " << pos.file << ":" << pos.line << ":" << pos.column << "\n";
	return ss.str();
}

class ProgGenError : public QinpError
{
public:
	ProgGenError(const Token::Position& pos, const std::vector<Token::Position>& posHistory, const std::string& what, const std::string& srcFile, int srcLine)
		: QinpError(getPosHistoryStr(posHistory) + getPosStr(pos) + ": " + what, srcFile, srcLine)
	{}
};

#define MAKE_PROG_GEN_ERROR_DETAILED(pos, posHistory, what) ProgGenError(pos, posHistory, what, __FILE__, __LINE__)
#define THROW_PROG_GEN_ERROR_DETAILED(pos, posHistory, what) throw MAKE_PROG_GEN_ERROR_DETAILED(pos, posHistory, what)

#define MAKE_PROG_GEN_ERROR_POS(pos, what) MAKE_PROG_GEN_ERROR_DETAILED(pos, {}, what)
#define THROW_PROG_GEN_ERROR_POS(pos, what) THROW_PROG_GEN_ERROR_DETAILED(pos, {}, what)

#define MAKE_PROG_GEN_ERROR_TOKEN(token, what) MAKE_PROG_GEN_ERROR_DETAILED((token).pos, (token).posHistory, what)
#define THROW_PROG_GEN_ERROR_TOKEN(token, what) THROW_PROG_GEN_ERROR_DETAILED((token).pos, (token).posHistory, what)