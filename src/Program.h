#pragma once

#include <vector>
#include <memory>
#include <string>

struct Statement
{
	enum class Type
	{
		None,
		Return,
		Expression,
	} type;

	Statement(Type type)
		: type(type) {}
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

	Expression(ExprType type)
		: Statement(Statement::Type::Expression), eType(type)
	{}

	ExprType eType;
};
typedef std::shared_ptr<Expression> ExpressionRef;

struct LiteralExpression : public Expression
{
	LiteralExpression(const std::string& valStr)
		: Expression(ExprType::Literal), value(std::stoi(valStr))
	{}

	int value;
};

struct ReturnStatement : public Statement
{
	ReturnStatement()
		: Statement(Statement::Type::Return)
	{}
};


typedef std::vector<StatementRef> Body;

struct Program
{
	Body body;
};
typedef std::shared_ptr<Program> ProgramRef;