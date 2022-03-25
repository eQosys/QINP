#include "OperatorPrecedence.h"

std::vector<OpPrecLvl> opPrecLvls = 
{
	{
		{
			{ "=", Expression::ExprType::Assign },
			{ "+=", Expression::ExprType::Assign_Sum },
			{ "-=", Expression::ExprType::Assign_Difference },
			{ "*=", Expression::ExprType::Assign_Product },
			{ "/=", Expression::ExprType::Assign_Quotient },
			{ "%=", Expression::ExprType::Assign_Remainder },
			{ "<<=", Expression::ExprType::Assign_Bw_LeftShift },
			{ ">>=", Expression::ExprType::Assign_Bw_RightShift },
			{ "&=", Expression::ExprType::Assign_Bw_AND },
			{ "^=", Expression::ExprType::Assign_Bw_XOR },
			{ "|=", Expression::ExprType::Assign_Bw_OR },
		},
		OpPrecLvl::Type::Binary,
		OpPrecLvl::EvalOrder::RightToLeft,
	},
	{
		{
			{ "||", Expression::ExprType::Logical_OR },
		},
		OpPrecLvl::Type::Binary,
		OpPrecLvl::EvalOrder::LeftToRight,
	},
	{
		{
			{ "&&", Expression::ExprType::Logical_AND },
		},
		OpPrecLvl::Type::Binary,
		OpPrecLvl::EvalOrder::LeftToRight,
	},
	{
		{
			{ "|", Expression::ExprType::Bitwise_OR },
		},
		OpPrecLvl::Type::Binary,
		OpPrecLvl::EvalOrder::LeftToRight,
	},
	{
		{
			{ "^", Expression::ExprType::Bitwise_XOR },
		},
		OpPrecLvl::Type::Binary,
		OpPrecLvl::EvalOrder::LeftToRight,
	},
	{
		{
			{ "&", Expression::ExprType::Bitwise_AND },
		},
		OpPrecLvl::Type::Binary,
		OpPrecLvl::EvalOrder::LeftToRight,
	},
	{
		{
			{ "==", Expression::ExprType::Comparison_Equal },
			{ "!=", Expression::ExprType::Comparison_NotEqual },
		},
		OpPrecLvl::Type::Binary,
		OpPrecLvl::EvalOrder::LeftToRight,
	},
	{
		{
			{ "<", Expression::ExprType::Comparison_Less },
			{ "<=", Expression::ExprType::Comparison_LessEqual },
			{ ">", Expression::ExprType::Comparison_Greater },
			{ ">=", Expression::ExprType::Comparison_GreaterEqual },
		},
		OpPrecLvl::Type::Binary,
		OpPrecLvl::EvalOrder::LeftToRight,
	},
	{
		{
			{ "<<", Expression::ExprType::Shift_Left },
			{ ">>", Expression::ExprType::Shift_Right },
		},
		OpPrecLvl::Type::Binary,
		OpPrecLvl::EvalOrder::LeftToRight,
	},
	{
		{
			{ "+", Expression::ExprType::Sum },
			{ "-", Expression::ExprType::Difference },
		},
		OpPrecLvl::Type::Binary,
		OpPrecLvl::EvalOrder::LeftToRight,
	},
	{
		{
			{ "*", Expression::ExprType::Product },
			{ "/", Expression::ExprType::Quotient },
			{ "%", Expression::ExprType::Remainder },
		},
		OpPrecLvl::Type::Binary,
		OpPrecLvl::EvalOrder::LeftToRight,
	},
	{
		{
			{ "&", Expression::ExprType::AddressOf },
			{ "*", Expression::ExprType::Dereference },
			{ "!", Expression::ExprType::Logical_NOT },
			{ "~", Expression::ExprType::Bitwise_NOT },
			{ "+", Expression::ExprType::Prefix_Plus },
			{ "-", Expression::ExprType::Prefix_Minus },
			{ "++", Expression::ExprType::Prefix_Increment },
			{ "--", Expression::ExprType::Prefix_Decrement },
			{ "(", Expression::ExprType::Explicit_Cast },
			{ "sizeof", Expression::ExprType::SizeOf },
			{ "::", Expression::ExprType::Namespace },
		},
		OpPrecLvl::Type::Unary_Prefix,
	},
	{
		{
			{ "[", Expression::ExprType::Subscript },
			{ "(", Expression::ExprType::FunctionCall },
			{ "++", Expression::ExprType::Suffix_Increment },
			{ "--", Expression::ExprType::Suffix_Decrement },

			// Experimental, should have own group
			{ ".", Expression::ExprType::MemberAccess },
			{ "->", Expression::ExprType::MemberAccessDereference },
		},
		OpPrecLvl::Type::Unary_Suffix,
	},
	{
		{
			{ "::", Expression::ExprType::Namespace },
		},
		OpPrecLvl::Type::Binary,
		OpPrecLvl::EvalOrder::LeftToRight,
	},
};