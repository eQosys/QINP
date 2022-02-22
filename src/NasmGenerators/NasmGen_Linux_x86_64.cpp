#include "NasmGen_Linux_x86_64.h"

#include <sstream>

#include "Errors/NasmGenError.h"

void generateNasm_Linux_x86_64(const Expression* expr, std::stringstream& ss)
{
	switch (expr->eType)
	{
	case Expression::ExprType::Assign:
		throw NasmGenError(expr->pos, "Assignment not supported!");
	case Expression::ExprType::Assign_Sum:
		throw NasmGenError(expr->pos, "Assignment by Sum not supported!");
	case Expression::ExprType::Assign_Difference:
		throw NasmGenError(expr->pos, "Assignment by Difference not supported!");
	case Expression::ExprType::Assign_Product:
		throw NasmGenError(expr->pos, "Assignment by Product not supported!");
	case Expression::ExprType::Assign_Quotient:
		throw NasmGenError(expr->pos, "Assignment by Quotient not supported!");
	case Expression::ExprType::Assign_Remainder:
		throw NasmGenError(expr->pos, "Assignment by Remainder not supported!");
	case Expression::ExprType::Assign_Bw_LeftShift: 
		throw NasmGenError(expr->pos, "Assignment by Left Shift not supported!");
	case Expression::ExprType::Assign_Bw_RightShift:
		throw NasmGenError(expr->pos, "Assignment by Right Shift not supported!");
	case Expression::ExprType::Assign_Bw_AND:
		throw NasmGenError(expr->pos, "Assignment by Bitwise AND not supported!");
	case Expression::ExprType::Assign_Bw_XOR:
		throw NasmGenError(expr->pos, "Assignment by Bitwise XOR not supported!");
	case Expression::ExprType::Assign_Bw_OR:
		throw NasmGenError(expr->pos, "Assignment by Bitwise OR not supported!");
	case Expression::ExprType::Logical_OR:
		throw NasmGenError(expr->pos, "Logical OR not supported!");
	case Expression::ExprType::Logical_AND:
		throw NasmGenError(expr->pos, "Logical AND not supported!");
	case Expression::ExprType::Bitwise_OR:
		throw NasmGenError(expr->pos, "Bitwise OR not supported!");
	case Expression::ExprType::Bitwise_XOR:
		throw NasmGenError(expr->pos, "Bitwise XOR not supported!");
	case Expression::ExprType::Bitwise_AND:
		throw NasmGenError(expr->pos, "Bitwise AND not supported!");
	case Expression::ExprType::Equality_Equal:
		throw NasmGenError(expr->pos, "Equality Equal not supported!");
	case Expression::ExprType::Equality_NotEqual:
		throw NasmGenError(expr->pos, "Equality Not Equal not supported!");
	case Expression::ExprType::Relational_Less:
		throw NasmGenError(expr->pos, "Relational Less not supported!");
	case Expression::ExprType::Relational_LessEqual:
		throw NasmGenError(expr->pos, "Relational Less Equal not supported!");
	case Expression::ExprType::Relational_Greater:
		throw NasmGenError(expr->pos, "Relational Greater not supported!");
	case Expression::ExprType::Relational_GreaterEqual:
		throw NasmGenError(expr->pos, "Relational Greater Equal not supported!");
	case Expression::ExprType::Shift_Left:
		throw NasmGenError(expr->pos, "Shift Left not supported!");
	case Expression::ExprType::Shift_Right:
		throw NasmGenError(expr->pos, "Shift Right not supported!");
	case Expression::ExprType::Sum:
		generateNasm_Linux_x86_64(expr->left.get(), ss);
		ss << "push rax\n";
		generateNasm_Linux_x86_64(expr->right.get(), ss);
		ss << "mov rbx, rax\n";
		ss << "pop rax\n";
		ss << "add rax, rbx\n";
		break;
	case Expression::ExprType::Difference:
		generateNasm_Linux_x86_64(expr->left.get(), ss);
		ss << "push rax\n";
		generateNasm_Linux_x86_64(expr->right.get(), ss);
		ss << "mov rbx, rax\n";
		ss << "pop rax\n";
		ss << "sub rax, rbx\n";
		break;
	case Expression::ExprType::Product:
		generateNasm_Linux_x86_64(expr->left.get(), ss);
		ss << "push rax\n";
		generateNasm_Linux_x86_64(expr->right.get(), ss);
		ss << "mov rbx, rax\n";
		ss << "pop rax\n";
		ss << "mul rbx\n";
		break;
	case Expression::ExprType::Quotient:
		generateNasm_Linux_x86_64(expr->left.get(), ss);
		ss << "push rax\n";
		generateNasm_Linux_x86_64(expr->right.get(), ss);
		ss << "mov rbx, rax\n";
		ss << "pop rax\n";
		ss << "div rbx\n";
		break;
	case Expression::ExprType::Remainder:
		generateNasm_Linux_x86_64(expr->left.get(), ss);
		ss << "push rax\n";
		generateNasm_Linux_x86_64(expr->right.get(), ss);
		ss << "mov rbx, rax\n";
		ss << "pop rax\n";
		ss << "div rbx\n";
		ss << "mov rax, rdx\n";
		break;
	case Expression::ExprType::Literal:
		ss << "mov rax, " << std::to_string(expr->valIntSigned) << "\n";
		break;
	default:
		throw NasmGenError(expr->pos, "Unsupported expression type!");
	}
}

std::string generateNasm_Linux_x86_64(ProgramRef program)
{
	std::stringstream ss;

	ss << "SECTION .text\n";
	ss << "global _start\n";
	ss << "_start:\n";
	for (auto& statement : program->body)
	{
		switch (statement->type)
		{
		case Statement::Type::Exit:
			ss << "mov rdi, rax\n";
			ss << "mov rax, 60\n";
			ss << "syscall\n";
			break;
		case Statement::Type::Expression:
			generateNasm_Linux_x86_64((Expression*)statement.get(), ss);
			break;
		default:
			throw NasmGenError(statement->pos, "Unsupported statement type!");
		}
	}
	
	return ss.str();
}