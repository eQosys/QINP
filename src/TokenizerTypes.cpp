#include "TokenizerTypes.h"

#include <iostream>

std::string TokenTypeToString(Token::Type type)
{
	switch (type)
	{
	case Token::Type::None: return "None";
	case Token::Type::Keyword: return "Keyword";
	case Token::Type::Identifier: return "Identifier";
	case Token::Type::String: return "String";
	case Token::Type::Newline: return "Newline";
	case Token::Type::Whitespace: return "Whitespace";
	case Token::Type::Comment: return "Comment";
	case Token::Type::Operator: return "Operator";
	case Token::Type::Separator: return "Separator";
	case Token::Type::Literal: return "Literal";
	default: return "Unknown";
	}
}

std::ostream& operator<<(std::ostream& os, const Token::Position& pos)
{
	return os << pos.file + ":" << std::to_string(pos.line) + ":" << std::to_string(pos.column);
}

std::ostream& operator<<(std::ostream& os, Token::Type type)
{
	return os << TokenTypeToString(type);
}

std::ostream& operator<<(std::ostream& os, const Token& token)
{
	return os << token.pos << " [" << token.type << "]: " << token.value;
}