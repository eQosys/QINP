#pragma once

#include <vector>
#include <string>
#include <memory>

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
typedef std::shared_ptr<TokenList> TokenListRef;

std::string TokenTypeToString(Token::Type type);

std::ostream& operator<<(std::ostream& os, const Token::Position& pos);

std::ostream& operator<<(std::ostream& os, Token::Type type);

std::ostream& operator<<(std::ostream& os, const Token& token);

bool isKeyword(const std::string& name);

bool isBuiltinType(const std::string& name);

bool isBooleanValue(const std::string& value);

bool isKeyword(const Token& token, const std::string& name);

bool isSeparator(const Token& token, const std::string& name);

bool isOperator(const Token& token, const std::string& name);

bool isSepOp(const Token& token);

bool isLiteral(const Token& token);

bool isNewline(const Token& token);

bool isEndOfCode(const Token& token);

bool isBuiltinType(const Token& token);

bool isIdentifier(const Token& token);

bool isString(const Token& token);

bool isWhitespace(const Token& token);

std::string getPosStr(const Token::Position& pos);