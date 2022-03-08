#include "Program.h"

#include "Errors/QinpError.h"

std::string getSignatureNoRet(const std::vector<Datatype>& paramTypes)
{
	std::string signature;
	for (const auto& paramType : paramTypes)
		signature += "." + getDatatypeStr(paramType);
	return signature;
}
std::string getSignatureNoRet(const FunctionRef func)
{
	std::string signature;
	for (const auto& param : func->params)
		signature += "." + getDatatypeStr(param.datatype);
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
std::string getSignature(const FunctionRef func)
{
	return getDatatypeStr(func->retType) + "$" + getSignatureNoRet(func);
}
std::string getSignature(const Expression* callExpr)
{
	return getDatatypeStr(callExpr->datatype) + "$" + getSignatureNoRet(callExpr);
}

std::string getMangledName(const std::string funcName, const Datatype& retType, const std::vector<Datatype>& paramTypes)
{
	return funcName + "#" + getSignature(retType, paramTypes);
}
std::string getMangledName(const FunctionRef func)
{
	return func->name + "#" + getSignature(func);
}
std::string getMangledName(const std::string& funcName, const Expression* callExpr)
{
	return funcName + "#" + getSignature(callExpr);
}
std::string getMangledName(const std::string& varName, const Datatype& datatype)
{
	return varName + "#" + getDatatypeStr(datatype);
}
std::string getMangledName(const Variable& var)
{
	return getMangledName(var.modName, var.datatype);
}
std::string getMangledName(int strID)
{
	return "str_##_" + std::to_string(strID);
}

bool isPackType(const ProgramRef program, const std::string& name)
{
	return program->packs.find(name) != program->packs.end();
}

bool isPackType(const ProgramRef program, const Datatype& datatype)
{
	return datatype.ptrDepth == 0 && isPackType(program, datatype.name);
}

int getPackSize(const ProgramRef program, const std::string& packName)
{
	auto pack = program->packs.find(packName);
	if (pack == program->packs.end())
		return -1;

	return pack->second->size;
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
	return std::max(8, getDatatypeSize(program, datatype));
}

int getDatatypePointedToSize(const ProgramRef program, Datatype datatype)
{
	if (!isPointer(datatype))
		THROW_QINP_ERROR("Cannot get size of non-pointer datatype");

	dereferenceDatatype(datatype);
	return getDatatypeSize(program, datatype);
}