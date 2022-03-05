#pragma once

#include "Token.h"
#include "Datatype.h"

struct Expression;
typedef std::shared_ptr<Expression> ExpressionRef;

struct Statement
{
	enum class Type
	{
		None,
		Return,
		Assembly,
		Expression,
	};

	Statement(const Token::Position& pos, Type type)
		: type(type), pos(pos), sID(sIDCounter++)
	{}

	Type type;
	Token::Position pos;
	int sID;

	std::vector<std::string> asmLines; // Single-/multi-line assembly code

	ExpressionRef subExpr; // Exit/Return
	int funcRetOffset; // Return

private:
	static inline int sIDCounter = 0;
};
typedef std::shared_ptr<Statement> StatementRef;

struct Expression : public Statement
{
	enum class ExprType
	{
		None,

		Conversion,

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

		Comparison_Equal,
		Comparison_NotEqual,

		Comparison_Less,
		Comparison_LessEqual,
		Comparison_Greater,
		Comparison_GreaterEqual,

		Shift_Left,
		Shift_Right,

		Sum,
		Difference,

		Product,
		Quotient,
		Remainder,

		AddressOf,
		Dereference,
		Logical_NOT,
		Bitwise_NOT,
		Prefix_Plus,
		Prefix_Minus,
		Prefix_Increment,
		Prefix_Decrement,
		Explicit_Cast,
		SizeOf,

		Subscript,
		FunctionCall,
		Suffix_Increment,
		Suffix_Decrement,

		Literal,
		GlobalVariable,
		LocalVariable,
		FunctionName,
	};

	Expression(const Token::Position& pos)
		: Statement(pos, Statement::Type::Expression)
	{}

	ExprType eType = ExprType::None;
	bool isLValue = false;
	Datatype datatype;

	ExpressionRef left; // Binary operator
	ExpressionRef right;
	std::string valStr; // Literal
	int localOffset; // Local variable
	std::string globName; // Global variable
	std::string funcName; // Function Address
	std::vector<ExpressionRef> paramExpr; // Function call
	int paramSizeSum;
};

std::string StatementTypeToString(Statement::Type type);

std::string ExpressionTypeToString(Expression::ExprType type);