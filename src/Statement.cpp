#include "Statement.h"

#include "Errors/QinpError.h"

std::string StatementTypeToString(Statement::Type type)
{
	switch (type)
	{
	case Statement::Type::None: return "None";
	case Statement::Type::Return: return "Return";
	case Statement::Type::Assembly: return "Assembly";
	case Statement::Type::Expression: return "Expression";
	case Statement::Type::If_Clause: return "If_Clause";
	case Statement::Type::While_Loop: return "While_Loop";
	case Statement::Type::Do_While_Loop: return "Do_While_Loop";
	case Statement::Type::Continue: return "Continue";
	case Statement::Type::Break: return "Break";
	default: THROW_QINP_ERROR("Unknown statement type!");
	}
}

std::string ExpressionTypeToString(Expression::ExprType type)
{
	switch (type)
	{
	case Expression::ExprType::None: return "None";
	case Expression::ExprType::Conversion: return "Conversion";
	case Expression::ExprType::Assign: return "Assign";
	case Expression::ExprType::Assign_Sum: return "Assign_Sum";
	case Expression::ExprType::Assign_Difference: return "Assign_Difference";
	case Expression::ExprType::Assign_Product: return "Assign_Product";
	case Expression::ExprType::Assign_Quotient: return "Assign_Quotient";
	case Expression::ExprType::Assign_Remainder: return "Assign_Remainder";
	case Expression::ExprType::Assign_Bw_LeftShift: return "Assign_Bw_LeftShift";
	case Expression::ExprType::Assign_Bw_RightShift: return "Assign_Bw_RightShift";
	case Expression::ExprType::Assign_Bw_AND: return "Assign_Bw_AND";
	case Expression::ExprType::Assign_Bw_XOR: return "Assign_Bw_XOR";
	case Expression::ExprType::Assign_Bw_OR: return "Assign_Bw_OR";
	case Expression::ExprType::Logical_OR: return "Logical_OR";
	case Expression::ExprType::Logical_AND: return "Logical_AND";
	case Expression::ExprType::Bitwise_OR: return "Bitwise_OR";
	case Expression::ExprType::Bitwise_XOR: return "Bitwise_XOR";
	case Expression::ExprType::Bitwise_AND: return "Bitwise_AND";
	case Expression::ExprType::Comparison_Equal: return "Comparison_Equal";
	case Expression::ExprType::Comparison_NotEqual: return "Comparison_NotEqual";
	case Expression::ExprType::Comparison_Less: return "Comparison_Less";
	case Expression::ExprType::Comparison_LessEqual: return "Comparison_LessEqual";
	case Expression::ExprType::Comparison_Greater: return "Comparison_Greater";
	case Expression::ExprType::Comparison_GreaterEqual: return "Comparison_GreaterEqual";
	case Expression::ExprType::Shift_Left: return "Shift_Left";
	case Expression::ExprType::Shift_Right: return "Shift_Right";
	case Expression::ExprType::Sum: return "Sum";
	case Expression::ExprType::Difference: return "Difference";
	case Expression::ExprType::Product: return "Product";
	case Expression::ExprType::Quotient: return "Quotient";
	case Expression::ExprType::Remainder: return "Remainder";
	case Expression::ExprType::MemberAccess: return "MemberAccess";
	case Expression::ExprType::MemberAccessDereference: return "MemberAccessDereference";
	case Expression::ExprType::AddressOf: return "AddressOf";
	case Expression::ExprType::Dereference: return "Dereference";
	case Expression::ExprType::Logical_NOT: return "Logical_NOT";
	case Expression::ExprType::Bitwise_NOT: return "Bitwise_NOT";
	case Expression::ExprType::Prefix_Plus: return "Prefix_Plus";
	case Expression::ExprType::Prefix_Minus: return "Prefix_Minus";
	case Expression::ExprType::Prefix_Increment: return "Prefix_Increment";
	case Expression::ExprType::Prefix_Decrement: return "Prefix_Decrement";
	case Expression::ExprType::Subscript: return "Subscript";
	case Expression::ExprType::FunctionCall: return "FunctionCall";
	case Expression::ExprType::Suffix_Increment: return "Suffix_Increment";
	case Expression::ExprType::Suffix_Decrement: return "Suffix_Decrement";
	case Expression::ExprType::Literal: return "Literal";
	case Expression::ExprType::Symbol: return "Symbol";
	default: THROW_QINP_ERROR("Unknown expression type!");
	}
}