#include "NasmGen_Linux_x86_64.h"

#include <stack>
#include <sstream>

#include "Errors/NasmGenError.h"

struct CellInfo
{
	enum class State
	{
		Unused,
		lValue,
		rValue,
	} state = State::Unused;
};

typedef CellInfo::State CellState;

struct NasmGenInfo
{
	std::stringstream ss;
	std::stack<CellInfo> stackCells;
	CellInfo primReg;
	CellInfo secReg;
};

void pushPrimReg(NasmGenInfo& ngi)
{
	ngi.ss << "  push rax\n";
	ngi.stackCells.push(ngi.primReg);
	ngi.primReg.state = CellState::Unused;
}
void popPrimReg(NasmGenInfo& ngi)
{
	ngi.ss << "  pop rax\n";
	ngi.primReg = ngi.stackCells.top();
	ngi.stackCells.pop();
}
void pushSecReg(NasmGenInfo& ngi)
{
	ngi.ss << "  push rdx\n";
	ngi.stackCells.push(ngi.secReg);
	ngi.secReg.state = CellState::Unused;
}
void popSecReg(NasmGenInfo& ngi)
{
	ngi.ss << "  pop rdx\n";
	ngi.secReg = ngi.stackCells.top();
	ngi.stackCells.pop();
}
void movePrimToSec(NasmGenInfo& ngi)
{
	ngi.ss << "  mov rdx, rax\n";
	ngi.secReg = ngi.primReg;
}
void moveSecToPrim(NasmGenInfo& ngi)
{
	ngi.ss << "  mov rax, rdx\n";
	ngi.primReg = ngi.secReg;
}

std::string primRegName(NasmGenInfo& ngi)
{
	switch (ngi.primReg.state)
	{
	case CellState::Unused:
	case CellState::rValue:
		return "rax";
	case CellState::lValue:
		return "[rax]";
	}
	throw NasmGenError(Token::Position(), "Invalid primReg state!");
}
std::string secRegName(NasmGenInfo& ngi)
{
	switch (ngi.secReg.state)
	{
	case CellState::Unused:
	case CellState::rValue:
		return "rdx";
	case CellState::lValue:
		return "[rdx]";
	}
	throw NasmGenError(Token::Position(), "Invalid secReg state!");
}

void generateNasm_Linux_x86_64(NasmGenInfo& ngi, const Expression* expr)
{
	auto& ss = ngi.ss;
	switch (expr->eType)
	{
	case Expression::ExprType::Conversion:
		throw NasmGenError(expr->pos, "Conversions are not supported!");
	case Expression::ExprType::Assign:
		generateNasm_Linux_x86_64(ngi, expr->right.get());
		pushPrimReg(ngi);
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		popSecReg(ngi);
		ss << "  mov [rax], rdx\n";
		break;
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
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		pushPrimReg(ngi);
		generateNasm_Linux_x86_64(ngi, expr->right.get());
		ss << "  mov cl, al\n";
		popPrimReg(ngi);
		ss << "  shl " << primRegName(ngi) << ", cl\n";
		break;
	case Expression::ExprType::Shift_Right:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		pushPrimReg(ngi);
		generateNasm_Linux_x86_64(ngi, expr->right.get());
		ss << "  mov cl, al\n";
		popPrimReg(ngi);
		ss << "  shr " << primRegName(ngi) << ", cl\n";
		break;
	case Expression::ExprType::Sum:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		pushPrimReg(ngi);
		generateNasm_Linux_x86_64(ngi, expr->right.get());
		movePrimToSec(ngi);
		popPrimReg(ngi);
		ss << "  add " << primRegName(ngi) << ", " << secRegName(ngi) << "\n";
		break;
	case Expression::ExprType::Difference:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		pushPrimReg(ngi);
		generateNasm_Linux_x86_64(ngi, expr->right.get());
		movePrimToSec(ngi);
		popPrimReg(ngi);
		ss << "  sub " << primRegName(ngi) << ", " << secRegName(ngi) << "\n";
		break;
	case Expression::ExprType::Product:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		pushPrimReg(ngi);
		generateNasm_Linux_x86_64(ngi, expr->right.get());
		movePrimToSec(ngi);
		popPrimReg(ngi);
		ss << "  mul " << secRegName(ngi) << "\n";
		break;
	case Expression::ExprType::Quotient:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		pushPrimReg(ngi);
		generateNasm_Linux_x86_64(ngi, expr->right.get());
		movePrimToSec(ngi);
		popPrimReg(ngi);
		ss << "  div " << secRegName(ngi) << "\n";
		break;
	case Expression::ExprType::Remainder:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		pushPrimReg(ngi);
		generateNasm_Linux_x86_64(ngi, expr->right.get());
		movePrimToSec(ngi);
		popPrimReg(ngi);
		ss << "  div " << secRegName(ngi) << "\n";
		moveSecToPrim(ngi);
		break;
	case Expression::ExprType::Literal:
		ss << "  mov rax, " << std::to_string(expr->valIntUnsigned) << "\n";
		ngi.primReg.state = CellState::rValue;
		break;
	case Expression::ExprType::GlobalVariable:
		ss << "  mov rax, " << expr->globName << "\n";
		ngi.primReg.state = CellState::lValue;
		break;
	default:
		throw NasmGenError(expr->pos, "Unsupported expression type!");
	}
}

std::string generateNasm_Linux_x86_64(ProgramRef program)
{
	NasmGenInfo ngi;
	auto& ss = ngi.ss;

	ss << "SECTION .text\n";
	ss << "  global _start\n";
	ss << "_start:\n";
	for (auto& statement : program->body)
	{
		switch (statement->type)
		{
		case Statement::Type::Exit:
			ss << "  mov rdi, " << primRegName(ngi) << "\n";
			ss << "  mov rax, 60\n";
			ss << "  syscall\n";
			break;
		case Statement::Type::Expression:
			generateNasm_Linux_x86_64(ngi, (Expression*)statement.get());
			break;
		default:
			throw NasmGenError(statement->pos, "Unsupported statement type!");
		}
	}
	
	ss << "SECTION .bss\n";
	for (auto& glob : program->globals)
	{
		std::string sizeStr;
		switch (getDatatypeSize(glob.second.datatype))
		{
		case 1: sizeStr = "b"; break;
		case 2: sizeStr = "w"; break;
		case 4: sizeStr = "d"; break;
		case 8: sizeStr = "q"; break;
		}
		ss << "  " << glob.first << " res" << sizeStr << " 1\n";
	}
	
	return ss.str();
}