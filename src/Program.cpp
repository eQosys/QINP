#include "Program.h"

#include <cassert>

#include "Errors/QinpError.h"

std::string getSignatureNoRet(const std::vector<Datatype>& paramTypes)
{
	std::string signature;
	for (const auto& paramType : paramTypes)
		signature += "." + getDatatypeStr(paramType);
	return signature;
}
std::string getSignatureNoRet(const SymbolRef func)
{
	std::string signature;
	for (const auto& param : func->func.params)
		signature += "." + getDatatypeStr(param->var.datatype);
	return signature;
}
std::string getSignatureNoRet(const Expression* callExpr)
{
	if (callExpr->eType != Expression::ExprType::FunctionCall)
		THROW_QINP_ERROR("Expected function call expression!");

	std::string signature;
	for (const auto& param : callExpr->paramExpr)
		signature += "." + getDatatypeStr(param->datatype);
	return signature;
}

std::string getSignature(const Datatype& retType, const std::vector<Datatype>& paramTypes)
{
	return getDatatypeStr(retType) + "$" + getSignatureNoRet(paramTypes);
}
std::string getSignature(const SymbolRef func)
{
	return getDatatypeStr(func->func.retType) + "$" + getSignatureNoRet(func);
}
std::string getSignature(const Expression* callExpr)
{
	return getDatatypeStr(callExpr->datatype) + "$" + getSignatureNoRet(callExpr);
}

std::string getMangledName(const std::string funcName, const Datatype& retType, const std::vector<Datatype>& paramTypes)
{
	return funcName + "#" + getSignature(retType, paramTypes);
}
std::string getMangledName(const std::string& funcName, const Expression* callExpr)
{
	return funcName + "#" + getSignature(callExpr);
}
std::string getMangledName(const std::string& varName, const Datatype& datatype)
{
	return varName + "#" + getDatatypeStr(datatype);
}
std::string getMangledName(SymbolRef symbol)
{
	if (isVariable(symbol))
		return getMangledName(symbol->var.modName, symbol->var.datatype);
	if (isFuncSpec(symbol))
		return SymPathToString(getSymbolPath(nullptr, symbol));
	if (isFuncName(symbol))
		return symbol->name;
	assert(false && "Unhandled symbol type!");
}
std::string getLiteralStringName(int strID)
{
	return "str_##_" + std::to_string(strID);
}
std::string getStaticLocalInitName(int initID)
{
	return "stat_loc_init_##_" + std::to_string(initID);
}

bool isPackType(const ProgramRef program, const std::string& name)
{
	return isPack(getSymbol(currSym(program), name));
}

bool isPackType(const ProgramRef program, const Datatype& datatype)
{
	return datatype.ptrDepth == 0 && isPackType(program, datatype.name);
}

int getPackSize(const ProgramRef program, const std::string& packName)
{
	auto sym = getSymbolFromPath(program->symbols, SymPathFromString(packName));
	if (!isPack(sym))
		return -1;

	if (!isDefined(sym))
		return 0;

	return sym->pack.size;
}

int getDatatypeSize(const ProgramRef program, const Datatype& datatype, bool treatArrayAsPointer)
{
	if (isPointer(datatype) || (treatArrayAsPointer && isArray(datatype)))
		return sizeof(void*);
	
	if (isArray(datatype))
	{
		int elemSize = getDatatypeSize(program, { datatype.name, 0 });
		return elemSize * getDatatypeNumElements(datatype);
	}

	int size = getBuiltinTypeSize(datatype.name);
	if (size >= 0)
		return size;

	size = getPackSize(program, datatype.name);
	if (size >= 0)
		return size;

	return -1;
}

int getDatatypePushSize(const ProgramRef program, const Datatype& datatype)
{
	int size = getDatatypeSize(program, datatype);
	if (size < 0)
		return -1;
	// Round the size to the next multiple of 8
	return (size + 7) & -8;
}

int getDatatypePointedToSize(const ProgramRef program, Datatype datatype)
{
	if (!isPointer(datatype) && !isArray(datatype))
		THROW_QINP_ERROR("Cannot get size of non-pointer datatype");

	dereferenceDatatype(datatype);
	return getDatatypeSize(program, datatype);
}

SymbolRef currSym(const ProgramRef program)
{
	return program->symStack.top();
}