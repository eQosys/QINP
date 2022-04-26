#pragma once

#include <string>
#include <vector>

struct Datatype
{
	std::string name;
	int ptrDepth = 0;
	std::vector<int> arraySizes;
};

bool operator==(const Datatype& left, const Datatype& right);
bool operator!=(const Datatype& left, const Datatype& right);
bool operator!(const Datatype& datatype);

bool isPointer(const Datatype& datatype);
bool isArray(const Datatype& datatype);
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