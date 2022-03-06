#include "Datatype.h"

#include <map>

#include "Errors/QinpError.h"

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

bool isPointer(const Datatype& datatype)
{
	return datatype.ptrDepth > datatype.arraySizes.size();
}

bool isArray(const Datatype& datatype)
{
	return !datatype.arraySizes.empty() && !isPointer(datatype);
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

int getDatatypeSize(const Datatype& datatype, bool treatArrayAsPointer)
{
	if (isPointer(datatype) || (treatArrayAsPointer && isArray(datatype)))
		return sizeof(void*);
	
	if (isArray(datatype))
	{
		int elemSize = getDatatypeSize({ datatype.name, 0 });
		return elemSize * getDatatypeNumElements(datatype);
	}

	int size = getBuiltinTypeSize(datatype.name);

	return size;
}

int getDatatypePushSize(const Datatype& datatype)
{
	return std::max(8, getDatatypeSize(datatype));
}

int getDatatypePointedToSize(Datatype& datatype)
{
	if (!isPointer(datatype))
		THROW_QINP_ERROR("Cannot get size of non-pointer datatype");

	dereferenceDatatype(datatype);
	return getDatatypeSize(datatype);
}

std::string getDatatypeStr(const Datatype& datatype)
{
	return datatype.name + "~" + std::to_string(datatype.ptrDepth);
}