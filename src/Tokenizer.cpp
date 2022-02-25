#include "Tokenizer.h"

#include <cstdio>
#include <set>
#include <stdexcept>
#include <set>

#include "Errors/QinpError.h"
#include "Errors/TokenizerError.h"

bool isAlpha(char c) { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'); }
bool isNum(char c) { return '0' <= c && c <= '9'; }
bool isAlphaNum(char c) { return isAlpha(c) || isNum(c); }
bool isWhitespace(char c) { return c == ' ' || c == '\t'; }
bool isNewline(char c) { return c == '\n'; }

bool isIDSpecial(char c) { return c == '_'; }
bool isIDBegin(char c) { return isAlpha(c) || isIDSpecial(c); }
bool isIDMid(char c) { return isAlphaNum(c) || isIDSpecial(c); }

static const std::set<std::string> operators =
{
	"+",  "-",  "*",  "/",  "%",  "^",  "&",  "|",  "!",  "=",  "<<",  ">>",  "<",  ">",  "&&",  "||",  "++",  "--",  "~",
	"+=", "-=", "*=", "/=", "%=", "^=", "&=", "|=", "!=", "==", "<<=", ">>=", "<=", ">="
};

static const std::set<std::string> separators =
{
	"(", ")", "[", "]", "{", "}", ":", "::", ",", ".", "..."
};

bool isOperatorBegin(const std::string& str)
{
	return operators.find(str) != operators.end();
}

bool isSeparatorBegin(const std::string& str)
{
	return separators.find(str) != separators.end();
}

char getEscapeChar(char c)
{
	switch (c)
	{
	case '\'': return '\'';
	case '\"': return '\"';
	case '\\': return '\\';
	case 'n': return '\n';
	case 'r': return '\r';
	case 't': return '\t';
	case 'b': return '\b';
	case 'f': return '\f';
	case 'v': return '\v';
	case '0': return '\0';
	default:
		throw std::runtime_error("Unknown escape sequence!");
	}
}

TokenList tokenize(const std::string& code, const std::string& name)
{
	enum class State
	{
		BeginToken,
		EndToken,
		TokenizeIdentifier,
		CheckSingleLineComment,
		TokenizeSingleLineComment,
		TokenizeLiteral,
		TokenizeOperator,
		TokenizeSeparator,
		TokenizeString,
		TokenizeChar,
	} state = State::BeginToken;

	TokenList tokens;
	Token token;

	int index = -1;
	int lastIndex = index;
	int line = 1;
	int column = 0;

	bool specialChar = false;

	auto handleEndToken = [&]()
	{
		state = State::BeginToken;
		if (token.type == Token::Type::Newline)
		{
			for (int i = tokens.size() - 1; i >= 0; --i)
			{
				if (tokens[i].type != Token::Type::Whitespace)
					break;
				tokens.pop_back();
			}
			if (tokens.back().type == Token::Type::Newline)
				return;
		}
		if (token.type == Token::Type::Whitespace &&
			(
				tokens.back().type != Token::Type::Newline &&
				tokens.back().type != Token::Type::Whitespace
			))
			return;
		if (token.type == Token::Type::Comment)
			return;
		
		tokens.push_back(token);
	};

	while (++index < code.size() + 1)
	{
		column += index - lastIndex;
		lastIndex = index;
		char c = (index == code.size()) ? '\n' : code[index];

		switch (state)
		{
		case State::BeginToken:
			token.pos = { name, line, column };
			token.type = Token::Type::None;
			token.value.clear();

			if (isIDBegin(c))
			{
				token.type = Token::Type::Identifier;
				token.value.push_back(c);
				state = State::TokenizeIdentifier;
			}
			else if (isNum(c))
			{
				token.type = Token::Type::Literal;
				token.value.push_back(c);
				state = State::TokenizeLiteral;
			}
			else if (isWhitespace(c))
			{
				token.value.push_back(c);
				token.type = Token::Type::Whitespace;
				state = State::EndToken;
			}
			else if (isNewline(c))
			{
				token.value.push_back(c);
				token.type = Token::Type::Newline;
				++line;
				column = 0;
				state = State::EndToken;
			}
			else if ('\\' == c)
			{
				token.value.push_back(c);
				token.type = Token::Type::Comment;
				state = State::CheckSingleLineComment;
			}
			else if (isOperatorBegin(std::string(1, c)))
			{
				token.value.push_back(c);
				token.type = Token::Type::Operator;
				state = State::TokenizeOperator;
			}
			else if (isSeparatorBegin(std::string(1, c)))
			{
				token.value.push_back(c);
				token.type = Token::Type::Separator;
				state = State::TokenizeSeparator;
			}
			else if ('\'' == c)
			{
				specialChar = false;
				token.type = Token::Type::Literal;
				state = State::TokenizeChar;
			}
			else if ('"' == c)
			{
				specialChar = false;
				token.type = Token::Type::String;
				state = State::TokenizeString;
			}
			else
			{
				throw TokenizerError(name, line, column, std::string("Illegal character '") + c + "' while beginning a token!");
			}
			break;
		case State::TokenizeIdentifier:
			if (isIDMid(c))
			{
				token.value.push_back(c);
				break;
			}
			--index;
			if (isKeyword(token.value))
				token.type = Token::Type::Keyword;
			else if (isBuiltinType(token.value))
				token.type = Token::Type::BuiltinType;
			state = State::EndToken;
			break;
		case State::CheckSingleLineComment:
			if ('\\' != c)
				throw TokenizerError(token.pos, "Expected '\\' after '\\'!");
			token.value.push_back(c);
			state = State::TokenizeSingleLineComment;
			break;
		case State::TokenizeSingleLineComment:
			if (!isNewline(c))
			{
				token.value.push_back(c);
				break;
			}
			--index;
			state = State::EndToken;
			break;
		case State::TokenizeLiteral:
			if (isNum(c) || '.' == c)
			{
				token.value.push_back(c);
				break;
			}
			--index;
			state = State::EndToken;
			break;
		case State::TokenizeOperator:
			if (isOperatorBegin(token.value + std::string(1, c)))
			{
				token.value.push_back(c);
				break;
			}
			--index;
			state = State::EndToken;
			break;
		case State::TokenizeSeparator:
			if (isSeparatorBegin(token.value + std::string(1, c)))
			{
				token.value.push_back(c);
				break;
			}
			--index;
			state = State::EndToken;
			break;
		case State::TokenizeChar:
			if ('\n' == c)
				throw TokenizerError(token.pos, "Unexpected newline!");
			if (!token.value.empty())
			{
				if ('\'' != c)
					throw TokenizerError(token.pos, "A char literal cannot contain more than one character!");
				token.value = std::to_string((int)token.value[0]);
				state = State::EndToken;
				break;
			}

			if (specialChar)
			{
				specialChar = false;
				token.value.push_back(getEscapeChar(c));
				break;
			}
			
			if ('\'' == c)
				throw TokenizerError(token.pos, "A char literal cannot be empty!");
			if ('\\' == c)
				specialChar = true;
			else
			 	token.value.push_back(c);
			break;
		case State::TokenizeString:
			if ('\n' == c)
				throw TokenizerError(token.pos, "Unexpected newline!");
			if (specialChar)
			{
				specialChar = false;
				token.value.push_back(getEscapeChar(c));
				break;
			}

			if ('"' == c)
				state = State::EndToken;
			else if ('\\' == c)
				specialChar = true;
			else
				token.value.push_back(c);

			break;
		case State::EndToken:
			handleEndToken();
			--index;
			break;
		default:
			throw TokenizerError(name, line, column, std::string("Illegal character '") + c + "!");
		}
	}

	if (token.value != "\n")
		throw TokenizerError(name, line, column, std::string("Unexpected End-Of-File while tokenizing '" + token.value + "'!"));

	return tokens;
}
