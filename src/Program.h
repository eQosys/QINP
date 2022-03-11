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
	std::string modName;
	bool isLocal = false;
	int offset = -1;
	Datatype datatype;
};

struct Function
{
	Token::Position pos;
	std::string name;
	Datatype retType;
	int retOffset = 16;
	int frameSize = 0;
	std::vector<Variable> params;
	BodyRef body;
	bool isDefined;

	bool isReachable = false;
};

typedef std::shared_ptr<Function> FunctionRef;

std::string getSignatureNoRet(const std::vector<Datatype>& paramTypes);
std::string getSignatureNoRet(const FunctionRef func);
std::string getSignatureNoRet(const Expression* callExpr);

std::string getSignature(const Datatype& retType, const std::vector<Datatype>& paramTypes);
std::string getSignature(const FunctionRef func);
std::string getSignature(const Expression* callExpr);

std::string getMangledName(const std::string& funcName, const Datatype& retType, const std::vector<Datatype>& paramTypes);
std::string getMangledName(const FunctionRef func);
std::string getMangledName(const std::string& funcName, const Expression* callExpr);
std::string getMangledName(const std::string& varName, const Datatype& datatype);
std::string getMangledName(const Variable& var);
std::string getLiteralStringName(int strID);
std::string getStaticLocalInitName(int initID);

typedef std::map<std::string, FunctionRef> FunctionOverloads; // signature (without return type) -> function

struct Pack
{
	Token::Position pos;
	std::string name;
	std::map<std::string, Variable> members;
	int size = 0;
	bool isDefined = false;
};

typedef std::shared_ptr<Pack> PackRef;

struct Program
{
	std::map<std::string, Variable> globals;
	std::map<std::string, FunctionOverloads> functions;
	std::map<std::string, PackRef> packs;
	std::map<int, std::string> strings;
	BodyRef body;
	int staticLocalInitCount = 0;
};
typedef std::shared_ptr<Program> ProgramRef;

bool isPackType(const ProgramRef program, const std::string& name);

bool isPackType(const ProgramRef program, const Datatype& datatype);

int getPackSize(const ProgramRef program, const std::string& packName);

int getDatatypeSize(const ProgramRef program, const Datatype& datatype, bool treatArrayAsPointer = false);
int getDatatypePushSize(const ProgramRef program, const Datatype& datatype);

int getDatatypePointedToSize(const ProgramRef program, Datatype datatype);