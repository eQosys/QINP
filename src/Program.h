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

		Assign,
		Assign_Sum,
		Assign_Difference,
		Assign_Product,
		Assign_Quotient,
		Assign_Remainder,
		Assign_Bw_LeftShift,
		Assign_Bw_RightShift,
		Assign_Bw_AND,
		Assign_Bw_XOR,
		Assign_Bw_OR,

		Logical_OR,

		Logical_AND,
		
		Bitwise_OR,
		
		Bitwise_XOR,
		
		Bitwise_AND,

		Equality_Equal,
		Equality_NotEqual,

		Relational_Less,
		Relational_LessEqual,
		Relational_Greater,
		Relational_GreaterEqual,

		Shift_Left,
		Shift_Right,

		Sum,
		Difference,

		Product,
		Quotient,
		Remainder,

		Literal,
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