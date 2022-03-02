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
};

bool operator==(const Datatype& left, const Datatype& right);
bool operator!=(const Datatype& left, const Datatype& right);
bool operator!(const Datatype& datatype);

int getBuiltinTypeSize(const std::string& name);
int getDatatypeSize(const Datatype& datatype);
int getDatatypePushSize(const Datatype& datatype);

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
		: type(type), pos(pos)
	{}

	Type type;
	Token::Position pos;

	std::vector<std::string> asmLines; // Single-/multi-line assembly code

	ExpressionRef subExpr; // Exit/Return
	int funcRetOffset; // Return
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

		Subscript,
		FunctionCall,
		Suffix_Increment,
		Suffix_Decrement,

		Literal,
		GlobalVariable,
		LocalVariable,
		FunctionAddress,
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
	int localOffset; // Local variable
	std::string globName; // Global variable
	std::string funcName; // Function call
	std::vector<ExpressionRef> paramExpr;
	int paramSizeSum;
};

typedef std::vector<StatementRef> Body;
typedef std::shared_ptr<Body> BodyRef;

struct Variable
{
	std::string name;
	bool isLocal = false;
	int offset = -1;
	Datatype datatype;
};

struct Function
{
	std::string name;
	Datatype retType;
	int retOffset = 8;
	std::vector<Variable> params;
	BodyRef body;
	bool isDefined;
};

typedef std::shared_ptr<Function> FunctionRef;

std::string getMangledSignature(const std::vector<ExpressionRef>& paramExpr);

std::string getMangledType(const Datatype& retType, const std::vector<Datatype>& paramTypes);
std::string getMangledType(const Datatype& retType, const std::vector<ExpressionRef>& paramExpr);
std::string getMangledType(const FunctionRef func);

std::string getSignatureFromMangledType(const std::string& mangledType);

Datatype getDatatypeFromMangledType(std::string mangledType);

struct Program
{
	std::map<std::string, Variable> globals;
	std::map<std::string, FunctionRef> functions;
	BodyRef body;
};
typedef std::shared_ptr<Program> ProgramRef;