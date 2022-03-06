#pragma once

#include <vector>
#include <memory>
#include <string>
#include <map>

#include "Token.h"
#include "Statement.h"

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

	bool isReachable = false;
};

typedef std::shared_ptr<Function> FunctionRef;

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