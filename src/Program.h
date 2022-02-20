#pragma once

#include <vector>
#include <memory>
#include <string>

#include "TokenizerTypes.h"

struct Statement
{
	enum class Type
	{
		None,
		Return,
		Expression,
	};

	Statement(const Token::Position& pos, Type type)
		: type(type), pos(pos)
	{}

	Type type;
	Token::Position pos;
};
typedef std::shared_ptr<Statement> StatementRef;

struct Expression : public Statement
{
	enum class ExprType
	{
		None,
		Literal,
		Identifier,
		BinaryOp,
		UnaryOp,
	};

	Expression(const Token::Position& pos, ExprType type)
		: Statement(pos, Statement::Type::Expression), eType(type)
	{}

	ExprType eType;
};
typedef std::shared_ptr<Expression> ExpressionRef;

struct LiteralExpression : public Expression
{
	LiteralExpression(const Token::Position& pos, const std::string& valStr)
		: Expression(pos, ExprType::Literal), value(std::stoi(valStr))
	{}

	int value;
};

struct ReturnStatement : public Statement
{
	ReturnStatement(const Token::Position& pos)
		: Statement(pos, Statement::Type::Return)
	{}
};


typedef std::vector<StatementRef> Body;

struct Program
{
	Body body;
};
typedef std::shared_ptr<Program> ProgramRef;