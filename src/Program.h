#pragma once

#include <vector>
#include <memory>
#include <string>
#include <map>

#include "Token.h"
#include "Statement.h"
#include "Symbols.h"

std::string getSignatureNoRet(const std::vector<Datatype>& paramTypes);
std::string getSignatureNoRet(const SymbolRef func);
std::string getSignatureNoRet(const Expression* callExpr);

std::string getSignature(const Datatype& retType, const std::vector<Datatype>& paramTypes);
std::string getSignature(const SymbolRef func);
std::string getSignature(const Expression* callExpr);

std::string getMangledName(const std::string& funcName, const Datatype& retType, const std::vector<Datatype>& paramTypes);
std::string getMangledName(const std::string& funcName, const Expression* callExpr);
std::string getMangledName(const std::string& varName, int id, const Datatype& datatype);
std::string getMangledName(SymbolRef symbol);

std::string getReadableName(const std::vector<ExpressionRef>& paramExpr);
std::string getReadableName(const std::vector<SymbolRef>& paramSym, bool isVariadic);
std::string getReadableName(SymbolRef symbol);

std::string getLiteralStringName(int strID);
std::string getStaticLocalInitName(int initID);

typedef std::map<std::string, FunctionRef> FunctionOverloads; // signature (without return type) -> function

struct Program
{
	SymbolRef symbols;
	std::stack<SymbolRef> symStack;
	std::map<std::string, int> strings; // string -> stringID
	BodyRef body;
	int staticLocalInitCount = 0;
	std::vector<int> staticLocalInitIDs;

	std::string platform;
};
typedef std::shared_ptr<Program> ProgramRef;

bool isPackType(const ProgramRef program, const std::string& name);

bool isPackType(const ProgramRef program, const Datatype& datatype);

int getPackSize(const ProgramRef program, const std::string& packName);

int getDatatypeSize(const ProgramRef program, const Datatype& datatype, bool treatArrayAsPointer = false);
int getDatatypePushSize(const ProgramRef program, const Datatype& datatype);

int getDatatypePointedToSize(const ProgramRef program, Datatype datatype);

SymbolRef currSym(const ProgramRef program);