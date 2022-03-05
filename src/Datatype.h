#pragma once

#include <string>

struct Datatype
{
	std::string name;
	int ptrDepth = 0;
	//std::vector<int> arraySizes;
};

bool operator==(const Datatype& left, const Datatype& right);
bool operator!=(const Datatype& left, const Datatype& right);
bool operator!(const Datatype& datatype);

int getBuiltinTypeSize(const std::string& name);

int getDatatypeSize(const Datatype& datatype);
int getDatatypePushSize(const Datatype& datatype);

std::string getDatatypeStr(const Datatype& datatype);