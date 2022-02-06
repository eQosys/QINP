#pragma once

#include <vector>
#include <string>

struct Token
{
	struct Position
	{
		std::string file;
		int line;
		int column;
	} pos;
	enum class Type
	{
		None,
		Keyword,
		Identifier,
		String,
		Newline,
		Whitespace,
		Comment,
		Operator,
		Separator,
		Literal,
	} type;
	std::string value;
};

typedef std::vector<Token> TokenList;

std::string TokenTypeToString(Token::Type type);

std::ostream& operator<<(std::ostream& os, const Token::Position& pos);

std::ostream& operator<<(std::ostream& os, Token::Type type);

std::ostream& operator<<(std::ostream& os, const Token& token);