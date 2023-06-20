#pragma once

#include <vector>
#include <list>
#include <string>
#include <memory>
#include <map>

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
		NewlineIgnore,
		Comment,
		Operator,
		Separator,
		LiteralInteger,
		LiteralChar,
		LiteralBoolean,
		LiteralNull,
		Indentation,
		EndOfCode,
	} type;
	std::string value;
	std::vector<Position> posHistory;
};

bool operator==(const Token::Position& left, const Token::Position& right);

bool operator==(const Token& left, const Token& right);

Token makeToken(Token::Position pos, Token::Type type, const std::string& value);

Token makeToken(Token::Type type, const std::string& value);

Token makeIndentation(uint64_t depth);

void addPosition(Token& token, const Token::Position& pos);

typedef std::list<Token> TokenList;
typedef std::shared_ptr<TokenList> TokenListRef;
typedef std::map<std::string, std::map<int, std::string>> CommentTokenMap;
typedef std::shared_ptr<CommentTokenMap> CommentTokenMapRef;

void addComment(CommentTokenMapRef comments, const Token& token);

extern const std::map<std::string, Token::Type> specialKeywords;

bool isSpecialKeywordBegin(const std::string& str);

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

bool isToken(const Token& token, Token::Type type, const std::string& value);

bool isSepOpKey(const Token& token);

bool isLiteral(const Token& token);

bool isNewline(const Token& token);

bool isEndOfCode(const Token& token);

bool isBuiltinType(const Token& token);

bool isIdentifier(const Token& token);

bool isString(const Token& token);

bool isIndentation(const Token& token);

std::string getPosStr(const Token::Position& pos);