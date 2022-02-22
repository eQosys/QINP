#include "Program.h"

bool operator==(const Datatype& left, const Datatype& right)
{
	return left.ptrDepth == right.ptrDepth && left.name == right.name;
}

bool operator!=(const Datatype& left, const Datatype& right)
{
	return !(left == right);
}