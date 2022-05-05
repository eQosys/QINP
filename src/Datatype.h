#pragma once

#include <string>
#include <vector>
#include <memory>

struct Datatype;
typedef std::shared_ptr<Datatype> DatatypeRef;

struct Datatype
{
	std::string name = "";
	enum class Type
	{
		None,
		Name, // Has no subtype
		Array, // Has subtype
		Pointer, // Has subtype
		Reference, // Has subtype
	} type = Type::None;
	bool isConst = false;
	DatatypeRef subType = nullptr;
	int arraySize = 0;

	Datatype() = default;
	Datatype(Datatype&& other) = default;
	Datatype(const Datatype& other) = default;
	Datatype(const std::string& name);
	Datatype(Type type, const Datatype& subType, int arraySize = 0);
	Datatype& operator=(Datatype&& other) = default;
	Datatype& operator=(const Datatype& other) = default;
};
typedef Datatype::Type DTType;

bool dtEqual(const Datatype& a, const Datatype& b);
bool dtEqualNoConst(const Datatype& a, const Datatype& b);
bool preservesConstness(const Datatype& oldDt, const Datatype& newDt);

bool operator!(const Datatype& datatype);

bool hasSubtype(const Datatype& datatype);
bool isOfType(const Datatype& datatype, DTType type);
bool isPointer(const Datatype& datatype);
bool isArray(const Datatype& datatype);
bool isDereferenceable(const Datatype& datatype);
bool isVoid(const Datatype& datatype);
bool isVoidPtr(const Datatype& datatype);
bool isBool(const Datatype& datatype);
bool isNull(const Datatype& datatype);
bool isInteger(const Datatype& datatype);
bool isUnsignedInt(const Datatype& datatype);
bool isSignedInt(const Datatype& datatype);

void dereferenceDatatype(Datatype& datatype);

int getDatatypeNumElements(const Datatype& datatype);

int getBuiltinTypeSize(const std::string& name);

std::string getDatatypeStr(const Datatype& datatype);