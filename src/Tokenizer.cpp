#include "Tokenizer.h"

#include <cstdio>
#include <set>
#include <stdexcept>
#include <set>
#include <fstream>
#include <sstream>

#include "Errors/QinpError.h"
#include "Errors/TokenizerError.h"

std::string readTextFile(const std::string& filename)
{
	std::ifstream file(filename);
	if (!file.is_open())
		THROW_QINP_ERROR("Unable to open file '" + filename + "'!");
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

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
	"+=", "-=", "*=", "/=", "%=", "^=", "&=", "|=", "!=", "==", "<<=", ">>=", "<=", ">=", "->"
};

static const std::set<std::string> separators =
{
	"(", ")",
	"[", "]",
	"{", "}",
	":", "::",
	",", ".",
	"..."
};

bool isOperatorBegin(const std::string& str)
{
	for (auto& op : operators)
	{
		if (op.find(str) == 0)
			return true;
		if (op > str)
			break;
	}
	return false;
}

bool isSeparatorBegin(const std::string& str)
{
	for (auto& sep : separators)
	{
		if (sep.find(str) == 0)
			return true;
		if (sep > str)
			break;
	}
	return false;
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
	case 'e': return '\033';
	default:
		throw std::runtime_error("Unknown escape sequence!");
	}
}

TokenListRef tokenize(const std::string& code, const std::string& name)
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

	TokenListRef tokens = std::make_shared<TokenList>();
	Token token;

	int index = -1;
	int lastIndex = index;
	Token::Position pos;
	pos.file = name;
	pos.line = 1;
	pos.column = 0;

	bool specialChar = false;

	auto handleEndToken = [&]()
	{
		state = State::BeginToken;
		if (token.type == Token::Type::Newline)
		{
			for (int i = tokens->size() - 1; i >= 0; --i)
			{
				if ((*tokens)[i].type != Token::Type::Whitespace)
					break;
				tokens->pop_back();
			}
			if (tokens->empty() || tokens->back().type == Token::Type::Newline)
				return;
		}
		if (token.type == Token::Type::Whitespace &&
			(
				tokens->back().type != Token::Type::Newline &&
				tokens->back().type != Token::Type::Whitespace
			))
			return;
		if (token.type == Token::Type::Comment)
			return;
		
		tokens->push_back(token);
	};

	while (++index < code.size() + 1)
	{
		pos.column += index - lastIndex;
		lastIndex = index;
		char c = (index == code.size()) ? '\n' : code[index];

		switch (state)
		{
		case State::BeginToken:
			token.pos = pos;
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
				token.type = Token::Type::LiteralInteger;
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
				++pos.line;
				pos.column = 0;
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
				token.type = Token::Type::LiteralChar;
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
				THROW_TOKENIZER_ERROR(pos, std::string("Illegal character '") + c + "' while beginning a token!");
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
			else if (isBooleanValue(token.value))
				token.type = Token::Type::LiteralBoolean;
			
			state = State::EndToken;
			break;
		case State::CheckSingleLineComment:
			if ('\\' != c)
				THROW_TOKENIZER_ERROR(token.pos, "Expected '\\' after '\\'!");
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
			if (token.value == "..")
				THROW_TOKENIZER_ERROR(token.pos, "Illegal character sequence '..'!");
			--index;
			state = State::EndToken;
			break;
		case State::TokenizeChar:
			if ('\n' == c)
				THROW_TOKENIZER_ERROR(token.pos, "Unexpected newline!");
			if (!token.value.empty())
			{
				if ('\'' != c)
					THROW_TOKENIZER_ERROR(token.pos, "A char literal cannot contain more than one character!");
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
				THROW_TOKENIZER_ERROR(token.pos, "A char literal cannot be empty!");
			if ('\\' == c)
				specialChar = true;
			else
			 	token.value.push_back(c);
			break;
		case State::TokenizeString:
			if ('\n' == c)
				THROW_TOKENIZER_ERROR(token.pos, "Unexpected newline!");
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
			THROW_TOKENIZER_ERROR(pos, std::string("Illegal character '") + c + "!");
		}
	}

	if (token.value != "\n")
		THROW_TOKENIZER_ERROR(pos, std::string("Unexpected End-Of-File while tokenizing '" + token.value + "'!"));

	return tokens;
}
