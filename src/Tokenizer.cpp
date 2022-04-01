#include "Tokenizer.h"

#include <cstdio>
#include <set>
#include <stdexcept>
#include <set>
#include <fstream>
#include <sstream>
#include <filesystem>

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
		CheckCommentOrNewlineIgnore,
		TokenizeSingleLineComment,
		TokenizeNewlineIgnore,
		TokenizeLiteral,
		TokenizeSpecialKeyword,
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
		
		if (token.type == Token::Type::Keyword)
		{
			if (token.value == "__file__")
			{
				token.type = Token::Type::String;
				token.value = std::filesystem::canonical(token.pos.file).string();
			}
			else if (token.value == "__line__")
			{
				token.type = Token::Type::LiteralInteger;
				token.value = std::to_string(token.pos.line);
			}
		}


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
				state = State::CheckCommentOrNewlineIgnore;
			}
			else if (isSpecialKeywordBegin(std::string(1, c)))
			{
				token.value.push_back(c);
				state = State::TokenizeSpecialKeyword;
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
		case State::CheckCommentOrNewlineIgnore:
			switch (c)
			{
			case '\\':
				token.value.push_back(c);
				state = State::TokenizeSingleLineComment;
				break;
			case ' ':
			case '\t':
			case '\n':
				--index;
				state = State::TokenizeNewlineIgnore;
				break;
			default:
				THROW_TOKENIZER_ERROR(token.pos, "Expected '\\' after '\\'!");
			}
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
		case State::TokenizeNewlineIgnore:
			if (c == '\n')
			{
				token.type = Token::Type::Whitespace;
				token.value = " ";
				state = State::EndToken;
			}
			else if (c != ' ' && c != '\t')
				THROW_TOKENIZER_ERROR(token.pos, "Expected whitespace or newline after '\\'!");
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
		case State::TokenizeSpecialKeyword:
			if (isSpecialKeywordBegin(token.value + std::string(1, c)))
			{
				token.value.push_back(c);
				break;
			}
			--index;
			token.type = specialKeywords.at(token.value);
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
