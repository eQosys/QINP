#include "NasmGenerator.h"

#include <stack>
#include <sstream>
#include <cassert>
#include <algorithm>

#include "Errors/NasmGenError.h"

#define LABEL_ID_CONTINUE		0
#define LABEL_ID_BREAK			1

#define LABEL_ID_DO_WHILE_BEGIN	2

#define LABEL_ID_WHILE_BEGIN	2
#define LABEL_ID_WHILE_END		3

#define LABEL_ID_IF_END			0
#define LABEL_ID_IF_NEXT		1

struct CellInfo
{
	enum class State
	{
		Unused,
		lValue,
		rValue,
		xValue,
		__xValueAddress,
	} state = State::Unused;
	Datatype datatype;
};

typedef CellInfo::State CellState;

struct NasmGenInfo
{
	ProgramRef program;
	std::stringstream ss;
	std::stack<CellInfo> stackCells; // Used to keep track of pushed values and to know when to destroy xvalues
	CellInfo primReg;
	CellInfo secReg;
	
	std::vector<std::string> labelStack; // Used for control flow statements

	std::stack<int> loopLabelMarks;
};

std::string makeUniqueLabel(const std::string& name)
{
	static int labelID = 0;
	return name + "__" + std::to_string(labelID++);
}

void pushLabel(NasmGenInfo& ngi, const std::string& name)
{
	ngi.labelStack.push_back(makeUniqueLabel(name));
}

const std::string& getLabel(NasmGenInfo& ngi, int index)
{
	assert(index < ngi.labelStack.size() && "Get label index out of range!");
	return ngi.labelStack[ngi.labelStack.size() - index - 1];
}

void popLabel(NasmGenInfo& ngi)
{
	assert(!ngi.labelStack.empty() && "Trying to pop label from empty label stack!");

	ngi.labelStack.pop_back();
}

void replaceLabel(NasmGenInfo& ngi, const std::string& name, int index)
{
	assert(index < ngi.labelStack.size() && "Replace label index out of range!");

	ngi.labelStack[ngi.labelStack.size() - 1 - index] = makeUniqueLabel(name);
}

void placeLabel(NasmGenInfo& ngi, int index)
{
	assert(index < ngi.labelStack.size() && "Place label index out of range!");

	ngi.ss << getLabel(ngi, index) << ":\n";
}

void pushLoopLabels(NasmGenInfo& ngi)
{
	pushLabel(ngi, "LOOP_BREAK");
	pushLabel(ngi, "LOOP_CONTINUE");
	ngi.loopLabelMarks.push(ngi.labelStack.size());
}

void popLoopLabels(NasmGenInfo& ngi)
{
	ngi.loopLabelMarks.pop();
	popLabel(ngi);
	popLabel(ngi);
}

std::string getLoopLabel(NasmGenInfo& ngi, int index)
{
	return ngi.labelStack[ngi.loopLabelMarks.top() - index - 1];
}

CellInfo::State getRValueIfArray(const Datatype& datatype)
{
	return isArray(datatype) ? CellState::rValue : CellState::lValue;
}

void genExpr(NasmGenInfo& ngi, const Expression* expr);

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
		assert("Invalid register size!" && false);
	}
	return ss.str();
}

std::string primRegName(int size)
{
	return regName('a', size);
}
std::string primRegName(NasmGenInfo& ngi)
{
	return primRegName(getDatatypeSize(ngi.program, ngi.primReg.datatype, true));
}
std::string secRegName(int size)
{
	return regName('c', size);
}
std::string secRegName(NasmGenInfo& ngi)
{
	return secRegName(getDatatypeSize(ngi.program, ngi.secReg.datatype, true));
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
	assert("Invalid primReg state!" && false);
	return "";
}
std::string secRegUsage(NasmGenInfo& ngi)
{
	switch (ngi.secReg.state)
	{
	case CellState::Unused:
	case CellState::rValue:
		return secRegName(ngi);
	case CellState::lValue:
		return "[" + secRegName(8) + "]";
	}
	assert("Invalid secReg state!" && false);
	return "";
}

std::string instrPrefix(const Datatype& datatype)
{
	if (isArray(datatype))
		return "";
	else if (isSignedInt(datatype))
		return "i";
	else if (isUnsignedInt(datatype))
		return "";
	else if (isPointer(datatype))
		return "";
	else
		assert("Invalid datatype!" && false);
	return "";
}

bool isUnused(const CellInfo& cell)
{
	return cell.state == CellState::Unused;
}
bool isRValue(const CellInfo& cell)
{
	return cell.state == CellState::rValue;
}
bool isLValue(const CellInfo& cell)
{
	return cell.state == CellState::lValue;
}
bool isXValue(const CellInfo& cell)
{
	return cell.state == CellState::xValue;
}

CellState getCellState(NasmGenInfo& ngi, const Datatype& datatype)
{
	if (isArray(datatype))
		return CellState::rValue;
	if (isPackType(ngi.program, datatype))
		return CellState::xValue;
	return CellState::lValue;
}

void pushXValue(NasmGenInfo& ngi, const Datatype& datatype)
{
	ngi.stackCells.push(CellInfo{ CellState::xValue, datatype });
	ngi.ss << "  sub rsp, " << getDatatypePushSize(ngi.program, datatype) << "\n";
}
void autoPopXValues(NasmGenInfo& ngi)
{
	while (!ngi.stackCells.empty() && ngi.stackCells.top().state == CellState::xValue)
	{
		ngi.ss << "  add rsp, " << getDatatypePushSize(ngi.program, ngi.stackCells.top().datatype) << "\n";
		ngi.stackCells.pop();
	}
}
void pushPrimReg(NasmGenInfo& ngi)
{
	ngi.ss << "  push " << primRegName(8) << "\n";
	if (isXValue(ngi.primReg))
		ngi.primReg.state = CellState::__xValueAddress;
	ngi.stackCells.push(ngi.primReg);
	ngi.primReg.state = CellState::Unused;
}
void popPrimReg(NasmGenInfo& ngi)
{
	autoPopXValues(ngi);
	ngi.primReg = ngi.stackCells.top();
	ngi.stackCells.pop();
	if (ngi.primReg.state == CellState::__xValueAddress)
		ngi.primReg.state = CellState::xValue;
	ngi.ss << "  pop " << primRegName(8) << "\n";
}
void pushSecReg(NasmGenInfo& ngi)
{
	ngi.ss << "  push " << secRegName(8) << "\n";
	if (isXValue(ngi.secReg))
		ngi.secReg.state = CellState::__xValueAddress;
	ngi.stackCells.push(ngi.secReg);
	ngi.secReg.state = CellState::Unused;
}
void popSecReg(NasmGenInfo& ngi)
{
	autoPopXValues(ngi);
	ngi.secReg = ngi.stackCells.top();
	ngi.stackCells.pop();
	if (ngi.secReg.state == CellState::__xValueAddress)
		ngi.secReg.state = CellState::xValue;
	ngi.ss << "  pop " << secRegName(8) << "\n";
}
void movePrimToSec(NasmGenInfo& ngi, bool markUnused = true)
{
	int size = sizeof(void*);
	if (isRValue(ngi.primReg))
		size = getDatatypeSize(ngi.program, ngi.primReg.datatype);
		
	ngi.ss << "  mov " << secRegName(size) << ", " << primRegName(size) << "\n";
	ngi.secReg = ngi.primReg;
	if (markUnused)
		ngi.primReg.state = CellState::Unused;
}
void moveSecToPrim(NasmGenInfo& ngi, bool markUnused = true)
{
	int size = sizeof(void*);
	if (isRValue(ngi.secReg))
		size = getDatatypeSize(ngi.program, ngi.secReg.datatype);

	ngi.ss << "  mov " << primRegName(size) << ", " << secRegName(size) << "\n";

	ngi.primReg = ngi.secReg;
	if (markUnused)
		ngi.secReg.state = CellState::Unused;
}

bool primRegLToRVal(NasmGenInfo& ngi, bool pushAddr = false)
{
	if (isRValue(ngi.primReg) || isXValue(ngi.primReg))
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

	movePrimToSec(ngi);
	popPrimReg(ngi);

	ngi.ss << "  mov " << primRegUsage(ngi) << ", " << secRegName(getDatatypeSize(ngi.program, ngi.primReg.datatype)) << "\n";
}

bool secRegLToRVal(NasmGenInfo& ngi, bool pushAddr = false)
{
	if (isRValue(ngi.secReg) || isXValue(ngi.primReg))
		return false;
	if (pushAddr)
		pushSecReg(ngi);

	ngi.ss << "  mov " << secRegName(ngi) << ", [" << secRegName(8) << "]\n";
	ngi.secReg.state = CellState::rValue;
	return true;
}

void generateBinaryEvaluation(NasmGenInfo& ngi, const Expression* expr)
{
	genExpr(ngi, expr->left.get());
	pushPrimReg(ngi);
	genExpr(ngi, expr->right.get());
	movePrimToSec(ngi);
	popPrimReg(ngi);
}

void generateComparison(NasmGenInfo& ngi, const Expression* expr)
{
	generateBinaryEvaluation(ngi, expr);
	primRegLToRVal(ngi);
	ngi.ss << "  cmp " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
}

void genMemcpy(NasmGenInfo& ngi, const std::string& destReg, const std::string& srcReg, int size)
{
	auto func = getSymbol(getSymbol(ngi.program->symbols, "memcpy"), getSignatureNoRet({ { "void", 1 }, { "void", 1 }, { "u64" } }), true);
	func->func.isReachable = true;

	ngi.ss << "  push " << size << "\n";
	ngi.ss << "  push " << srcReg << "\n";
	ngi.ss << "  push " << destReg << "\n";
	ngi.ss << "  call " << getMangledName(func) << "\n";
	ngi.ss << "  add rsp, " << 3 * 8 << "\n";
}

#define DISABLE_EXPR_FOR_PACKS(ngi, expr) \
	assert(!isPackType(ngi.program, expr->datatype) && "Invalid expression for pack type!")

void genFuncCall(NasmGenInfo& ngi, const Expression* expr)
{
	bool isVoidFunc = expr->datatype == Datatype{ "void" };
	if (!isVoidFunc)
	{
		if (isPackType(ngi.program, expr->datatype))
			pushXValue(ngi, expr->datatype);
		else
			ngi.ss << "  sub rsp, 8\n"; // All non-pack types (builtins) have a maximum size of 8 bytes
	}

	for (int i = expr->paramExpr.size() - 1; i >= 0; --i)
	{
		genExpr(ngi, expr->paramExpr[i].get());
		if (isPackType(ngi.program, ngi.primReg.datatype))
		{
			if (isXValue(ngi.primReg)) // xvalues are already on the stack
				continue;

			ngi.ss << "  sub rsp, " << getDatatypePushSize(ngi.program, ngi.primReg.datatype) << "\n";
			ngi.ss << "  mov rcx, rsp\n";

			genMemcpy(ngi, "rcx", "rax", getDatatypeSize(ngi.program, ngi.primReg.datatype));
			continue;
		}
		primRegLToRVal(ngi);
		ngi.ss << "  push " << primRegName(8) << "\n";
	}

	genExpr(ngi, expr->left.get());
	bool typesMatch = ngi.primReg.datatype == Datatype{ getSignature(expr), 1 };
	assert(typesMatch && "Cannot call non-function!");
	ngi.ss << "  call " << primRegUsage(ngi) << "\n";
	ngi.ss << "  add rsp, " << std::to_string(expr->paramSizeSum) << "\n";

	ngi.primReg.datatype = expr->datatype;
	ngi.primReg.state = CellState::rValue;

	if (!isVoidFunc)
	{
		if (isPackType(ngi.program, ngi.primReg.datatype))
		{
			ngi.ss << "  mov rax, rsp\n";
			ngi.primReg.state = CellState::xValue;
		}
		else
		{
			ngi.ss << "  pop " << primRegName(8) << "\n";
		}
	}
}

void genExtCall(NasmGenInfo& ngi, const Expression* expr)
{
	// extern functions only support base types atm

	for (int i = expr->paramExpr.size() - 1; i >= 0; --i)
	{
		genExpr(ngi, expr->paramExpr[i].get());
		primRegLToRVal(ngi);
		ngi.ss << "  push " << primRegName(8) << "\n";
		assert((isPointer(ngi.primReg.datatype) || isBuiltinType(ngi.primReg.datatype.name)) && "Arguments of extern functions must be base types!");
	}

	static const std::vector<const char*> paramRegs = { "rcx", "rdx", "r8", "r9" };
	for (int i = 0; i < std::min(expr->paramExpr.size(), paramRegs.size()); ++i)
		ngi.ss << "  pop " << paramRegs[i] << "\n";

	genExpr(ngi, expr->left.get());
	bool typesMatch = ngi.primReg.datatype == Datatype{ getSignature(expr), 1 };
	assert(typesMatch && "Cannot call non-function!");
	ngi.ss << "  call " << primRegUsage(ngi) << "\n";

	// Return value already in rax
	ngi.primReg.datatype = expr->datatype;
	assert((isPointer(ngi.primReg.datatype) || isBuiltinType(ngi.primReg.datatype.name)) && "Return value of extern functions must be of base type!");
	ngi.primReg.state = CellState::rValue;
}

void genExpr(NasmGenInfo& ngi, const Expression* expr)
{
	auto& ss = ngi.ss;

	switch (expr->eType)
	{
	case Expression::ExprType::Conversion:
	{
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->left.get());

		auto& oldType = expr->left->datatype;
		auto& newType = expr->datatype;

		int oldSize = getDatatypeSize(ngi.program, oldType);
		int newSize = getDatatypeSize(ngi.program, newType);

		primRegLToRVal(ngi);

		ngi.primReg.datatype = newType;

		if (isArray(oldType) && isPointer(newType))
			break;

		if (isBool(newType))
		{
			if (isInteger(oldType) || isPointer(oldType))
			{
				ss << "  cmp " << primRegName(oldSize) << ", 0\n";
				ss << "  setne " << primRegName(newSize) << "\n";
				break;
			}
			assert("Invalid conversion!" && false);
		}

		if (
			(isUnsignedInt(oldType) && isInteger(newType)) ||
			(isInteger(oldType) && isPointer(newType)) ||
			(isBool(oldType) && (isInteger || isPointer(newType)))
			)
		{
			if (oldSize == 4 && newSize == 8)
				break;
			if (oldSize < newSize)
				ss << "  movzx " << primRegName(newSize) << ", " << primRegName(oldSize) << "\n";
			break;
		}

		if (isSignedInt(oldType) && isInteger(newType))
		{
			if (oldSize < newSize)
				ss << "  movsx " << primRegName(newSize) << ", " << primRegName(oldSize) << "\n";
			break;
		}

		if (isPointer(oldType) && ((isPointer(newType) || isInteger(newType))))
		{
			break;
		}

		assert("Invalid conversion!" && false);

		// Datatype & state already modified
	}
		break;
	case Expression::ExprType::Assign:
	{
		genExpr(ngi, expr->right.get());
		bool isPack = isPackType(ngi.program, expr->left->datatype);
		pushPrimReg(ngi);
		genExpr(ngi, expr->left.get());
		assert(ngi.primReg.datatype == expr->right->datatype && "Assign: datatype mismatch!");
		assert(isLValue(ngi.primReg) && "Cannot assign to non-lvalue!");
		popSecReg(ngi);

		if (isPackType(ngi.program, ngi.secReg.datatype))
		{
			pushPrimReg(ngi);
			genMemcpy(ngi, "rax", "rcx", getDatatypeSize(ngi.program, ngi.primReg.datatype));
			popPrimReg(ngi);
		}
		else
		{
			secRegLToRVal(ngi);
			ss << "  mov " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		}

		// Datatype & state don't change
	}
		break;
	case Expression::ExprType::Assign_Sum:
	{
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->right.get());
		pushPrimReg(ngi);
		genExpr(ngi, expr->left.get());
		popSecReg(ngi);

		bool pushed = primRegLToRVal(ngi, true);

		ss << "  add " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";

		primRegRToLVal(ngi, pushed);

		break;
	}
	case Expression::ExprType::Assign_Difference:
	{
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->right.get());
		pushPrimReg(ngi);
		genExpr(ngi, expr->left.get());
		popSecReg(ngi);

		bool pushed = primRegLToRVal(ngi, true);

		ss << "  sub " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";

		primRegRToLVal(ngi, pushed);

		break;
	}
	case Expression::ExprType::Assign_Product:
	{
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->right.get());
		pushPrimReg(ngi);
		genExpr(ngi, expr->left.get());
		popSecReg(ngi);
		secRegLToRVal(ngi);

		bool pushed = primRegLToRVal(ngi, true);

		ss << "  " << instrPrefix(ngi.primReg.datatype) << "mul " << secRegUsage(ngi) << "\n";

		primRegRToLVal(ngi, pushed);

		break;
	}
	case Expression::ExprType::Assign_Quotient:
	{
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->right.get());
		pushPrimReg(ngi);
		genExpr(ngi, expr->left.get());
		popSecReg(ngi);
		secRegLToRVal(ngi);

		bool pushed = primRegLToRVal(ngi, true);

		auto remainderName = regName('d', getDatatypeSize(ngi.program, ngi.primReg.datatype));
		ss << "  xor " << remainderName << ", " << remainderName << "\n";

		ss << "  " << instrPrefix(ngi.primReg.datatype) << "div " << secRegUsage(ngi) << "\n";

		primRegRToLVal(ngi, pushed);

		break;
	}
	case Expression::ExprType::Assign_Remainder:
	{
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->right.get());
		pushPrimReg(ngi);
		genExpr(ngi, expr->left.get());
		popSecReg(ngi);
		secRegLToRVal(ngi);

		bool pushed = primRegLToRVal(ngi, true);

		auto remainderName = regName('d', getDatatypeSize(ngi.program, ngi.primReg.datatype));

		ss << "  xor " << remainderName << ", " << remainderName << "\n";

		ss << "  " << instrPrefix(ngi.primReg.datatype) << "div " << secRegUsage(ngi) << "\n";
		ss << "  mov " << primRegUsage(ngi) << ", " << remainderName << "\n";

		primRegRToLVal(ngi, pushed);

		break;
	}
	case Expression::ExprType::Assign_Bw_LeftShift:
	{
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->right.get());
		pushPrimReg(ngi);
		genExpr(ngi, expr->left.get());
		popSecReg(ngi);

		bool pushed = primRegLToRVal(ngi, true);

		ss << "  shl " << primRegUsage(ngi) << ", " << secRegName(1) << "\n";

		primRegRToLVal(ngi, pushed);

		break;
	}
	case Expression::ExprType::Assign_Bw_RightShift:
	{
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->right.get());
		pushPrimReg(ngi);
		genExpr(ngi, expr->left.get());
		popSecReg(ngi);

		bool pushed = primRegLToRVal(ngi, true);

		ss << "  shr " << primRegUsage(ngi) << ", " << secRegName(1) << "\n";

		primRegRToLVal(ngi, pushed);

		break;
	}
	case Expression::ExprType::Assign_Bw_AND:
	{
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->right.get());
		pushPrimReg(ngi);
		genExpr(ngi, expr->left.get());
		popSecReg(ngi);

		bool pushed = primRegLToRVal(ngi, true);

		ss << "  and " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";

		primRegRToLVal(ngi, pushed);

		break;
	}
	case Expression::ExprType::Assign_Bw_XOR:
	{
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->right.get());
		pushPrimReg(ngi);
		genExpr(ngi, expr->left.get());
		popSecReg(ngi);

		bool pushed = primRegLToRVal(ngi, true);

		ss << "  xor " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";

		primRegRToLVal(ngi, pushed);

		break;
	}
	case Expression::ExprType::Assign_Bw_OR:
	{
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->right.get());
		pushPrimReg(ngi);
		genExpr(ngi, expr->left.get());
		popSecReg(ngi);

		bool pushed = primRegLToRVal(ngi, true);

		ss << "  or " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";

		primRegRToLVal(ngi, pushed);

		break;
	}
	case Expression::ExprType::Conditional_Op:
	{
		genExpr(ngi, expr->left.get());
		pushLabel(ngi, "COND_END");
		pushLabel(ngi, "COND_FALSE");

		primRegLToRVal(ngi);

		ss << "  cmp " << primRegUsage(ngi) << ", 0\n";
		ss << "  je " << getLabel(ngi, 0) << "\n";

		genExpr(ngi, expr->right.get());
		ss << "  jmp " << getLabel(ngi, 1) << "\n";
		
		placeLabel(ngi, 0);
		genExpr(ngi, expr->farRight.get());

		placeLabel(ngi, 1);

		popLabel(ngi);
		popLabel(ngi);
	}
		break;
	case Expression::ExprType::Logical_OR:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->left.get());
		primRegLToRVal(ngi);
		pushLabel(ngi, "LOGICAL_OR_SKIP");
		ss << "  cmp " << primRegUsage(ngi) << ", 1\n";
		ss << "  je " << getLabel(ngi, 0) << "\n";
		genExpr(ngi, expr->right.get());
		placeLabel(ngi, 0);
		popLabel(ngi);
		ngi.primReg.datatype = { "bool" };
		// State already modified
		break;
	case Expression::ExprType::Logical_AND:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->left.get());
		primRegLToRVal(ngi);
		pushLabel(ngi, "LOGICAL_AND_SKIP");
		ss << "  cmp " << primRegUsage(ngi) << ", 0\n";
		ss << "  je " << getLabel(ngi, 0) << "\n";
		genExpr(ngi, expr->right.get());
		placeLabel(ngi, 0);
		popLabel(ngi);
		ngi.primReg.datatype = { "bool" };
		// State already modified
		break;
	case Expression::ExprType::Bitwise_OR:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		ss << "  or " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		// Datatype doesn't change
		// State already modified
		break;
	case Expression::ExprType::Bitwise_XOR:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		ss << "  xor " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		// Datatype doesn't change
		// State already modified
		break;
	case Expression::ExprType::Bitwise_AND:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		ss << "  and " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		// Datatype doesn't change
		// State already modified
		break;
	case Expression::ExprType::Comparison_Equal:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		generateComparison(ngi, expr);
		ss << "  sete al\n";
		ngi.primReg.datatype = { "bool" };
		ngi.primReg.state = CellState::rValue;
		break;
	case Expression::ExprType::Comparison_NotEqual:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		generateComparison(ngi, expr);
		ss << "  setne al\n";
		ngi.primReg.datatype = { "bool" };
		ngi.primReg.state = CellState::rValue;
		break;
	case Expression::ExprType::Comparison_Less:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		generateComparison(ngi, expr);
		ss << "  setl al\n";
		ngi.primReg.datatype = { "bool" };
		ngi.primReg.state = CellState::rValue;
		break;
	case Expression::ExprType::Comparison_LessEqual:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		generateComparison(ngi, expr);
		ss << "  setle al\n";
		ngi.primReg.datatype = { "bool" };
		ngi.primReg.state = CellState::rValue;
		break;
	case Expression::ExprType::Comparison_Greater:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		generateComparison(ngi, expr);
		ss << "  setg al\n";
		ngi.primReg.datatype = { "bool" };
		ngi.primReg.state = CellState::rValue;
		break;
	case Expression::ExprType::Comparison_GreaterEqual:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		generateComparison(ngi, expr);
		ss << "  setge al\n";
		ngi.primReg.datatype = { "bool" };
		ngi.primReg.state = CellState::rValue;
		break;
	case Expression::ExprType::Shift_Left:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		ss << "  shl " << primRegUsage(ngi) << ", " << secRegName(1) << "\n";
		// Datatype doesn't change
		// State already modified
		break;
	case Expression::ExprType::Shift_Right:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		ss << "  shr " << primRegUsage(ngi) << ", " << secRegName(1) << "\n";
		// Datatype doesn't change
		// State already modified
		break;
	case Expression::ExprType::Sum:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		ss << "  add " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		// Datatype doesn't change
		// State already modified
		break;
	case Expression::ExprType::Difference:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		ss << "  sub " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		// Datatype doesn't change
		// State already modified
		break;
	case Expression::ExprType::Product:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		secRegLToRVal(ngi);
		ss << "  " << instrPrefix(ngi.primReg.datatype) << "mul " << secRegUsage(ngi) << "\n";
		// Datatype doesn't change
		// State already modified
		break;
	case Expression::ExprType::Quotient:
	{
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		secRegLToRVal(ngi);

		auto remainderName = regName('d', getDatatypeSize(ngi.program, ngi.primReg.datatype));
		ss << "  xor " << remainderName << ", " << remainderName << "\n";

		ss << "  " << instrPrefix(ngi.primReg.datatype) << "div " << secRegUsage(ngi) << "\n";
		// Datatype doesn't change
		// State already modified
	}
		break;
	case Expression::ExprType::Remainder:
	{
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		secRegLToRVal(ngi);

		auto remainderName = regName('d', getDatatypeSize(ngi.program, ngi.primReg.datatype));

		ss << "  xor " << remainderName << ", " << remainderName << "\n";

		ss << "  " << instrPrefix(ngi.primReg.datatype) << "div " << secRegUsage(ngi) << "\n";
		ss << "  mov " << primRegUsage(ngi) << ", " << remainderName << "\n";
		// Datatype doesn't change
		// State already modified
	}
		break;
	case Expression::ExprType::MemberAccess:
	{
		genExpr(ngi, expr->left.get());
		assert((isLValue(ngi.primReg) || isXValue(ngi.primReg)) && "Left operand of member access must be lValue");
		ss << "  add " << primRegName(8) << ", " << expr->symbol->var.offset << "\n";
		
		ngi.primReg.datatype = expr->datatype;
		ngi.primReg.state = getCellState(ngi, ngi.primReg.datatype);
	}
		break;
	case Expression::ExprType::MemberAccessDereference:
		assert("Member access dereference should be converted to member access by the program generator" && false);
		break;
	case Expression::ExprType::AddressOf:
		genExpr(ngi, expr->left.get());
		assert((isLValue(ngi.primReg) || isXValue(ngi.primReg)) && "Cannot take address of non-lvalue!");
		ngi.primReg.datatype = expr->datatype;
		ngi.primReg.state = CellState::rValue;
		break;
	case Expression::ExprType::Dereference:
		genExpr(ngi, expr->left.get());
		assert(ngi.primReg.datatype.ptrDepth != 0 && "Cannot dereference non-pointer!");
		ngi.primReg.datatype = expr->datatype;
		
		switch (ngi.primReg.state)
		{
		case CellState::lValue:
			ss << "  mov " << primRegName(8) << ", " << primRegUsage(ngi) << "\n";
			break;
		case CellState::rValue:
			ngi.primReg.state = getCellState(ngi, ngi.primReg.datatype);
			break;
		default:
			THROW_NASM_GEN_ERROR(expr->pos, "Invalid CellState!");
		}
		// Datatype & state already modified
		break;
	case Expression::ExprType::Logical_NOT:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->left.get());\
		primRegLToRVal(ngi);
		ss << "  cmp " << primRegUsage(ngi) << ", 0\n";
		ss << "  sete al\n";
		// Datatype doesn't change
		// State already modified
		break;
	case Expression::ExprType::Bitwise_NOT:
	{
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->left.get());
		primRegLToRVal(ngi);
		ss << "  not " << primRegUsage(ngi) << "\n";
		// Datatype doesn't change
		// State already modified
	}
		break;
	case Expression::ExprType::Prefix_Plus:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->left.get());
		// Nothing else to do
		break;
	case Expression::ExprType::Prefix_Minus:
	{
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->left.get());
		primRegLToRVal(ngi);
		ss << "  neg " << primRegUsage(ngi) << "\n";
		if (isUnsignedInt(ngi.primReg.datatype))
			ngi.primReg.datatype.name = "i" + ngi.primReg.datatype.name.substr(1);
		// Datatype doesn't change
		// State already modified
	}
		break;
	case Expression::ExprType::Prefix_Increment:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->left.get());
		assert(isLValue(ngi.primReg) && "Cannot increment non-lvalue!");
		primRegLToRVal(ngi, true);
		if (isPointer(ngi.primReg.datatype))
			ss << "  add " << primRegUsage(ngi) << ", " << getDatatypePointedToSize(ngi.program, ngi.primReg.datatype) << "\n";
		else
			ss << "  inc " << primRegUsage(ngi) << "\n";
		primRegRToLVal(ngi, true);
		// Datatype & state don't change
		break;
	case Expression::ExprType::Prefix_Decrement:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->left.get());
		assert(isLValue(ngi.primReg) && "Cannot decrement non-lvalue!");
		primRegLToRVal(ngi, true);
		if (isPointer(ngi.primReg.datatype))
			ss << "  sub " << primRegUsage(ngi) << ", " << getDatatypePointedToSize(ngi.program, ngi.primReg.datatype) << "\n";
		else
			ss << "  dec " << primRegUsage(ngi) << "\n";
		primRegRToLVal(ngi, true);
		// Datatype & state don't change
		break;
	case Expression::ExprType::Subscript:
		genExpr(ngi, expr->left.get());
		assert(ngi.primReg.datatype.ptrDepth != 0 && "Cannot subscript non-pointer!");
		primRegLToRVal(ngi);
		pushPrimReg(ngi);
		genExpr(ngi, expr->right.get());
		primRegLToRVal(ngi);
		ss << "  mov " << secRegName(8) << ", " << getDatatypeSize(ngi.program, expr->datatype) << "\n";
		ss << "  mul " << secRegName(8) << "\n";
		popSecReg(ngi);
		ss << "  add " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		ngi.primReg.datatype = expr->datatype;
		ngi.primReg.state = getCellState(ngi, ngi.primReg.datatype);
		break;
	case Expression::ExprType::FunctionCall:
	{
		if (expr->isExtCall)
			genExtCall(ngi, expr);
		else
			genFuncCall(ngi, expr);
	}
		break;
	case Expression::ExprType::Suffix_Increment:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->left.get());
		assert(isLValue(ngi.primReg) && "Cannot increment non-lvalue!");
		movePrimToSec(ngi, false);
		primRegLToRVal(ngi);
		if (isPointer(ngi.primReg.datatype))
			ss << "  add " << primRegUsage(ngi) << ", " << getDatatypePointedToSize(ngi.program, ngi.primReg.datatype) << "\n";
		else
			ss << "  inc " << primRegUsage(ngi) << "\n";
		
		ss << "  mov " << secRegUsage(ngi) << ", " << primRegUsage(ngi) << "\n";
		
		if (isPointer(ngi.primReg.datatype))
			ss << "  sub " << primRegUsage(ngi) << ", " << getDatatypePointedToSize(ngi.program, ngi.primReg.datatype) << "\n";
		else
			ss << "  dec " << primRegUsage(ngi) << "\n";

		// Datatype doesn't change
		// State already modified
		break;
	case Expression::ExprType::Suffix_Decrement:
		DISABLE_EXPR_FOR_PACKS(ngi, expr->left);
		genExpr(ngi, expr->left.get());
		assert(isLValue(ngi.primReg) && "Cannot decrement non-lvalue!");
		movePrimToSec(ngi, false);
		primRegLToRVal(ngi);
		if (isPointer(ngi.primReg.datatype))
			ss << "  sub " << primRegUsage(ngi) << ", " << getDatatypePointedToSize(ngi.program, ngi.primReg.datatype) << "\n";
		else
			ss << "  dec " << primRegUsage(ngi) << "\n";

		ss << "  mov " << secRegUsage(ngi) << ", " << primRegUsage(ngi) << "\n";

		if (isPointer(ngi.primReg.datatype))
			ss << "  add " << primRegUsage(ngi) << ", " << getDatatypePointedToSize(ngi.program, ngi.primReg.datatype) << "\n";
		else
			ss << "  inc " << primRegUsage(ngi) << "\n";

		// Datatype doesn't change
		// State already modified
		break;
	case Expression::ExprType::Literal:
		ngi.primReg.datatype = expr->datatype;
		ngi.primReg.state = CellState::rValue;
		ss << "  mov " << primRegUsage(ngi) << ", " << expr->valStr << "\n";
		break;
	case Expression::ExprType::Symbol:
	{
		if (isVarLabeled(expr->symbol))
		{
			ss << "  mov " << primRegName(8) << ", " << getMangledName(expr->symbol->var.modName, expr->datatype) << "\n";
			ngi.primReg.datatype = expr->datatype;
			ngi.primReg.state = getRValueIfArray(ngi.primReg.datatype);
		}
		else if (isVarOffset(expr->symbol))
		{
			ss << "  mov " << primRegName(8) << ", rbp\n";
			ss << "  add " << primRegName(8) << ", " << expr->symbol->var.offset << " ; local '" << expr->symbol->var.modName << "'\n";
			ngi.primReg.datatype = expr->datatype;
			ngi.primReg.state = getRValueIfArray(ngi.primReg.datatype);
		}
		else if (isFuncSpec(expr->symbol))
		{
			ss << "  mov " << primRegName(8) << ", " << SymPathToString(getSymbolPath(nullptr, expr->symbol)) << "\n";
			ngi.primReg.datatype = expr->datatype;
			ngi.primReg.state = CellState::rValue;
		}
		else if (isExtFunc(expr->symbol))
		{
			ss << "  mov " << primRegName(8) << ", " << expr->symbol->func.asmName << "\n";
			ngi.primReg.datatype = expr->datatype;
			ngi.primReg.state = CellState::rValue;
		}
		else
		{
			assert(false && "Unknown symbol type!");
		}
	}
		break;
	default:
		THROW_NASM_GEN_ERROR(expr->pos, "Unsupported expression type!");
	}
}

void genStatementAsm(NasmGenInfo& ngi, StatementRef statement);

// Generates Nasm code for any code 'body'
void genBodyAsm(NasmGenInfo& ngi, BodyRef body)
{
	auto& ss = ngi.ss;

	for (auto& statement : body->statements)
		genStatementAsm(ngi, statement);
}

void genStatementAsm(NasmGenInfo& ngi, StatementRef statement)
{
	auto& ss = ngi.ss;

	ss << "; " << getPosStr(statement->pos) << "  ->  ";
	if (statement->type != Statement::Type::Expression)
		ss << StatementTypeToString(statement->type) << "\n";
	else
		ss << ExpressionTypeToString(((Expression*)statement.get())->eType) << "\n";

	switch (statement->type)
	{
	case Statement::Type::Return:
		if (statement->subExpr)
		{
			genExpr(ngi, statement->subExpr.get());
			if (isPackType(ngi.program, ngi.primReg.datatype))
			{
				ss << "  mov rcx, rbp\n";
				ss << "  add rcx, " << statement->funcRetOffset << "\n";
				genMemcpy(ngi, "rcx", "rax", getDatatypeSize(ngi.program, ngi.primReg.datatype));
			}
			else
			{
				primRegLToRVal(ngi);
				ss << "  mov " << basePtrOffset(statement->funcRetOffset) << ", " << primRegUsage(ngi) << "\n";
			}
		}
		ss << "  mov rsp, rbp\n";
		ss << "  pop rbp\n";
		ss << "  ret\n";
		break;
	case Statement::Type::Assembly:
		for (auto& line : statement->asmLines)
			ss << "  " << line << "\n";
		break;
	case Statement::Type::If_Clause:
	{
		pushLabel(ngi, "IF_NEXT");
		pushLabel(ngi, "IF_END");

		for (int i = 0; i < statement->ifConditionalBodies.size(); ++i)
		{
			auto& condBody = statement->ifConditionalBodies[i];

			if (i != 0)
			{
				ss << "  jmp " << getLabel(ngi, LABEL_ID_IF_END) << "\n";

				placeLabel(ngi, LABEL_ID_IF_NEXT);
				replaceLabel(ngi, "IF_NEXT", LABEL_ID_IF_NEXT);
			}

			genExpr(ngi, condBody.condition.get());

			primRegLToRVal(ngi);
			ss << "  cmp " << primRegUsage(ngi) << ", 0\n";
			ss << "  je " << getLabel(ngi, LABEL_ID_IF_NEXT) << "\n";

			genBodyAsm(ngi, condBody.body);
		}

		if (statement->elseBody)
		{
			ss << "  jmp " << getLabel(ngi, LABEL_ID_IF_END) << "\n";
			placeLabel(ngi, LABEL_ID_IF_NEXT);
			replaceLabel(ngi, "IF_NEXT", LABEL_ID_IF_NEXT);

			genBodyAsm(ngi, statement->elseBody);
		}

		placeLabel(ngi, LABEL_ID_IF_NEXT);
		placeLabel(ngi, LABEL_ID_IF_END);
		popLabel(ngi);
		popLabel(ngi);
	}
		break;
	case Statement::Type::While_Loop:
	{
		pushLabel(ngi, "WHILE_END");
		pushLabel(ngi, "WHILE_BEGIN");
		pushLoopLabels(ngi);

		placeLabel(ngi, LABEL_ID_WHILE_BEGIN);
		placeLabel(ngi, LABEL_ID_CONTINUE);

		genExpr(ngi, statement->whileConditionalBody.condition.get());
		primRegLToRVal(ngi);
		ss << "  cmp " << primRegUsage(ngi) << ", 0\n";
		ss << "  je " << getLabel(ngi, LABEL_ID_WHILE_END) << "\n";

		genBodyAsm(ngi, statement->whileConditionalBody.body);

		ss << "  jmp " << getLabel(ngi, LABEL_ID_WHILE_BEGIN) << "\n";

		placeLabel(ngi, LABEL_ID_BREAK);
		placeLabel(ngi, LABEL_ID_WHILE_END);
		popLoopLabels(ngi);
		popLabel(ngi);
		popLabel(ngi);
	}
		break;
	case Statement::Type::Do_While_Loop:
	{
		pushLabel(ngi, "DO_WHILE_BEGIN");
		pushLoopLabels(ngi);
		placeLabel(ngi, LABEL_ID_DO_WHILE_BEGIN);

		genBodyAsm(ngi, statement->doWhileConditionalBody.body);

		placeLabel(ngi, LABEL_ID_CONTINUE);
		
		genExpr(ngi, statement->doWhileConditionalBody.condition.get());

		primRegLToRVal(ngi);
		ss << "  cmp " << primRegUsage(ngi) << ", 0\n";
		ss << "  jne " << getLabel(ngi, LABEL_ID_DO_WHILE_BEGIN) << "\n";

		placeLabel(ngi, LABEL_ID_BREAK);
		popLoopLabels(ngi);
		popLabel(ngi);
	}
		break;
	case Statement::Type::Continue:
		ss << "  jmp " << getLoopLabel(ngi, LABEL_ID_CONTINUE) << "\n";
		break;
	case Statement::Type::Break:
		ss << "  jmp " << getLoopLabel(ngi, LABEL_ID_BREAK) << "\n";
		break;
	case Statement::Type::Expression:
		genExpr(ngi, (Expression*)statement.get());
		autoPopXValues(ngi);
		break;
	default:
		THROW_NASM_GEN_ERROR(statement->pos, "Unsupported statement type!");
	}
}

// Generates Nasm code for a function
void genFuncAsm(NasmGenInfo& ngi, const std::string& name, SymbolRef func)
{
	assert(func->func.body->statements.back()->type == Statement::Type::Return && "Function must end with return statement!");

	// Function prologue
	ngi.ss << getMangledName(func) << ":\n";
	ngi.ss << "  push rbp\n";
	ngi.ss << "  mov rbp, rsp\n";

	if (func->frame.size > 0)
		ngi.ss << "  sub rsp, " << std::to_string(func->frame.size) << "\n";

	genBodyAsm(ngi, func->func.body);
}

void genPrologue(NasmGenInfo& ngi)
{
	// Prologue

	if (ngi.program->platform == "windows")
		ngi.ss << "default rel\n";

	ngi.ss << "  global _start\n";

	std::for_each(
		ngi.program->symbols->begin(),
		ngi.program->symbols->end(),
		[&](SymbolRef sym) {
			if (isExtFunc(sym) && isReachable(sym))
				ngi.ss << "  extern " << sym->func.asmName << "\n";
		}
	);

	// init argc, argv, env
	if (ngi.program->platform == "linux")
	{
		ngi.ss << "section .text\n";
		ngi.ss << "_start:\n";
		ngi.ss << "  pop rax\n";
		ngi.ss << "  mov [__##__argc], rax\n";
		ngi.ss << "  mov [__##__argv], rsp\n";
		ngi.ss << "  inc rax\n";
		ngi.ss << "  mov rcx, 8\n";
		ngi.ss << "  mul rcx\n";
		ngi.ss << "  mov rcx, rsp\n";
		ngi.ss << "  add rcx, rax\n";
		ngi.ss << "  mov [__##__envp], rcx\n";
	}
	else if (ngi.program->platform == "windows")
	{
		ngi.ss << "section .text\n";
		// TODO: LOAD ARGC, ARGV, ENV
		ngi.ss << "_start:\n";
	}

	// Static local initializer test values
	for (int i = 0; i < ngi.program->staticLocalInitCount; ++i)
		ngi.ss << "  mov BYTE [" << getMangledName(getStaticLocalInitName(i), { "bool" }) << "], 1\n";
}

void genEpilogue(NasmGenInfo& ngi)
{
	// Global code epilogue
	if (ngi.program->platform == "linux")
	{
		ngi.ss << "  mov rdi, 0\n";
		ngi.ss << "  mov rax, 60\n";
		ngi.ss << "  syscall\n";
	}
	else if (ngi.program->platform == "windows")
	{
		ngi.ss << "  hlt\n";
	}
}

void genFunctions(NasmGenInfo& ngi)
{
	std::for_each(
		ngi.program->symbols->begin(),
		ngi.program->symbols->end(),
		[&](SymbolRef sym) {
			if (!isFuncSpec(sym) || !isDefined(sym))
				return;
			if (isReachable(sym))
				genFuncAsm(ngi, getParent(sym)->name, sym);
			else
				for (int strID : sym->func.instantiatedStrings)
					--ngi.program->strings.find(strID)->second.first;
		}
	);
}

void genGlobals(NasmGenInfo& ngi)
{
	// Global variables
	ngi.ss << "section .bss\n";
	ngi.ss << "  __##__argc: resq 1\n";
	ngi.ss << "  __##__argv: resq 1\n";
	ngi.ss << "  __##__envp: resq 1\n";

	std::for_each(
		ngi.program->symbols->begin(),
		ngi.program->symbols->end(),
		[&](SymbolRef sym) {
			if (!isVarLabeled(sym))
				return;

			ngi.ss << "  " << getMangledName(sym) << ": resb " << getDatatypeSize(ngi.program, sym->var.datatype) << "\t\t; " << getPosStr(sym->pos) << "\n";
		}
	);
}

void genStrings(NasmGenInfo& ngi)
{
	// C-Strings
	ngi.ss << "section .data\n";
	for (auto& str : ngi.program->strings)
	{
		if (str.second.first <= 0)
			continue;

		ngi.ss << "  " << getLiteralStringName(str.first) << ": db ";
		for (char c : str.second.second)
			ngi.ss << (int)c << ",";
		ngi.ss << "0\n";
	}
}

// Generates Nasm code for the entire program
std::string genAsm(ProgramRef program)
{
	NasmGenInfo ngi;
	ngi.program = program;

	genPrologue(ngi);
	genBodyAsm(ngi, program->body);
	genEpilogue(ngi);
	genFunctions(ngi);
	genGlobals(ngi);
	genStrings(ngi);

	if (!ngi.labelStack.empty())
		THROW_NASM_GEN_ERROR(Token::Position(), "Unused label(s)!");
	
	return ngi.ss.str();
}