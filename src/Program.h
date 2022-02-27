#pragma once

#include <vector>
#include <memory>
#include <string>
#include <map>

#include "TokenizerTypes.h"

struct Datatype
{
	int ptrDepth = 0;
	std::string name;
	std::vector<int> arraySizes;
};

bool operator==(const Datatype& left, const Datatype& right);
bool operator!=(const Datatype& left, const Datatype& right);

int getBuiltinTypeSize(const std::string& name);

int getDatatypeSize(const Datatype& datatype);

struct Statement
{
	enum class Type
	{
		None,
		Exit,
		Assembly,
		Expression,
	};

	Statement(const Token::Position& pos, Type type)
		: type(type), pos(pos)
	{}

	Type type;
	Token::Position pos;

	std::vector<std::string> asmLines; // Single-/multi-line assembly code
};
typedef std::shared_ptr<Statement> StatementRef;

struct Expression;
typedef std::shared_ptr<Expression> ExpressionRef;
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

		Subscript,
		FunctionCall,
		Suffix_Increment,
		Suffix_Decrement,

		Literal,
		GlobalVariable,
	};

	Expression(const Token::Position& pos)
		: Statement(pos, Statement::Type::Expression)
	{}

	ExprType eType = ExprType::None;
	bool isLValue = false;
	Datatype datatype;

	ExpressionRef left; // Binary operator
	ExpressionRef right;
	long long valIntSigned; // Signed integer literal
	unsigned long long valIntUnsigned; // Unsigned integer literal
	double valFloat; // Floating point literal
	int offset; // Local variable
	std::string globName; // Global variable
};

typedef std::vector<StatementRef> Body;

struct Variable
{
	bool isLocal = false;
	int offset = -1;
	Datatype datatype;
};

struct Program
{
	std::map<std::string, Variable> globals;
	Body body;
};
typedef std::shared_ptr<Program> ProgramRef;