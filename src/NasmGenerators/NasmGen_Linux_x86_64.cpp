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
	int size = 0;
};

typedef CellInfo::State CellState;

struct NasmGenInfo
{
	std::stringstream ss;
	std::stack<CellInfo> stackCells;
	CellInfo primReg;
	CellInfo secReg;
};

void generateNasm_Linux_x86_64(NasmGenInfo& ngi, const Expression* expr);

std::string regName(char baseChar, int size)
{
	std::stringstream ss;
	switch (size)
	{
	case 1: ss << baseChar << "l"; break;
	case 2: ss << baseChar << "x"; break;
	case 4: ss << "e" << baseChar << "x"; break;
	case 8: ss << "r" << baseChar << "x"; break;
	default: throw NasmGenError(Token::Position(), "Invalid register size!");
	}
	return ss.str();
}

std::string primRegName(int size)
{
	return regName('a', size);
}
std::string secRegName(int size)
{
	return regName('c', size);
}

void pushPrimReg(NasmGenInfo& ngi)
{
	ngi.ss << "  push " << primRegName(8) << "\n";
	ngi.stackCells.push(ngi.primReg);
	ngi.primReg.state = CellState::Unused;
}
void popPrimReg(NasmGenInfo& ngi)
{
	ngi.primReg = ngi.stackCells.top();
	ngi.stackCells.pop();
	ngi.ss << "  pop " << primRegName(8) << "\n";
}
void pushSecReg(NasmGenInfo& ngi)
{
	ngi.ss << "  push " << secRegName(8) << "\n";
	ngi.stackCells.push(ngi.secReg);
	ngi.secReg.state = CellState::Unused;
}
void popSecReg(NasmGenInfo& ngi)
{
	ngi.secReg = ngi.stackCells.top();
	ngi.stackCells.pop();
	ngi.ss << "  pop " << secRegName(8) << "\n";
}
void movePrimToSec(NasmGenInfo& ngi)
{
	ngi.ss << "  mov " << secRegName(ngi.primReg.size) << ", " << primRegName(ngi.primReg.size) << "\n";
	ngi.secReg = ngi.primReg;
}
void moveSecToPrim(NasmGenInfo& ngi)
{
	ngi.ss << "  mov " << primRegName(ngi.secReg.size) << ", " << secRegName(ngi.secReg.size) << "\n";
	ngi.primReg = ngi.secReg;
}

bool primRegLToRVal(NasmGenInfo& ngi, bool pushAddr = false)
{
	if (ngi.primReg.state != CellState::lValue)
		return false;
	if (pushAddr)
		pushPrimReg(ngi);

	ngi.ss << "  mov " << primRegName(ngi.primReg.size) << ", [" << primRegName(8) << "]\n";
	ngi.primReg.state = CellState::rValue;
	return true;
}

void primRegRToLVal(NasmGenInfo& ngi, bool wasPushed)
{
	if (!wasPushed)
		return;
	
	movePrimToSec(ngi);
	popPrimReg(ngi);

	ngi.ss << "  mov [" << primRegName(8) << "], " << secRegName(ngi.primReg.size) << "\n";
	ngi.primReg.state = CellState::lValue;
}

bool secRegLToRVal(NasmGenInfo& ngi, bool pushAddr = false)
{
	if (ngi.secReg.state != CellState::lValue)
		return false;
	if (pushAddr)
		pushSecReg(ngi);

	ngi.ss << "  mov " << secRegName(ngi.secReg.size) << ", [" << secRegName(8) << "]\n";
	ngi.secReg.state = CellState::rValue;
	return true;
}

std::string primRegUsage(NasmGenInfo& ngi)
{
	switch (ngi.primReg.state)
	{
	case CellState::Unused:
	case CellState::rValue:
		return primRegName(ngi.primReg.size);
	case CellState::lValue:
		return "[" + primRegName(8) + "]";
	}
	throw NasmGenError(Token::Position(), "Invalid primReg state!");
}
std::string secRegUsage(NasmGenInfo& ngi)
{
	switch (ngi.secReg.state)
	{
	case CellState::Unused:
	case CellState::rValue:
		return secRegName(ngi.secReg.size);
	case CellState::lValue:
		return "[" + secRegName(ngi.secReg.size) + "]";
	}
	throw NasmGenError(Token::Position(), "Invalid secReg state!");
}

void generateArithmeticBase(NasmGenInfo& ngi, const Expression* expr)
{
	generateNasm_Linux_x86_64(ngi, expr->left.get());
	pushPrimReg(ngi);
	generateNasm_Linux_x86_64(ngi, expr->right.get());
	movePrimToSec(ngi);
	popPrimReg(ngi);
}

void generateNasm_Linux_x86_64(NasmGenInfo& ngi, const Expression* expr)
{
	auto& ss = ngi.ss;
	int dtSize = getDatatypeSize(expr->datatype);
	switch (expr->eType)
	{
	case Expression::ExprType::Conversion:
	{
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		
		int oldSize = getDatatypeSize(expr->left->datatype);
		if (oldSize >= dtSize)
			break;

		primRegLToRVal(ngi);

		switch (oldSize) // Maybe wrong conversion?
		{
		case 1: ss << "  cbw\n"; if (dtSize == 2) break;
		case 2: ss << "  cwd\n"; if (dtSize == 4) break;
		case 4: ss << "  cdq\n"; break;
		}

		ngi.primReg.state = CellState::rValue;
		ngi.primReg.size = dtSize;
	}
		break;
	case Expression::ExprType::Assign:
		generateNasm_Linux_x86_64(ngi, expr->right.get());
		pushPrimReg(ngi);
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		popSecReg(ngi);
		secRegLToRVal(ngi);
		
		if (ngi.primReg.state != CellState::lValue)
			throw NasmGenError(expr->pos, "Cannot assign to non-lvalue!");

		ss << "  mov [" << primRegName(8) << "], " << secRegName(ngi.secReg.size) << "\n";
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
		movePrimToSec(ngi);
		popPrimReg(ngi);
		primRegLToRVal(ngi);
		ss << "  shl " << primRegUsage(ngi) << ", " << secRegName(1) << "\n";
		break;
	case Expression::ExprType::Shift_Right:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		pushPrimReg(ngi);
		generateNasm_Linux_x86_64(ngi, expr->right.get());
		movePrimToSec(ngi);
		popPrimReg(ngi);
		primRegLToRVal(ngi);
		ss << "  shr " << primRegUsage(ngi) << ", " << secRegName(1) << "\n";
		break;
	case Expression::ExprType::Sum:
		generateArithmeticBase(ngi, expr);
		ss << "  add " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		break;
	case Expression::ExprType::Difference:
		generateArithmeticBase(ngi, expr);
		ss << "  sub " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		break;
	case Expression::ExprType::Product:
		generateArithmeticBase(ngi, expr);
		ss << "  mul " << secRegUsage(ngi) << "\n";
		break;
	case Expression::ExprType::Quotient:
		generateArithmeticBase(ngi, expr);
		ss << "  div " << secRegUsage(ngi) << "\n";
		break;
	case Expression::ExprType::Remainder:
		generateArithmeticBase(ngi, expr);
		ss << "  div " << secRegUsage(ngi) << "\n";
		moveSecToPrim(ngi);
		break;
	case Expression::ExprType::AddressOf:
		throw NasmGenError(expr->pos, "AddressOf not supported!");
	case Expression::ExprType::Dereference:
		throw NasmGenError(expr->pos, "Dereference not supported!");
	case Expression::ExprType::Logical_NOT:
		throw NasmGenError(expr->pos, "Logical NOT not supported!");
	case Expression::ExprType::Bitwise_NOT:
		throw NasmGenError(expr->pos, "Bitwise NOT not supported!");
	case Expression::ExprType::Prefix_Plus:
		throw NasmGenError(expr->pos, "Prefix Plus not supported!");
	case Expression::ExprType::Prefix_Minus:
		throw NasmGenError(expr->pos, "Prefix Minus not supported!");
	case Expression::ExprType::Prefix_Increment:
	{
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		if (!expr->left->isLValue)
			throw NasmGenError(expr->pos, "Cannot increment non-lvalue!");
		bool converted = primRegLToRVal(ngi, true);
		ss << "  inc " << primRegUsage(ngi) << "\n";
		primRegRToLVal(ngi, converted);
	}
		break;
	case Expression::ExprType::Prefix_Decrement:
	{
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		if (!expr->left->isLValue)
			throw NasmGenError(expr->pos, "Cannot decrement non-lvalue!");
		bool converted = primRegLToRVal(ngi, true);
		ss << "  dec " << primRegUsage(ngi) << "\n";
		primRegRToLVal(ngi, converted);
	}
		break;
	case Expression::ExprType::Subscript:
		throw NasmGenError(expr->pos, "Subscript not supported!");
	case Expression::ExprType::FunctionCall:
		throw NasmGenError(expr->pos, "FunctionCall not supported!");
	case Expression::ExprType::Suffix_Increment:
		throw NasmGenError(expr->pos, "Suffix Increment not supported!");
	case Expression::ExprType::Suffix_Decrement:
		throw NasmGenError(expr->pos, "Suffix Decrement not supported!");
	case Expression::ExprType::Literal:
		ss << "  mov " << primRegName(dtSize) << ", " << std::to_string(expr->valIntUnsigned) << "\n";
		ngi.primReg.state = CellState::rValue;
		ngi.primReg.size = dtSize;
		break;
	case Expression::ExprType::GlobalVariable:
		ss << "  mov " << primRegName(8) << ", " << expr->globName << "\n";
		ngi.primReg.state = CellState::lValue;
		ngi.primReg.size = dtSize;
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
			ss << "  mov rdi, " << primRegUsage(ngi) << "\n";
			ss << "  mov rax, 60\n";
			ss << "  syscall\n";
			break;
		case Statement::Type::Assembly:
			for (auto& line : statement->asmLines)
				ss << line << "\n";
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