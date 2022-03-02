#pragma once

#include <vector>
#include <string>

struct Token
{
	struct Position
	{
		std::string file = "<unknown>";
		int line = 0;
		int column = 0;
	} pos;
	enum class Type
	{
		None,
		Keyword,
		BuiltinType,
		Identifier,
		String,
		Newline,
		Whitespace,
		Comment,
		Operator,
		Separator,
		LiteralInteger,
		LiteralChar,
		LiteralBoolean,
		EndOfCode,
	} type;
	std::string value;
};

typedef std::vector<Token> TokenList;

std::string TokenTypeToString(Token::Type type);

std::ostream& operator<<(std::ostream& os, const Token::Position& pos);

std::ostream& operator<<(std::ostream& os, Token::Type type);

std::ostream& operator<<(std::ostream& os, const Token& token);

bool isKeyword(const std::string& name);

bool isBuiltinType(const std::string& name);

bool isBooleanValue(const std::string& value);