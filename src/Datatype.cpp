#include "Datatype.h"

#include <map>

#include "Errors/QinpError.h"

bool operator==(const Datatype& left, const Datatype& right)
{
	return
		left.ptrDepth == right.ptrDepth &&
		left.name == right.name &&
		left.arraySizes == right.arraySizes;
}

bool operator!=(const Datatype& left, const Datatype& right)
{
	return !(left == right);
}

bool operator!(const Datatype& datatype)
{
	return datatype.name.empty();
}

bool isPointer(const Datatype& datatype)
{
	return datatype.ptrDepth > datatype.arraySizes.size();
}

bool isArray(const Datatype& datatype)
{
	return !datatype.arraySizes.empty() && !isPointer(datatype);
}

bool isVoidPtr(const Datatype& datatype)
{
	return datatype == Datatype{ "void", 1 };
}

bool isBool(const Datatype& datatype)
{
	return 
		datatype.ptrDepth == 0 &&
		datatype.name == "bool";
}

bool isInteger(const Datatype& datatype)
{
	return isUnsignedInt(datatype) || isSignedInt(datatype);
}

bool isUnsignedInt(const Datatype& datatype)
{
	return datatype.ptrDepth == 0 &&
		(
			datatype.name == "u8" ||
			datatype.name == "u16" ||
			datatype.name == "u32" ||
			datatype.name == "u64"
		);
}

bool isSignedInt(const Datatype& datatype)
{
	return datatype.ptrDepth == 0 &&
		(
			datatype.name == "i8" ||
			datatype.name == "i16" ||
			datatype.name == "i32" ||
			datatype.name == "i64"
		);
}

void dereferenceDatatype(Datatype& datatype)
{
	if (isPointer(datatype))
	{
		--datatype.ptrDepth;
	}
	else if (isArray(datatype))
	{
		--datatype.ptrDepth;
		datatype.arraySizes.erase(datatype.arraySizes.begin());
	}
	else
	{
		THROW_QINP_ERROR("Cannot dereference non-pointer or non-array datatype");
	}
}

int getDatatypeNumElements(const Datatype& datatype)
{
	if (!isArray(datatype))
		return 1;
	
	int numElements = 1;
	for (int arraySize : datatype.arraySizes)
		numElements *= arraySize;

	return numElements;
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
	return datatype.name + "~" + std::to_string(datatype.ptrDepth);
}