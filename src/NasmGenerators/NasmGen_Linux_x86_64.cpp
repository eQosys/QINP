#include "NasmGen_Linux_x86_64.h"

#include <sstream>

#include "Errors/NasmGenError.h"

void generateNasm_Linux_x86_64(const Expression* expr, std::stringstream& ss)
{
	switch (expr->eType)
	{
	case Expression::ExprType::Literal:
		ss << "mov rax, " << std::to_string(expr->value) << "\n";
		break;
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
		case Statement::Type::Return:
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