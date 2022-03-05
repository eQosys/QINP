#include "Program.h"

#include "Errors/QinpError.h"

std::string getSignatureNoRet(const FunctionRef func)
{
	std::string signature;
	for (const auto& param : func->params)
		signature += "." + getDatatypeStr(param.datatype);
	return signature;
}

std::string getSignature(const FunctionRef func)
{
	return getDatatypeStr(func->retType) + "$" + getSignatureNoRet(func);
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

std::string getSignature(const Expression* callExpr)
{
	return getDatatypeStr(callExpr->datatype) + "$" + getSignatureNoRet(callExpr);
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
	return getMangledName(var.name, var.datatype);
}

std::string getMangledName(int strID)
{
	return "str_##_" + std::to_string(strID);
}