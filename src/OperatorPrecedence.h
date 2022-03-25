#pragma once

#include <map>
#include <string>
#include <vector>

#include "Statement.h"

struct OpPrecLvl
{
	std::map<std::string, Expression::ExprType> ops;
	enum class Type
	{
		Unary_Prefix,
		Unary_Suffix,
		Binary,
	} type;
	enum class EvalOrder
	{
		LeftToRight,
		RightToLeft,
	} evalOrder;
};

extern std::vector<OpPrecLvl> opPrecLvls;