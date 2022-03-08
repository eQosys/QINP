#include "NasmGen_Linux_x86_64.h"

#include <stack>
#include <sstream>
#include <cassert>

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
	} state = State::Unused;
	Datatype datatype;
};

typedef CellInfo::State CellState;

struct NasmGenInfo
{
	ProgramRef program;
	std::stringstream ss;
	std::stack<CellInfo> stackCells;
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
		return "[" + secRegName(ngi) + "]";
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
	ngi.ss << "  mov " << secRegName(getDatatypeSize(ngi.program, ngi.primReg.datatype)) << ", " << primRegName(ngi) << "\n";
	ngi.secReg = ngi.primReg;
	if (markUnused)
		ngi.primReg.state = CellState::Unused;
}
void moveSecToPrim(NasmGenInfo& ngi, bool markUnused = true)
{
	ngi.ss << "  mov " << primRegName(getDatatypeSize(ngi.program, ngi.secReg.datatype)) << ", " << secRegName(ngi) << "\n";
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
	
	//pushSecReg(ngi); // Does not preserve the state of the secondary register

	movePrimToSec(ngi);
	popPrimReg(ngi);

	ngi.ss << "  mov " << primRegUsage(ngi) << ", " << secRegName(getDatatypeSize(ngi.program, ngi.primReg.datatype)) << "\n";

	//popSecReg(ngi);
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
	ngi.ss << "  cmp " << primRegUsage(ngi) << ", " << secRegName(getDatatypeSize(ngi.program, ngi.primReg.datatype)) << "\n";
}

void generateNasm_Linux_x86_64(NasmGenInfo& ngi, const Expression* expr)
{
	auto& ss = ngi.ss;

	switch (expr->eType)
	{
	case Expression::ExprType::Conversion:
	{
		generateNasm_Linux_x86_64(ngi, expr->left.get());

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

		if (isUnsignedInt(oldType) && isInteger(newType))
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

		if (isPointer(oldType) && (isPointer(newType) || isInteger(newType)))
		{
			break;
		}

		assert("Invalid conversion!" && false);

		// Datatype & state already modified
	}
		break;
	case Expression::ExprType::Assign:
		generateNasm_Linux_x86_64(ngi, expr->right.get());
		pushPrimReg(ngi);
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		assert(ngi.primReg.datatype == expr->right->datatype && "Assign: datatype mismatch!");
		assert(ngi.primReg.state == CellState::lValue && "Cannot assign to non-lvalue!");
		popSecReg(ngi);
		secRegLToRVal(ngi);

		ss << "  mov " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";

		// Datatype & state don't change
		break;
	case Expression::ExprType::Assign_Sum:
		THROW_NASM_GEN_ERROR(expr->pos, "Assignment by Sum not supported!");
	case Expression::ExprType::Assign_Difference:
		THROW_NASM_GEN_ERROR(expr->pos, "Assignment by Difference not supported!");
	case Expression::ExprType::Assign_Product:
		THROW_NASM_GEN_ERROR(expr->pos, "Assignment by Product not supported!");
	case Expression::ExprType::Assign_Quotient:
		THROW_NASM_GEN_ERROR(expr->pos, "Assignment by Quotient not supported!");
	case Expression::ExprType::Assign_Remainder:
		THROW_NASM_GEN_ERROR(expr->pos, "Assignment by Remainder not supported!");
	case Expression::ExprType::Assign_Bw_LeftShift:
		THROW_NASM_GEN_ERROR(expr->pos, "Assignment by Left Shift not supported!");
	case Expression::ExprType::Assign_Bw_RightShift:
		THROW_NASM_GEN_ERROR(expr->pos, "Assignment by Right Shift not supported!");
	case Expression::ExprType::Assign_Bw_AND:
		THROW_NASM_GEN_ERROR(expr->pos, "Assignment by Bitwise AND not supported!");
	case Expression::ExprType::Assign_Bw_XOR:
		THROW_NASM_GEN_ERROR(expr->pos, "Assignment by Bitwise XOR not supported!");
	case Expression::ExprType::Assign_Bw_OR:
		THROW_NASM_GEN_ERROR(expr->pos, "Assignment by Bitwise OR not supported!");
	case Expression::ExprType::Logical_OR:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		secRegLToRVal(ngi);
		ss << "  or " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		ngi.primReg.datatype = { "bool" };
		// State already modified
		break;
	case Expression::ExprType::Logical_AND:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		secRegLToRVal(ngi);
		ss << "  and " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		ngi.primReg.datatype = { "bool" };
		// State already modified
		break;
	case Expression::ExprType::Bitwise_OR:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		secRegLToRVal(ngi);
		ss << "  or " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		// Datatype doesn't change
		// State already modified
		break;
	case Expression::ExprType::Bitwise_XOR:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		secRegLToRVal(ngi);
		ss << "  xor " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		// Datatype doesn't change
		// State already modified
		break;
	case Expression::ExprType::Bitwise_AND:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		secRegLToRVal(ngi);
		ss << "  and " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		// Datatype doesn't change
		// State already modified
		break;
	case Expression::ExprType::Comparison_Equal:
		generateComparison(ngi, expr);
		ss << "  sete al\n";
		ngi.primReg.datatype = { "bool" };
		ngi.primReg.state = CellState::rValue;
		break;
	case Expression::ExprType::Comparison_NotEqual:
		generateComparison(ngi, expr);
		ss << "  setne al\n";
		ngi.primReg.datatype = { "bool" };
		ngi.primReg.state = CellState::rValue;
		break;
	case Expression::ExprType::Comparison_Less:
		generateComparison(ngi, expr);
		ss << "  setl al\n";
		ngi.primReg.datatype = { "bool" };
		ngi.primReg.state = CellState::rValue;
		break;
	case Expression::ExprType::Comparison_LessEqual:
		generateComparison(ngi, expr);
		ss << "  setle al\n";
		ngi.primReg.datatype = { "bool" };
		ngi.primReg.state = CellState::rValue;
		break;
	case Expression::ExprType::Comparison_Greater:
		generateComparison(ngi, expr);
		ss << "  setg al\n";
		ngi.primReg.datatype = { "bool" };
		ngi.primReg.state = CellState::rValue;
		break;
	case Expression::ExprType::Comparison_GreaterEqual:
		generateComparison(ngi, expr);
		ss << "  setge al\n";
		ngi.primReg.datatype = { "bool" };
		ngi.primReg.state = CellState::rValue;
		break;
	case Expression::ExprType::Shift_Left:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		ss << "  shl " << primRegUsage(ngi) << ", " << secRegName(1) << "\n";
		// Datatype doesn't change
		// State already modified
		break;
	case Expression::ExprType::Shift_Right:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		ss << "  shr " << primRegUsage(ngi) << ", " << secRegName(1) << "\n";
		// Datatype doesn't change
		// State already modified
		break;
	case Expression::ExprType::Sum:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		ss << "  add " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		// Datatype doesn't change
		// State already modified
		break;
	case Expression::ExprType::Difference:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		ss << "  sub " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		// Datatype doesn't change
		// State already modified
		break;
	case Expression::ExprType::Product:
		generateBinaryEvaluation(ngi, expr);
		primRegLToRVal(ngi);
		secRegLToRVal(ngi);
		ss << "  " << instrPrefix(ngi.primReg.datatype) << "mul " << secRegUsage(ngi) << "\n";
		// Datatype doesn't change
		// State already modified
		break;
	case Expression::ExprType::Quotient:
	{
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
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		assert(ngi.primReg.state == CellState::lValue && "Left operand of member access must be lValue");
		ss << ";; MEMBER ACCESS\n";
		ss << "  add " << primRegName(8) << ", " << expr->memberOffset << "\n";
		
		ngi.primReg.datatype = expr->datatype;
		//ngi.primReg.state = CellState::lValue;
		ngi.primReg.state = getRValueIfArray(ngi.primReg.datatype);
	}
		break;
	case Expression::ExprType::MemberAccessDereference:
		assert("Member access dereference should be converted to member access by the program generator" && false);
		break;
	case Expression::ExprType::AddressOf:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		assert(ngi.primReg.state == CellState::lValue && "Cannot take address of non-lvalue!");
		ngi.primReg.datatype = expr->datatype;
		ngi.primReg.state = CellState::rValue;
		break;
	case Expression::ExprType::Dereference:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		assert(ngi.primReg.datatype.ptrDepth != 0 && "Cannot dereference non-pointer!");
		ngi.primReg.datatype = expr->datatype;
		
		switch (ngi.primReg.state)
		{
		case CellState::lValue:
			ss << "  mov " << primRegName(8) << ", " << primRegUsage(ngi) << "\n";
			break;
		case CellState::rValue:
			ngi.primReg.state = getRValueIfArray(ngi.primReg.datatype);
			break;
		default:
			THROW_NASM_GEN_ERROR(expr->pos, "Invalid CellState!");
		}
		// Datatype & state already modified
		break;
	case Expression::ExprType::Logical_NOT:
		generateNasm_Linux_x86_64(ngi, expr->left.get());\
		primRegLToRVal(ngi);
		ss << "  cmp " << primRegUsage(ngi) << ", 0\n";
		ss << "  sete al\n";
		// Datatype doesn't change
		// State already modified
		break;
	case Expression::ExprType::Bitwise_NOT:
	{
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		primRegLToRVal(ngi);
		ss << "  not " << primRegUsage(ngi) << "\n";
		// Datatype doesn't change
		// State already modified
	}
		break;
	case Expression::ExprType::Prefix_Plus:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		// Nothing else to do
		break;
	case Expression::ExprType::Prefix_Minus:
	{
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		primRegLToRVal(ngi);
		ss << "  neg " << primRegUsage(ngi) << "\n";
		if (isUnsignedInt(ngi.primReg.datatype))
			ngi.primReg.datatype.name = "i" + ngi.primReg.datatype.name.substr(1);
		// Datatype doesn't change
		// State already modified
	}
		break;
	case Expression::ExprType::Prefix_Increment:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		assert(ngi.primReg.state == CellState::lValue && "Cannot increment non-lvalue!");
		primRegLToRVal(ngi, true);
		if (isPointer(ngi.primReg.datatype))
			ss << "  add " << primRegUsage(ngi) << ", " << getDatatypePointedToSize(ngi.program, ngi.primReg.datatype) << "\n";
		else
			ss << "  inc " << primRegUsage(ngi) << "\n";
		primRegRToLVal(ngi, true);
		// Datatype & state don't change
		break;
	case Expression::ExprType::Prefix_Decrement:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		assert(ngi.primReg.state == CellState::lValue && "Cannot decrement non-lvalue!");
		primRegLToRVal(ngi, true);
		if (isPointer(ngi.primReg.datatype))
			ss << "  sub " << primRegUsage(ngi) << ", " << getDatatypePointedToSize(ngi.program, ngi.primReg.datatype) << "\n";
		else
			ss << "  dec " << primRegUsage(ngi) << "\n";
		primRegRToLVal(ngi, true);
		// Datatype & state don't change
		break;
	case Expression::ExprType::Subscript:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		assert(ngi.primReg.datatype.ptrDepth != 0 && "Cannot subscript non-pointer!");
		ss << ";; SUBSCRIPT\n";
		primRegLToRVal(ngi);
		pushPrimReg(ngi);
		generateNasm_Linux_x86_64(ngi, expr->right.get());
		primRegLToRVal(ngi);
		ss << "  mov " << secRegName(8) << ", " << std::to_string(getDatatypeSize(ngi.program, expr->datatype)) << "\n";
		ss << "  mul " << secRegName(8) << "\n";
		popSecReg(ngi);
		ss << "  add " << primRegUsage(ngi) << ", " << secRegUsage(ngi) << "\n";
		ngi.primReg.datatype = expr->datatype;
		ngi.primReg.state = getRValueIfArray(ngi.primReg.datatype);
		break;
	case Expression::ExprType::FunctionCall:
	{
		bool isVoidFunc = expr->datatype == Datatype{ "void" };
		if (!isVoidFunc)
			ss << "  sub rsp, " << std::to_string(getDatatypeSize(ngi.program, expr->datatype)) << "\n";

		for (int i = expr->paramExpr.size() - 1; i >= 0; --i)
		{
			generateNasm_Linux_x86_64(ngi, expr->paramExpr[i].get());
			primRegLToRVal(ngi);
			ss << "  push " << primRegName(8) << "\n";
		}

		generateNasm_Linux_x86_64(ngi, expr->left.get());
		bool typesMatch = ngi.primReg.datatype == Datatype{ getSignature(expr), 1 };
		assert(typesMatch && "Cannot call non-function!");
		ss << "  call " << primRegUsage(ngi) << "\n";
		ss << "  add rsp, " << std::to_string(expr->paramSizeSum) << "\n";

		ngi.primReg.datatype = expr->datatype;
		ngi.primReg.state = CellState::rValue;

		if (!isVoidFunc)
			ss << "  pop " << primRegName(8) << "\n";
	}
		break;
	case Expression::ExprType::Suffix_Increment:
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		assert(ngi.primReg.state == CellState::lValue && "Cannot increment non-lvalue!");
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
		generateNasm_Linux_x86_64(ngi, expr->left.get());
		assert(ngi.primReg.state == CellState::lValue && "Cannot decrement non-lvalue!");
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
	case Expression::ExprType::GlobalVariable:
		ss << "  mov " << primRegName(8) << ", " << getMangledName(expr->globName, expr->datatype) << "\n";
		ngi.primReg.datatype = expr->datatype;
		ngi.primReg.state = getRValueIfArray(ngi.primReg.datatype);
		break;
	case Expression::ExprType::LocalVariable:
		ss << "  mov " << primRegName(8) << ", rbp\n";
		ss << "  add " << primRegName(8) << ", " << std::to_string(expr->localOffset) << "\n";
		ngi.primReg.datatype = expr->datatype;
		ngi.primReg.state = getRValueIfArray(ngi.primReg.datatype);
		break;
	case Expression::ExprType::FunctionName:
		ss << "  mov " << primRegName(8) << ", " << expr->funcName << "\n";
		ngi.primReg.datatype = expr->datatype;
		ngi.primReg.state = CellState::rValue;
		break;
	default:
		THROW_NASM_GEN_ERROR(expr->pos, "Unsupported expression type!");
	}
}

void generateNasm_Linux_x86_64(NasmGenInfo& ngi, StatementRef statement);

// Generates Nasm code for any code 'body'
void generateNasm_Linux_x86_64(NasmGenInfo& ngi, BodyRef body)
{
	auto& ss = ngi.ss;

	for (auto& statement : body->statements)
		generateNasm_Linux_x86_64(ngi, statement);
}

void generateNasm_Linux_x86_64(NasmGenInfo& ngi, StatementRef statement)
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

			generateNasm_Linux_x86_64(ngi, condBody.condition.get());

			primRegLToRVal(ngi);
			ss << "  cmp " << primRegUsage(ngi) << ", 0\n";
			ss << "  je " << getLabel(ngi, LABEL_ID_IF_NEXT) << "\n";

			generateNasm_Linux_x86_64(ngi, condBody.body);
		}

		if (statement->elseBody)
		{
			ss << "  jmp " << getLabel(ngi, LABEL_ID_IF_END) << "\n";
			placeLabel(ngi, LABEL_ID_IF_NEXT);
			replaceLabel(ngi, "IF_NEXT", LABEL_ID_IF_NEXT);

			generateNasm_Linux_x86_64(ngi, statement->elseBody);
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

		generateNasm_Linux_x86_64(ngi, statement->whileConditionalBody.condition.get());
		primRegLToRVal(ngi);
		ss << "  cmp " << primRegUsage(ngi) << ", 0\n";
		ss << "  je " << getLabel(ngi, LABEL_ID_WHILE_END) << "\n";

		generateNasm_Linux_x86_64(ngi, statement->whileConditionalBody.body);

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
		placeLabel(ngi, LABEL_ID_CONTINUE);

		generateNasm_Linux_x86_64(ngi, statement->doWhileConditionalBody.body);

		generateNasm_Linux_x86_64(ngi, statement->doWhileConditionalBody.condition.get());

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
		generateNasm_Linux_x86_64(ngi, (Expression*)statement.get());
		break;
	default:
		THROW_NASM_GEN_ERROR(statement->pos, "Unsupported statement type!");
	}
}

// Generates Nasm code for a function
void generateNasm_Linux_x86_64(NasmGenInfo& ngi, const std::string& name, FunctionRef func)
{
	assert(func->body->statements.back()->type == Statement::Type::Return && "Function must end with return statement!");

	// Function prologue
	ngi.ss << getMangledName(func) << ":\n";
	ngi.ss << "  push rbp\n";
	ngi.ss << "  mov rbp, rsp\n";

	if (func->frameSize > 0)
		ngi.ss << "  sub rsp, " << std::to_string(func->frameSize) << "\n";

	generateNasm_Linux_x86_64(ngi, func->body);
}

void writeEscapedString(std::stringstream& ss, const std::string& str)
{
	bool isInString = false;
	for (int i = 0; i < str.size(); ++i)
	{
		char c = str[i];
		switch (c)
		{
		case '\'':
		case '\"':
		case '\\':
		case '\n':
		case '\r':
		case '\t':
		case '\b':
		case '\f':
		case '\v':
		case '\0':
		case '\033':
			if (isInString)
				ss << '"';
			isInString = false;
			if (i != 0)
				ss << ',';
			ss << (int)c;
			break;
		default:
			if (!isInString)
			{
				if (i != 0)
					ss << ',';
				ss << '"';
			}
			isInString = true;
			ss << c;
		}
	}

	if (isInString)
		ss << '"';
}

// Generates Nasm code for the entire program
std::string generateNasm_Linux_x86_64(ProgramRef program)
{
	NasmGenInfo ngi;
	ngi.program = program;
	auto& ss = ngi.ss;

	// Prologue
	ss << "SECTION .text\n";
	ss << "  global _start\n";
	ss << "_start:\n";
	ss << "  pop rax\n";
	ss << "  mov [__##__argc], rax\n";
	ss << "  mov [__##__argv], rsp\n";
	ss << "  inc rax\n";
	ss << "  mov rcx, 8\n";
	ss << "  mul rcx\n";
	ss << "  mov rcx, rsp\n";
	ss << "  add rcx, rax\n";
	ss << "  mov [__##__envp], rcx\n";
	
	// Global code
	generateNasm_Linux_x86_64(ngi, program->body);

	// Global code epilogue
	ss << "  mov rdi, 0\n";
	ss << "  mov rax, 60\n";
	ss << "  syscall\n";

	// Functions
	for (auto& overloads : program->functions)
		for (auto& func : overloads.second)
			if (func.second->isReachable)
				generateNasm_Linux_x86_64(ngi, func.first, func.second);
	
	// Global variables
	ss << "SECTION .bss\n";
	ss << "  __##__argc: resq 1\n";
	ss << "  __##__argv: resq 1\n";
	ss << "  __##__envp: resq 1\n";

	for (auto& glob : program->globals)
	{
		std::string sizeStr;

		int nElements = getDatatypeNumElements(glob.second.datatype);
		int baseSize;

		if (isArray(glob.second.datatype))
			baseSize = getDatatypeSize(ngi.program, { glob.second.datatype.name, 0 });
		else if (!isPointer(glob.second.datatype) && isPackType(program, glob.second.datatype.name))
		{
			baseSize = 1;
			nElements = getDatatypeSize(program, glob.second.datatype);
		}
		else
			baseSize = getDatatypeSize(ngi.program, glob.second.datatype);

		switch (baseSize)
		{
		case 1: sizeStr = "b"; break;
		case 2: sizeStr = "w"; break;
		case 4: sizeStr = "d"; break;
		case 8: sizeStr = "q"; break;
		}
		ss << "  " << getMangledName(glob.second) << ": res" << sizeStr << " " << nElements << "\t\t; " << getPosStr(glob.second.pos) << "\n";
	}

	// C-Strings
	ss << "SECTION .data\n";
	for (auto& str : program->strings)
	{
		ss << "  " << getMangledName(str.first) << ": db ";
		writeEscapedString(ss, str.second);
		ss << ", 0\n";
	}

	if (!ngi.labelStack.empty())
		THROW_NASM_GEN_ERROR(Token::Position(), "Unused label(s)!");
	
	return ss.str();
}