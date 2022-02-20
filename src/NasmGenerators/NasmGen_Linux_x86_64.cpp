#include "NasmGen_Linux_x86_64.h"

#include <sstream>

#include "Errors/NasmGenError.h"

std::string generateNasm_Linux_x86_64(const ProgramRef program)
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
		{
			auto expr = (Expression*)(statement.get());
			switch (expr->eType)
			{
			case Expression::ExprType::Literal:
				ss << "mov rax, " << std::to_string(((LiteralExpression*)expr)->value) << "\n";
				break;
			default:
				throw NasmGenError(statement->pos, "Unsupported expression type!");
			}
			break;
		}
		default:
			throw NasmGenError(statement->pos, "Unsupported statement type!");
		}
	}

	return ss.str();
}