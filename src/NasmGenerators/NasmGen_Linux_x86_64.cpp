#include "NasmGen_Linux_x86_64.h"

#include <stack>
#include <sstream>
#include <cassert>

#include "Errors/NasmGenError.h"

struct CellInfo
{
	enum class State
	{
		Unused,
		lValue,
		rValue,
	} state = State::Unused;
	Datatype datatype;
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

std::string basePtrOffset(int offset)
{
	bool isNeg = offset < 0;
	if (isNeg) offset = -offset;
	return std::string("[rbp") + (isNeg ? "-" : "+") + std::to_string(offset) + "]";
}

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
std::string primRegName(NasmGenInfo& ngi)
{
	return primRegName(getDatatypeSize(ngi.primReg.datatype));
}
std::string secRegName(int size)
{
	return regName('c', size);
}
std::string secRegName(NasmGenInfo& ngi)
{
	return secRegName(getDatatypeSize(ngi.secReg.datatype));
}

std::string primRegUsage(NasmGenInfo& ngi)
{
	switch (ngi.primReg.state)
	{
	case CellState::Unused:
	case CellState::rValue:
		return primRegName(ngi);
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
		return secRegName(ngi);
	case CellState::lValue:
		return "[" + secRegName(ngi) + "]";
	}
	throw NasmGenError(Token::Position(), "Invalid secReg state!");
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
void movePrimToSec(NasmGenInfo& ngi, bool markUnused = true)
{
	ngi.ss << "  mov " << secRegName(getDatatypeSize(ngi.primReg.datatype)) << ", " << primRegName(ngi) << "\n";
	ngi.secReg = ngi.primReg;
	if (markUnused)
		ngi.primReg.state = CellState::Unused;
}
void moveSecToPrim(NasmGenInfo& ngi, bool markUnused = true)
{
	ngi.ss << "  mov " << primRegName(getDatatypeSize(ngi.secReg.datatype)) << ", " << secRegName(ngi) << "\n";
	ngi.primReg = ngi.secReg;
	if (markUnused)
		ngi.secReg.state = CellState::Unused;
}

bool primRegLToRVal(NasmGenInfo& ngi, bool pushAddr = false)
{
	if (ngi.primReg.state != CellState::lValue)
		return false;
	if (pushAddr)
		pushPrimReg(ngi);

	ngi.ss << "  mov " << primRegName(ngi) << ", [" << primRegName(8) << "]\n";
	ngi.primReg.state = CellState::rValue;
	return true;
}

void primRegRToLVal(NasmGenInfo& ngi, bool wasPushed)
{
	if (!wasPushed)
		return;
	
	pushSecReg(ngi);

	movePrimToSec(ngi);
	popPrimReg(ngi);

	ngi.ss << "  mov " << primRegUsage(ngi) << ", " << secRegName(getDatatypeSize(ngi.primReg.datatype)) << "\n";

	popSecReg(ngi);
}

bool secRegLToRVal(NasmGenInfo& ngi, bool pushAddr = false)
{
	if (ngi.secReg.state != CellState::lValue)
		return false;
	if (pushAddr)
		pushSecReg(ngi);

	ngi.ss << "  mov " << secRegName(ngi) << ", [" << secRegName(8) << "]\n";
	ngi.secReg.state = CellState::rValue;
	return true;
}

void generateBinaryEvaluation(NasmGenInfo& ngi, const Expression* expr)
{
	generateNasm_Linux_x86_64(ngi, expr->left.get());
	pushPrimReg(ngi);
	generateNasm_Linux_x86_64(ngi, expr->right.get());
	movePrimToSec(ngi);
	popPrimReg(ngi);
}

void generateComparison(NasmGenInfo& ngi, const Expression* expr)
{
	generateBinaryEvaluation(ngi, expr);
	primRegLToRVal(ngi);
	secRegLToRVal(ngi);
	ngi.ss << "  cmp " << primRegUsage(ngi) << ", " << secRegName(getDatatypeSize(ngi.primReg.datatype)) << "\n";
	ngi.primReg.datatype = { 0, "bool" };
}

void generateNasm_Linux_x86_64(NasmGenInfo& ngi, const Expression* expr)
{
	auto& ss = ngi.ss;
	switch (expr->eType)
	{
	case Expression::ExprType::Conversion:
	{
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		
		int newSize = getDatatypeSize(expr->datatype);
		int oldSize = getDatatypeSize(expr->left->datatype);
		if (oldSize >= newSize)
			break;

		primRegLToRVal(ngi);

		switch (oldSize) // Maybe wrong conversion?
		{
		case 1: ss << "  cbw\n"; if (newSize == 2) break;
		case 2: ss << "  cwd\n"; if (newSize == 4) break;
		case 4: ss << "  cdq\n"; break;
		}

		ngi.primReg.datatype = expr->datatype;
	}
		break;
	case Expression::ExprType::Assign:
		generateNasm_Linux_x86_64(ngi, expr->right.get());
		pushPrimReg(ngi);
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		assert(ngi.primReg.state == CellState::lValue && "Cannot assign to non-lvalue!");
		popSecReg(ngi);
		secRegLToRVal(ngi);

		ss << "  mov " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
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
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		secRegLToRVal(ngi);
		ss << "  or " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		ngi.primReg.datatype = { 0, "bool" };
		break;
	case Expression::ExprType::Logical_AND:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		secRegLToRVal(ngi);
		ss << "  and " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		ngi.primReg.datatype = { 0, "bool" };
		break;
	case Expression::ExprType::Bitwise_OR:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		secRegLToRVal(ngi);
		ss << "  or " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		break;
	case Expression::ExprType::Bitwise_XOR:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		secRegLToRVal(ngi);
		ss << "  xor " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		break;
	case Expression::ExprType::Bitwise_AND:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		secRegLToRVal(ngi);
		ss << "  and " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		break;
	case Expression::ExprType::Comparison_Equal:
		generateComparison(ngi, expr);
		ss << "  sete al\n";
		break;
	case Expression::ExprType::Comparison_NotEqual:
		generateComparison(ngi, expr);
		ss << "  setne al\n";
		break;
	case Expression::ExprType::Comparison_Less:
		generateComparison(ngi, expr);
		ss << "  setl al\n";
		break;
	case Expression::ExprType::Comparison_LessEqual:
		generateComparison(ngi, expr);
		ss << "  setle al\n";
		break;
	case Expression::ExprType::Comparison_Greater:
		generateComparison(ngi, expr);
		ss << "  setg al\n";
		break;
	case Expression::ExprType::Comparison_GreaterEqual:
		generateComparison(ngi, expr);
		ss << "  setge al\n";
		break;
	case Expression::ExprType::Shift_Left:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		ss << "  shl " << primRegUsage(ngi) << ", " << secRegName(1) << "\n";
		break;
	case Expression::ExprType::Shift_Right:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		ss << "  shr " << primRegUsage(ngi) << ", " << secRegName(1) << "\n";
		break;
	case Expression::ExprType::Sum:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		ss << "  add " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		break;
	case Expression::ExprType::Difference:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		ss << "  sub " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		break;
	case Expression::ExprType::Product:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		secRegLToRVal(ngi);
		ss << "  mul " << secRegUsage(ngi) << "\n";
		break;
	case Expression::ExprType::Quotient:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		secRegLToRVal(ngi);
		ss << "  div " << secRegUsage(ngi) << "\n";
		break;
	case Expression::ExprType::Remainder:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		secRegLToRVal(ngi);
		ss << "  div " << secRegUsage(ngi) << "\n";
		moveSecToPrim(ngi);
		break;
	case Expression::ExprType::AddressOf:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		assert(ngi.primReg.state == CellState::lValue && "Cannot take address of non-lvalue!");
		++ngi.primReg.datatype.ptrDepth;
		ngi.primReg.state = CellState::rValue;
		break;
	case Expression::ExprType::Dereference:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		assert(ngi.primReg.datatype.ptrDepth != 0 && "Cannot dereference non-pointer!");
		--ngi.primReg.datatype.ptrDepth;
		
		switch (ngi.primReg.state)
		{
		case CellState::lValue:
			ss << "  mov " << primRegName(8) << ", " << primRegUsage(ngi) << "\n";
			break;
		case CellState::rValue:
			ngi.primReg.state = CellState::lValue;
			break;
		default:
			throw NasmGenError(expr->pos, "Invalid CellState!");
		}
		break;
	case Expression::ExprType::Logical_NOT:
		generateNasm_Linux_x86_64(ngi, expr->left.get());\
		primRegLToRVal(ngi);
		ss << "  cmp " << primRegUsage(ngi) << ", 0\n";
		ss << "  sete al\n";
		break;
	case Expression::ExprType::Bitwise_NOT:
	{
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		bool converted = primRegLToRVal(ngi, true);
		ss << "  not " << primRegUsage(ngi) << "\n";
		primRegRToLVal(ngi, converted);
	}
		break;
	case Expression::ExprType::Prefix_Plus:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		break;
	case Expression::ExprType::Prefix_Minus:
	{
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		bool converted = primRegLToRVal(ngi, true);
		ss << "  neg " << primRegUsage(ngi) << "\n";
		primRegRToLVal(ngi, converted);
	}
		break;
	case Expression::ExprType::Prefix_Increment:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		assert(ngi.primReg.state == CellState::lValue && "Cannot increment non-lvalue!");
		primRegLToRVal(ngi, true);
		ss << "  inc " << primRegUsage(ngi) << "\n";
		primRegRToLVal(ngi, true);
		break;
	case Expression::ExprType::Prefix_Decrement:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		assert(ngi.primReg.state == CellState::lValue && "Cannot decrement non-lvalue!");
		primRegLToRVal(ngi, true);
		ss << "  dec " << primRegUsage(ngi) << "\n";
		primRegRToLVal(ngi, true);
		break;
	case Expression::ExprType::Subscript:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		assert(ngi.primReg.datatype.ptrDepth != 0 && "Cannot subscript non-pointer!");
		primRegLToRVal(ngi);
		pushPrimReg(ngi);
		generateNasm_Linux_x86_64(ngi, expr->right.get());
		ss << "  mov " << secRegName(8) << ", " << std::to_string(getDatatypeSize(expr->datatype)) << "\n";
		ss << "  mul " << secRegName(8) << "\n";
		popSecReg(ngi);
		ss << "  add " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		ngi.primReg.state = CellState::lValue;
		break;
	case Expression::ExprType::FunctionCall:
	{
		ss << "  sub rsp, " << std::to_string(getDatatypeSize(expr->datatype)) << "\n";
		for (int i = expr->paramExpr.size() - 1; i >= 0; --i)
		{
			generateNasm_Linux_x86_64(ngi, expr->paramExpr[i].get());
			primRegLToRVal(ngi);
			ss << "  push " << primRegName(8) << "\n";
		}
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		bool typesMatch = ngi.primReg.datatype == Datatype{ 1, getMangledType(expr->datatype, expr->paramExpr) };
		assert(typesMatch && "Cannot call non-function!");
		ss << "  call " << primRegUsage(ngi) << "\n";
		ss << "  add rsp, " << std::to_string(expr->paramSizeSum) << "\n";
		ss << "  pop " << primRegName(8) << "\n";
	}
		break;
	case Expression::ExprType::Suffix_Increment:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		assert(ngi.primReg.state == CellState::lValue && "Cannot increment non-lvalue!");
		movePrimToSec(ngi, false);
		primRegLToRVal(ngi);
		ss << "  inc " << primRegUsage(ngi) << "\n";
		ss << "  mov " << secRegUsage(ngi) << ", " << primRegUsage(ngi) << "\n";
		ss << "  dec " << primRegUsage(ngi) << "\n";
		break;
	case Expression::ExprType::Suffix_Decrement:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		assert(ngi.primReg.state == CellState::lValue && "Cannot decrement non-lvalue!");
		movePrimToSec(ngi, false);
		primRegLToRVal(ngi);
		ss << "  dec " << primRegUsage(ngi) << "\n";
		ss << "  mov " << secRegUsage(ngi) << ", " << primRegUsage(ngi) << "\n";
		ss << "  inc " << primRegUsage(ngi) << "\n";
		break;
	case Expression::ExprType::Literal:
		ngi.primReg.datatype = expr->datatype;
		ngi.primReg.state = CellState::rValue;
		ss << "  mov " << primRegUsage(ngi) << ", " << std::to_string(expr->valIntUnsigned) << "\n";
		break;
	case Expression::ExprType::GlobalVariable:
		ss << "  mov " << primRegName(8) << ", " << expr->globName << "\n";
		ngi.primReg.state = CellState::lValue;
		break;
	case Expression::ExprType::LocalVariable:
		ss << "  mov " << primRegName(8) << ", rbp\n";
		ss << "  add " << primRegName(8) << ", " << std::to_string(expr->localOffset) << "\n";
		ngi.primReg.state = CellState::lValue;
		break;
	case Expression::ExprType::FunctionAddress:
		ss << "  mov " << primRegName(8) << ", " << expr->funcName << "\n";
		ngi.primReg.state = CellState::rValue;
		ngi.primReg.datatype = expr->datatype;
		break;
	default:
		throw NasmGenError(expr->pos, "Unsupported expression type!");
	}
}

void generateNasm_Linux_x86_64(NasmGenInfo& ngi, BodyRef body)
{
	auto& ss = ngi.ss;

	for (auto& statement : *body)
	{
		switch (statement->type)
		{
		case Statement::Type::Return:
			if (statement->subExpr)
			{
				generateNasm_Linux_x86_64(ngi, statement->subExpr.get());
				primRegLToRVal(ngi);
				ss << "  mov " << basePtrOffset(statement->funcRetOffset) << ", " << primRegUsage(ngi) << "\n";
			}
			ss << "  mov rsp, rbp\n";
			ss << "  pop rbp\n";
			ss << "  ret\n";
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
}

void generateNasm_Linux_x86_64(NasmGenInfo& ngi, const std::string& name, FunctionRef func)
{
	auto& ss = ngi.ss;
	ss << name << ":\n";

	ss << "  push rbp\n";
	ss << "  mov rbp, rsp\n";

	generateNasm_Linux_x86_64(ngi, func->body);

	if (func->body->back()->type != Statement::Type::Return)
	{
		ss << "  mov rsp, rbp\n";
		ss << "  pop rbp\n";
		ss << "  ret\n";
	}
}

std::string generateNasm_Linux_x86_64(ProgramRef program)
{
	NasmGenInfo ngi;
	auto& ss = ngi.ss;

	ss << "SECTION .text\n";
	ss << "  global _start\n";
	ss << "_start:\n";
	
	generateNasm_Linux_x86_64(ngi, program->body);

	for (auto& func : program->functions)
		generateNasm_Linux_x86_64(ngi, func.first, func.second);
	
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