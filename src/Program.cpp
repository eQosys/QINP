#include "Program.h"

bool operator==(const Datatype& left, const Datatype& right)
{
	return left.ptrDepth == right.ptrDepth && left.name == right.name;
}

bool operator!=(const Datatype& left, const Datatype& right)
{
	return !(left == right);
}

bool operator!(const Datatype& datatype)
{
	return datatype.name.empty();
}

int getBuiltinTypeSize(const std::string& name)
{
	static const std::map<std::string, int> sizes =
	{
		{ "void", 0 },
		{ "bool", 1 },
		//{ "i8", 1 },
		//{ "i16", 2 },
		//{ "i32", 4 },
		//{ "i64", 8 },
		{ "u8", 1 },
		{ "u16", 2 },
		{ "u32", 4 },
		{ "u64", 8 },
		//{ "f32", 4 },
		//{ "f64", 8 },
	};

	auto it = sizes.find(name);
	return (it != sizes.end()) ? it->second : -1;
}

int getDatatypeSize(const Datatype& datatype)
{
	if (datatype.ptrDepth > 0)
		return sizeof(void*);

	int size = getBuiltinTypeSize(datatype.name);
	//for (int s : datatype.arraySizes)
	//	size *= s;

	return size;
}

int getDatatypePushSize(const Datatype& datatype)
{
	if (datatype.ptrDepth > 0)
		return sizeof(void*);
	
	if (getBuiltinTypeSize(datatype.name) > 0)
		return 8;

	// Return size for custom datatypes (packs)
	return -1;
}

std::string getMangledDatatype(const Datatype& datatype)
{
	return datatype.name + "~" + std::to_string(datatype.ptrDepth);
}

std::string getMangledSignature(const std::vector<ExpressionRef>& paramExpr)
{
	std::string signature;
	for (auto& expr : paramExpr)
		signature += "." + getMangledDatatype(expr->datatype);
	return signature;
}

std::string getMangledType(const Datatype& retType, const std::vector<Datatype>& paramTypes)
{
	std::string mangledType = getMangledDatatype(retType) + "#";

	for (auto& paramType : paramTypes)
		mangledType += "." + getMangledDatatype(paramType);

	return mangledType;
}

std::string getMangledType(const Datatype& retType, const std::vector<ExpressionRef>& paramExpr)
{
	return getMangledDatatype(retType) + "#" + getMangledSignature(paramExpr);
}

std::string getMangledType(const Datatype& retType, const std::vector<Variable>& params)
{
	std::string mangledType = getMangledDatatype(retType) + "#";

	for (auto& param : params)
		mangledType += "." + getMangledDatatype(param.datatype);

	return mangledType;
}

std::string getMangledType(const FunctionRef func)
{
	return getMangledType(func->retType, func->params);
}

std::string getSignatureFromMangledType(const std::string& mangledType)
{
	auto it = mangledType.find('#');
	return mangledType.substr(it + 1);
}

Datatype getDatatypeFromMangledType(std::string mangledType)
{
	mangledType = mangledType.substr(0, mangledType.find('#'));
	Datatype datatype;
	datatype.name = mangledType.substr(0, mangledType.find('~'));
	datatype.ptrDepth = std::stoi(mangledType.substr(mangledType.find('~') + 1));
	return datatype;
}