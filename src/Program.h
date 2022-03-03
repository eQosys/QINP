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

typedef std::vector<StatementRef> Body;
typedef std::shared_ptr<Body> BodyRef;

struct Variable
{
	Token::Position pos;
	std::string name;
	bool isLocal = false;
	int offset = -1;
	Datatype datatype;
};

struct Function
{
	Token::Position pos;
	std::string name;
	Datatype retType;
	int retOffset = 8;
	int frameSize = 0;
	std::vector<Variable> params;
	BodyRef body;
	bool isDefined;
};

typedef std::shared_ptr<Function> FunctionRef;

std::string getDatatypeStr(const Datatype& datatype);
std::string getSignatureNoRet(const FunctionRef func);
std::string getSignature(const FunctionRef func);
std::string getSignatureNoRet(const Expression* callExpr);
std::string getSignature(const Expression* callExpr);
std::string getMangledName(const FunctionRef func);
std::string getMangledName(const std::string& funcName, const Expression* callExpr);
std::string getMangledName(const std::string& varName, const Datatype& datatype);
std::string getMangledName(const Variable& var);
std::string getMangledName(int strID);

typedef std::map<std::string, FunctionRef> FunctionOverloads;

struct Program
{
	std::map<std::string, Variable> globals;
	std::map<std::string, FunctionOverloads> functions;
	std::map<int, std::string> strings;
	BodyRef body;
};
typedef std::shared_ptr<Program> ProgramRef;

std::string StatementTypeToString(Statement::Type type);

std::string ExpressionTypeToString(Expression::ExprType type);