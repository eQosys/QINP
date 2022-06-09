#include "Datatype.h"

#include <map>
#include <cassert>

#include "Errors/QinpError.h"
#include "Program.h"

Datatype::Datatype(const std::string& name)
	: name(name), type(Type::Name)
{}

Datatype::Datatype(Type type, const Datatype& subType, int arraySize)
	: type(type), subType(std::make_shared<Datatype>(subType)), arraySize(arraySize)
{}

bool dtEqual(const Datatype& a, const Datatype& b, bool ignoreFirstConstness)
{
	if (!dtEqualNoConst(a, b))
		return false;

	if (a.isConst != b.isConst && !ignoreFirstConstness)
		return false;
	
	auto sta = a.subType;
	auto stb = b.subType;
	while (sta && stb)
	{
		if (sta->isConst != stb->isConst)
			return false;

		sta = sta->subType;
		stb = stb->subType;
	}

	return true;
}

bool dtEqualNoConst(const Datatype& a, const Datatype& b)
{
	if (a.type != b.type)
		return false;

	switch (a.type)
	{
	case DTType::None:
		return true;
	case DTType::Name:
		return a.name == b.name;
	case DTType::Array:
		if (a.arraySize != b.arraySize)
			return false; // Fallthrough to subtype check
	case DTType::Pointer:
	case DTType::Reference:
		return dtEqualNoConst(*a.subType, *b.subType);
	default:
		assert(false && "Unhandled datatype type");
	}
	return false;
}

bool preservesConstness(const Datatype& oldDt, const Datatype& newDt, bool ignoreFirst)
{
	if (!ignoreFirst && oldDt.isConst && !newDt.isConst)
		return false;
	
	if (hasSubtype(oldDt) && hasSubtype(newDt))
		return preservesConstness(*oldDt.subType, *newDt.subType);
	else if (hasSubtype(oldDt))
		return preservesConstness(*oldDt.subType, newDt);
	else if (hasSubtype(newDt))
		return preservesConstness(oldDt, *newDt.subType);
	
	return true;
}

bool operator!(const Datatype& datatype)
{
	return isOfType(datatype, DTType::None);
}

bool hasSubtype(const Datatype& datatype)
{
	return
		datatype.type != DTType::Name;
}

bool isOfType(const Datatype& datatype, DTType type)
{
	return datatype.type == type;
}

bool isPointer(const Datatype& datatype)
{
	return isOfType(datatype, DTType::Pointer);
}

bool isArray(const Datatype& datatype)
{
	return isOfType(datatype, DTType::Array);
}

bool isDereferenceable(const Datatype& datatype)
{
	return isPointer(datatype) || isArray(datatype);
}

bool isEnum(const ProgramRef program, const Datatype& datatype)
{
	return
		isOfType(datatype, DTType::Name) &&
		isEnum(getSymbolFromPath(program->symbols, SymPathFromString(datatype.name)));
}

bool isVoid(const Datatype& datatype)
{
	return
		isOfType(datatype, DTType::Name) &&
		datatype.name == "void";
}

bool isVoidPtr(const Datatype& datatype)
{
	return
		isOfType(datatype, DTType::Pointer) &&
		isVoid(*datatype.subType);
}

bool isBool(const Datatype& datatype)
{
	return
		isOfType(datatype, DTType::Name) &&
		datatype.name == "bool";
}

bool isNull(const Datatype& datatype)
{
	return
		isOfType(datatype, DTType::Name) &&
		datatype.name == "null";
}

bool isInteger(const Datatype& datatype)
{
	return isUnsignedInt(datatype) || isSignedInt(datatype);
}

bool isUnsignedInt(const Datatype& datatype)
{
	return 
		isOfType(datatype, DTType::Name) &&
		(
			datatype.name == "u8" ||
			datatype.name == "u16" ||
			datatype.name == "u32" ||
			datatype.name == "u64"
		);
}

bool isSignedInt(const Datatype& datatype)
{
	return isOfType(datatype, DTType::Name) &&
		(
			datatype.name == "i8" ||
			datatype.name == "i16" ||
			datatype.name == "i32" ||
			datatype.name == "i64"
		);
}

void dereferenceDatatype(Datatype& datatype)
{
	if (isDereferenceable(datatype))
		datatype = *datatype.subType;
	else
		THROW_QINP_ERROR("Cannot dereference non-pointer or non-array datatype");
}

int getDatatypeNumElements(const Datatype& datatype)
{;
	if (!isArray(datatype))
		return 1;
	return datatype.arraySize * getDatatypeNumElements(*datatype.subType);
}

int getBuiltinTypeSize(const std::string& name)
{
	static const std::map<std::string, int> sizes =
	{
		{ "void", 0 },
		{ "bool", 1 },
		{ "i8", 1 },
		{ "i16", 2 },
		{ "i32", 4 },
		{ "i64", 8 },
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

std::string getDatatypeStr(const Datatype& datatype)
{
	// TODO: Generates names not suitable for NASM
	std::string result;
	if (datatype.isConst)
		result += "c";
	if (isOfType(datatype, DTType::Name))
		result += "?" + datatype.name;
	else if (isOfType(datatype, DTType::Array))
		result += "a" + std::to_string(datatype.arraySize) + getDatatypeStr(*datatype.subType);
	else if (isOfType(datatype, DTType::Pointer))
		result += "p" + getDatatypeStr(*datatype.subType);
	else if (isOfType(datatype, DTType::Reference))
		result += "r" + getDatatypeStr(*datatype.subType);
	return result;
}