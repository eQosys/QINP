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

struct Expression;
typedef std::shared_ptr<Expression> ExpressionRef;
struct Expression : public Statement
{
	enum class ExprType
	{
		None,
		Literal,
		Identifier,
		Sum,
		Difference,
		Product,
		Division,
	};

	Expression(const Token::Position& pos)
		: Statement(pos, Statement::Type::Expression)
	{}

	ExprType eType = ExprType::None;
	ExpressionRef left = nullptr;
	ExpressionRef right = nullptr;
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