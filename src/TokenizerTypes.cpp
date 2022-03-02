#include "TokenizerTypes.h"

#include <iostream>
#include <set>

std::string TokenTypeToString(Token::Type type)
{
	switch (type)
	{
	case Token::Type::None: return "None";
	case Token::Type::Keyword: return "Keyword";
	case Token::Type::BuiltinType: return "BuiltinType";
	case Token::Type::Identifier: return "Identifier";
	case Token::Type::String: return "String";
	case Token::Type::Newline: return "Newline";
	case Token::Type::Whitespace: return "Whitespace";
	case Token::Type::Comment: return "Comment";
	case Token::Type::Operator: return "Operator";
	case Token::Type::Separator: return "Separator";
	case Token::Type::LiteralInteger: return "LiteralInteger";
	case Token::Type::LiteralChar: return "LiteralChar";
	case Token::Type::LiteralBoolean: return "LiteraBoolean";
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

bool isKeyword(const std::string& name)
{
	static const std::set<std::string> keywords =
	{
		"import",
		"asm", "assembly",
		"return", "pass"
		// "const"
		// "default",
		// "sizeof", "offsetof",
		// "if", "elif", "else",
		// "for", "while", "do",
		// "break", "continue",
		// "new", "delete",
		// "goto", "operator",
		// "enum", "union", "pack",
		// "blueprint",
		// "switch", "case"
	};

	return keywords.find(name) != keywords.end();
}

bool isBuiltinType(const std::string& name)
{
	static const std::set<std::string> builtinTypes =
	{
		"void",
		"bool",
		//"i8", "i16", "i32", "i64",
		"u8", "u16", "u32", "u64",
		//"f32", "f64",
	};

	return builtinTypes.find(name) != builtinTypes.end();
}

bool isBooleanValue(const std::string& value)
{
	return value == "true" || value == "false";
}