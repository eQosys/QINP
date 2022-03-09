#include "ProgramGenerator.h"

#include <map>
#include <set>
#include <stack>
#include <algorithm>
#include <filesystem>
#include <cassert>

#include "Errors/ProgGenError.h"

#include "Tokenizer.h"
#include "OperatorPrecedence.h"

struct LocalStackFrame
{
	std::map<std::string, Variable> locals;
	int totalOffset = 0; // The offset of the last local variable pushed onto the stack.
	int frameSize = 0; // The size of the stack frame. (Sum of the push sizes of all locals)
};

struct ProgGenInfo
{
	TokenListRef tokens;
	ProgramRef program;
	std::set<std::string> importDirs;
	int currToken = 0;
	struct Indent
	{
		int lvl = 0;
		int nPerLvl = 0;
		std::string chStr = "";
	} indent;
	std::string lastLoadedFunctionName = "";

	std::vector<LocalStackFrame> localStack; // The stack of local variables, mostly one frame per indentation level.

	std::stack<BodyRef> mainBodyBackups;
	int funcRetOffset;
	int funcFrameSize;
	Datatype funcRetType;

	std::set<std::string> imports;

	std::map<std::string, Token> defSymbols;

	enum class Context
	{
		Global,
		Function,
		Pack,
	};
	
	std::stack<Context> contextStack;

	int continueEnableCount = 0;
	int breakEnableCount = 0;

	PackRef currPack = nullptr;
};

struct ProgGenInfoBackup
{
	TokenListRef tokens;
	int currToken;
	ProgGenInfo::Indent indent;
};

ProgGenInfoBackup makeProgGenInfoBackup(const ProgGenInfo& info)
{
	ProgGenInfoBackup backup;
	backup.tokens = info.tokens;
	backup.currToken = info.currToken;
	backup.indent = info.indent;
	return backup;
}

void loadProgGenInfoBackup(ProgGenInfo& info, const ProgGenInfoBackup& backup)
{
	info.tokens = backup.tokens;
	info.currToken = backup.currToken;
	info.indent = backup.indent;
}

const Token& peekToken(ProgGenInfo& info, int offset = 0, bool ignoreSymDef = false)
{
	static const Token emptyToken = { { "", 0, 0 }, Token::Type::EndOfCode, "" };
	int index = info.currToken + offset;

	auto tok = (index < info.tokens->size()) ? &(*info.tokens)[index] : &emptyToken;

	if (ignoreSymDef)
		return *tok;

	while (isIdentifier(*tok))
	{
		auto it = info.defSymbols.find(tok->value);
		if (it == info.defSymbols.end())
			break;
		tok = &it->second;
	}

	return *tok;
}

const Token& nextToken(ProgGenInfo& info, int offset = 1)
{
	auto& temp = peekToken(info, offset - 1);
	info.currToken += offset;
	return temp;
}

void enableContinue(ProgGenInfo& info)
{
	++info.continueEnableCount;
}
void disableContinue(ProgGenInfo& info)
{
	assert(info.continueEnableCount > 0 && "Cannot disable continue, continue is not enabled.");
	--info.continueEnableCount;
}
bool isContinueEnabled(const ProgGenInfo& info)
{
	return info.continueEnableCount > 0;
}

void enableBreak(ProgGenInfo& info)
{
	++info.breakEnableCount;
}
void disableBreak(ProgGenInfo& info)
{
	assert(info.breakEnableCount > 0 && "Cannot disable break, break is not enabled.");
	--info.breakEnableCount;
}
bool isBreakEnabled(const ProgGenInfo& info)
{
	return info.breakEnableCount > 0;
}

LocalStackFrame& currLocalFrame(ProgGenInfo& info)
{
	return info.localStack.back();
}

void pushLocalFrame(ProgGenInfo& info)
{
	info.localStack.push_back({});
	if (info.localStack.size() > 1)
		currLocalFrame(info).totalOffset = info.localStack[info.localStack.size() - 2].totalOffset;
}

void popLocalFrame(ProgGenInfo& info)
{
	info.localStack.pop_back();
}

void pushContext(ProgGenInfo& info, ProgGenInfo::Context context)
{
	info.contextStack.push(context);
}

void popContext(ProgGenInfo& info)
{
	info.contextStack.pop();
}

ProgGenInfo::Context currContext(ProgGenInfo& info)
{
	return info.contextStack.top();
}

void mergeSubUsages(BodyRef parent, BodyRef child)
{
	parent->usedFunctions.merge(child->usedFunctions);
	child->usedFunctions.clear();
}

void pushStatement(ProgGenInfo& info, StatementRef statement)
{
	info.program->body->statements.push_back(statement);
}

StatementRef lastStatement(ProgGenInfo& info)
{
	return info.program->body->statements.back();
}

bool isPackType(ProgGenInfo& info, const std::string& name)
{
	return isPackType(info.program, name);
}

bool isPackType(ProgGenInfo& info, const Token& token)
{
	if (!isIdentifier(token))
		return false;
	return isPackType(info, token.value);
}

bool leftConversionIsProhibited(Expression::ExprType eType)
{
	static const std::set<Expression::ExprType> prohibitedTypes = 
	{
		Expression::ExprType::Assign,
		Expression::ExprType::Assign_Sum,
		Expression::ExprType::Assign_Difference,
		Expression::ExprType::Assign_Product,
		Expression::ExprType::Assign_Quotient,
		Expression::ExprType::Assign_Remainder,
		Expression::ExprType::Assign_Bw_LeftShift,
		Expression::ExprType::Assign_Bw_RightShift,
		Expression::ExprType::Assign_Bw_AND,
		Expression::ExprType::Assign_Bw_XOR,
		Expression::ExprType::Assign_Bw_OR,
		Expression::ExprType::Shift_Left,
		Expression::ExprType::Shift_Right,
	};

	return prohibitedTypes.find(eType) != prohibitedTypes.end();
}

ExpressionRef genConvertExpression(ExpressionRef expToConvert, const Datatype& newDatatype, bool isExplicit = false, bool doThrow = true);

FunctionRef getMatchingOverload(ProgGenInfo& info, const FunctionOverloads& overloads, std::vector<ExpressionRef>& paramExpr)
{
	FunctionRef match = nullptr;

	for (auto& overload : overloads)
	{
		auto& func = overload.second;

		if (func->params.size() != paramExpr.size())
			continue;

		bool matchFound = true;
		bool isExactMatch = true;
		for (int i = 0; i < paramExpr.size() && matchFound; ++i)
		{
			if (paramExpr[i]->datatype != func->params[i].datatype)
				isExactMatch = false;
			matchFound = !!genConvertExpression(paramExpr[i], func->params[i].datatype, false, false);
		}

		if (isExactMatch)
			return func;
		
		if (matchFound)
		{
			if (match)
				THROW_PROG_GEN_ERROR(peekToken(info).pos, "Multiple overloads found for function call!");
			else
				match = func;
		}
	}

	if (match)
	{
		for (int i = 0; i < paramExpr.size(); ++i)
			paramExpr[i] = genConvertExpression(paramExpr[i], match->params[i].datatype, false);
	}

	return match;
}

const Variable* getVariable(ProgGenInfo& info, const std::string& name)
{
	// First check for locals, then for globals (shadowing)
	for (auto frameIt = info.localStack.rbegin(); frameIt != info.localStack.rend(); ++frameIt)
	{
		auto locIt = frameIt->locals.find(name);
		if (locIt != frameIt->locals.end())
			return &locIt->second;
	}

	auto& globals = info.program->globals;
	auto globIt = globals.find(name);
	if (globIt != globals.end())
		return &globIt->second;

	return nullptr;
}

const FunctionOverloads* getFunctions(ProgGenInfo& info, const std::string& name)
{
	auto& functions = info.program->functions;
	auto it = functions.find(name);
	if (it != functions.end())
		return &it->second;

	return nullptr;
}

Datatype getParseDatatype(ProgGenInfo& info);

bool parseEmptyLine(ProgGenInfo& info)
{
	if (peekToken(info).type != Token::Type::Newline)
		return false;
	nextToken(info);
	return true;
}

void parseExpected(ProgGenInfo& info, Token::Type type)
{
	auto& token = nextToken(info);
	if (token.type != type)
		THROW_PROG_GEN_ERROR(token.pos, "Unexpected token '" + token.value + "'!");
}

void parseExpected(ProgGenInfo& info, Token::Type type, const std::string& value)
{
	auto& token = nextToken(info);
	if (token.type != type || token.value != value)
		THROW_PROG_GEN_ERROR(token.pos, "Unexpected token '" + token.value + "'!");
}

void parseExpectedNewline(ProgGenInfo& info)
{
	auto& token = nextToken(info);
	if (!isNewline(token) && !isEndOfCode(token))
		THROW_PROG_GEN_ERROR(token.pos, "Expected newline!");
}

void parseExpectedColon(ProgGenInfo& info)
{
	parseExpected(info, Token::Type::Separator, ":");
}

void addVariable(ProgGenInfo& info, Variable var)
{
	var.modName = var.name;

	if (currContext(info) == ProgGenInfo::Context::Pack)
	{
		var.offset = info.currPack->size;
		info.currPack->size += getDatatypeSize(info.program, var.datatype);

		info.currPack->members.insert({ var.name, var });
		return;
	}

	bool isGlobal = currContext(info) == ProgGenInfo::Context::Global;
	int varSize = getDatatypeSize(info.program, var.datatype);

	if (!info.localStack.empty())
	{
		auto varIt = currLocalFrame(info).locals.find(var.name);
		if (varIt != currLocalFrame(info).locals.end())
			THROW_PROG_GEN_ERROR(var.pos, "Variable with name '" + var.name + "' already declared in the same frame: " + getPosStr(varIt->second.pos));
	}
	else
	{
		if (isGlobal)
		{
			auto varIt = info.program->globals.find(var.name);
			if (varIt != info.program->globals.end())
				THROW_PROG_GEN_ERROR(var.pos, "Variable with name '" + var.name + "' already declared globally: " + getPosStr(varIt->second.pos));
			auto funcs = getFunctions(info, var.name);
			if (funcs)
				THROW_PROG_GEN_ERROR(var.pos, "Function with name '" + var.name + "' already declared: " + getPosStr(funcs->begin()->second->pos));
		}
	}

	var.isLocal = !isGlobal;
	static int globVarID = 0;
	if (isGlobal && !info.localStack.empty())
		var.modName.append("__" + std::to_string(globVarID++));

	if (!isGlobal && !info.localStack.empty())
	{
		varSize = getDatatypeSize(info.program, var.datatype);
		currLocalFrame(info).frameSize += varSize;
		currLocalFrame(info).totalOffset -= varSize;
		var.offset = currLocalFrame(info).totalOffset;
		if (-currLocalFrame(info).totalOffset > info.funcFrameSize)
			info.funcFrameSize = -currLocalFrame(info).totalOffset;
	}

	if (info.localStack.empty())
		info.program->globals.insert({ var.name, var });
	else
	{
		currLocalFrame(info).locals.insert({ var.name, var });
		if (isGlobal)
			info.program->globals.insert({ var.name, var });
	}
}

void addFunction(ProgGenInfo& info, FunctionRef func)
{
	auto varIt = info.program->globals.find(func->name);
	if (varIt != info.program->globals.end())
		THROW_PROG_GEN_ERROR(func->pos, "Variable with name '" + func->name + "' already declared here: " + getPosStr(varIt->second.pos));

	auto sigNoRet = getSignatureNoRet(func);

	auto nameIt = info.program->functions.find(func->name);
	if (nameIt == info.program->functions.end())
	{
		info.program->functions[func->name].insert({ sigNoRet, func });
		return;
	}

	auto sigIt = nameIt->second.find(sigNoRet);

	if (sigIt == nameIt->second.end())
	{
		nameIt->second.insert({ sigNoRet, func });
		return;
	}

	if (func->retType != sigIt->second->retType)
		THROW_PROG_GEN_ERROR(func->pos, "Function with name '" + func->name + "' already declared with different return type: " + getPosStr(sigIt->second->pos));

	if (!func->isDefined)
		return;

	if (sigIt->second->isDefined && func->isDefined)
		THROW_PROG_GEN_ERROR(func->pos, "Function already defined here: " + getPosStr(sigIt->second->pos));

	sigIt->second = func;
}

PackRef addPack(ProgGenInfo& info, PackRef pack)
{
	auto packIt = info.program->packs.find(pack->name);
	if (packIt != info.program->packs.end())
	{
		if (packIt->second->isDefined)
			THROW_PROG_GEN_ERROR(pack->pos, "Pack already defined here: " + getPosStr(packIt->second->pos));
		if (pack->isDefined)
			packIt->second->pos = pack->pos;
		return packIt->second;
	}
	return info.program->packs.insert({ pack->name, pack }).first->second;
}

std::string preprocessAsmCode(ProgGenInfo& info, const Token& asmToken)
{
	std::string result;
	bool parseVar = false;
	bool parsedParen = false;
	std::string varName = "";
	for (int i = 0; i < asmToken.value.size(); ++i)
	{
		char c = asmToken.value[i];

		if (parseVar)
		{
			if (!parsedParen)
			{
				if (c != '(')
					THROW_PROG_GEN_ERROR(asmToken.pos, "Expected '(' after '$'!");
				parsedParen = true;
				continue;
			}

			if (c != ')')
			{
				varName.push_back(c);
				continue;
			}

			bool alreadyPlaced = false;
			auto defIt = info.defSymbols.find(varName);
			while (!alreadyPlaced && defIt != info.defSymbols.end())
			{
				auto& tok = defIt->second;

				if (isIdentifier(tok))
				{
					varName = tok.value;
					defIt = info.defSymbols.find(varName);
					continue;
				}
				
				if (isString(tok))
				{
					result += "\"";
					result += tok.value;
					result += "\"";
					alreadyPlaced = true;
					break;
				}
				
				if (isLiteral(tok))
				{
					result += tok.value;
					alreadyPlaced = true;
					break;
				}
				
				THROW_PROG_GEN_ERROR(asmToken.pos, "Illegal use of symbol!");
			}
			
			if (!alreadyPlaced)
			{
				auto pVar = getVariable(info, varName);
				if (!pVar)
					THROW_PROG_GEN_ERROR(asmToken.pos, "Unknown variable '" + varName + "'!");
				
				if (pVar->isLocal)
				{
					int offset = pVar->offset;
					result += (offset < 0) ? "- " : "+ ";
					result += std::to_string(std::abs(offset));
				}
				else
				{
					result += getMangledName(*pVar);
				}
			}

			parseVar = false;
			parsedParen = false;
			varName = "";

			continue;
		}

		if (c == '$')
			parseVar = true;
		else
			result.push_back(c);
	}

	if (parseVar)
		THROW_PROG_GEN_ERROR(asmToken.pos, "Missing ')'!");
	return result;
}

void increaseIndent(ProgGenInfo::Indent& indent)
{
	++indent.lvl;
}

bool parseIndent(ProgGenInfo& info)
{
	if (info.indent.chStr.empty())
	{
		if (info.indent.lvl == 0)
		{
			if (isWhitespace(peekToken(info)))
				THROW_PROG_GEN_ERROR(peekToken(info).pos, "Unexpected indentation!");
			return true;
		}

		if (info.indent.lvl != 1)
			THROW_PROG_GEN_ERROR(peekToken(info).pos, "Indentation level must be 1 for the first used indentation!");

		auto& firstToken = peekToken(info);
		if (!isWhitespace(firstToken))
			return false;

		nextToken(info);
		info.indent.chStr = firstToken.value;
		info.indent.nPerLvl = 1;

		auto pIndentToken = &peekToken(info);

		while (isWhitespace(*pIndentToken))
		{
			if (pIndentToken->value != info.indent.chStr)
				THROW_PROG_GEN_ERROR(pIndentToken->pos, "Inconsistent indentation!");
			nextToken(info);
			++info.indent.nPerLvl;
			pIndentToken = &peekToken(info);
		}

		return true;
	}

	int indentCount = 0;
	for (int i = 0; i < info.indent.nPerLvl * info.indent.lvl; ++i)
	{
		auto& token = peekToken(info, indentCount);
		if (!isWhitespace(token) || token.value != info.indent.chStr)
			return false;
		++indentCount;
	}

	nextToken(info, indentCount);

	if (peekToken(info).type == Token::Type::Whitespace)
			THROW_PROG_GEN_ERROR(peekToken(info).pos, "Inconsistent indentation!");

	return true;
}

void unparseIndent(ProgGenInfo& info)
{
	nextToken(info, -info.indent.nPerLvl * info.indent.lvl);
}

void decreaseIndent(ProgGenInfo::Indent& indent)
{
	if (indent.lvl == 0)
		THROW_PROG_GEN_ERROR(Token::Position(), "Indent level already 0!");
	--indent.lvl;
}

void pushTempBody(ProgGenInfo& info, BodyRef body)
{
	info.mainBodyBackups.push(info.program->body);
	info.program->body = body;
}

void popTempBody(ProgGenInfo& info)
{
	if (info.mainBodyBackups.empty())
		THROW_PROG_GEN_ERROR(Token::Position(), "No backup body to pop!");

	info.program->body = info.mainBodyBackups.top();
	info.mainBodyBackups.pop();
}

Datatype getBestConvDatatype(const Datatype& left, const Datatype& right)
{
	if (left.ptrDepth > 0 || right.ptrDepth > 0)
		return Datatype();

	if (!isBuiltinType(left.name) || !isBuiltinType(right.name))
		return Datatype();
	
	static const std::vector<std::string> typeOrder = 
	{
		"bool", "i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64",
	};
	
	auto leftIt = std::find(typeOrder.begin(), typeOrder.end(), left.name);
	auto rightIt = std::find(typeOrder.begin(), typeOrder.end(), right.name);
	if (leftIt == typeOrder.end() || rightIt == typeOrder.end())
		return Datatype();

	return (leftIt > rightIt) ? left : right;
}

ExpressionRef makeConvertExpression(ExpressionRef expToConvert, const Datatype& newDatatype)
{
	auto exp = std::make_shared<Expression>(expToConvert->pos);
	exp->eType = Expression::ExprType::Conversion;
	exp->left = expToConvert;
	exp->datatype = newDatatype;
	return exp;
}

ExpressionRef genAutoArrayToPtr(ExpressionRef expToConvert)
{
	if (!isArray(expToConvert->datatype))
		return expToConvert;

	return makeConvertExpression(expToConvert, { expToConvert->datatype.name, 1 });
}

ExpressionRef genConvertExpression(ExpressionRef expToConvert, const Datatype& newDatatype, bool isExplicit, bool doThrow)
{
	expToConvert = genAutoArrayToPtr(expToConvert);

	if (expToConvert->datatype == newDatatype)
		return expToConvert;


	if (isBool(expToConvert->datatype))
	{
		if (isInteger(newDatatype))
			return makeConvertExpression(expToConvert, newDatatype);
		
		else if (isExplicit && isPointer(newDatatype))
			return makeConvertExpression(expToConvert, newDatatype);

		else if (doThrow)
			THROW_PROG_GEN_ERROR(expToConvert->pos, "Cannot implicitly convert from '" + getDatatypeStr(expToConvert->datatype) + "' to '" + getDatatypeStr(newDatatype) + "'!");
		else
			return nullptr;
	}
	if (isInteger(expToConvert->datatype))
	{
		if (isBool(newDatatype))
			return makeConvertExpression(expToConvert, newDatatype);
		if (isInteger(newDatatype))
			return makeConvertExpression(expToConvert, newDatatype);

		else if (isExplicit && isPointer(newDatatype))
			return makeConvertExpression(expToConvert, newDatatype);
		
		else if (doThrow)
			THROW_PROG_GEN_ERROR(expToConvert->pos, "Cannot implicitly convert from '" + getDatatypeStr(expToConvert->datatype) + "' to '" + getDatatypeStr(newDatatype) + "'!");
		else
			return nullptr;
	}
	if (isPointer(expToConvert->datatype))
	{
		if (isBool(newDatatype))
			return makeConvertExpression(expToConvert, newDatatype);
		else if (expToConvert->datatype.ptrDepth == 1 && isVoidPtr(newDatatype))
			return makeConvertExpression(expToConvert, newDatatype);
		else if (newDatatype == Datatype{ "u64" })
			return makeConvertExpression(expToConvert, newDatatype);
		
		else if (isExplicit && isInteger(newDatatype))
			return makeConvertExpression(expToConvert, newDatatype);
		else if (isExplicit && isPointer(newDatatype))
			return makeConvertExpression(expToConvert, newDatatype);
		
		else if (doThrow)
			THROW_PROG_GEN_ERROR(expToConvert->pos, "Cannot implicitly convert from '" + getDatatypeStr(expToConvert->datatype) + "' to '" + getDatatypeStr(newDatatype) + "'!");
		else
			return nullptr;
	}

	if (doThrow)
		THROW_PROG_GEN_ERROR(expToConvert->pos, "Cannot implicitly convert from '" + getDatatypeStr(expToConvert->datatype) + "' to '" + getDatatypeStr(newDatatype) + "'!");
	return nullptr;
}

void autoFixDatatypeMismatch(ExpressionRef exp)
{
	if (exp->left->datatype == exp->right->datatype)
		return;

	if (isPointer(exp->left->datatype))
	{
		switch (exp->eType)
		{
		case Expression::ExprType::Sum:
		case Expression::ExprType::Difference:
		case Expression::ExprType::Assign_Sum:
		case Expression::ExprType::Assign_Difference:
			exp->right = genConvertExpression(exp->right, { "u64" });
			return;
		}
	}

	Datatype newDatatype;
	if (leftConversionIsProhibited(exp->eType))
		newDatatype = exp->left->datatype;
	else
		newDatatype = getBestConvDatatype(exp->left->datatype, exp->right->datatype);

	if (!newDatatype)
		THROW_PROG_GEN_ERROR(
			exp->pos, "Cannot convert " + 
			getDatatypeStr(exp->left->datatype) + " and " +
			getDatatypeStr(exp->right->datatype) + " to a common datatype!");
	
	exp->left = genConvertExpression(exp->left, newDatatype);
	exp->right = genConvertExpression(exp->right, newDatatype);
}

ExpressionRef getParseExpression(ProgGenInfo& info, int precLvl = 0);

ExpressionRef getParseLiteral(ProgGenInfo& info)
{
	auto& litToken = peekToken(info);
	if (!isLiteral(litToken))
		return nullptr;

	nextToken(info);
	ExpressionRef exp = std::make_shared<Expression>(litToken.pos);

	exp->eType = Expression::ExprType::Literal;

	switch (litToken.type)
	{
	case Token::Type::LiteralInteger:
		exp->valStr = litToken.value;
		exp->datatype = { (std::stoull(exp->valStr) >> 63) ? "u64" : "i64" };
		break;
	case Token::Type::LiteralChar:
		exp->valStr = std::to_string(litToken.value[0]);
		exp->datatype = { "u8" };
		break;
	case Token::Type::LiteralBoolean:
		exp->valStr = std::to_string(litToken.value == "true" ? 1 : 0);
		exp->datatype = { "bool" };
		break;
	case Token::Type::String:
		exp->valStr = getMangledName(info.program->strings.size());
		info.program->strings.insert({ info.program->strings.size(), litToken.value });
		exp->datatype = { "u8", 1, { (int)litToken.value.size() + 1 } };
		break;
	default:
		THROW_PROG_GEN_ERROR(litToken.pos, "Invalid literal type!");
	}


	return exp;
}

ExpressionRef getParseVariable(ProgGenInfo& info)
{
	auto& litToken = peekToken(info);
	if (!isIdentifier(litToken))
		return nullptr;

	ExpressionRef exp = std::make_shared<Expression>(litToken.pos);

	auto pVar = getVariable(info, litToken.value);
	if (!pVar)
		return nullptr;

	exp->eType = pVar->isLocal ? Expression::ExprType::LocalVariable : Expression::ExprType::GlobalVariable;
	exp->localOffset = pVar->offset;
	exp->datatype = pVar->datatype;
	exp->globName = pVar->modName;
	exp->isLValue = isArray(exp->datatype) ? false : true;

	nextToken(info);

	return exp;
}

ExpressionRef getParseFunctionName(ProgGenInfo& info)
{
	auto& litToken = peekToken(info);
	if (!isIdentifier(litToken))
		return nullptr;

	ExpressionRef exp = std::make_shared<Expression>(litToken.pos);

	auto funcs = getFunctions(info, litToken.value);
	if (!funcs)
		return nullptr;

	exp->eType = Expression::ExprType::FunctionName;
	exp->isLValue = false;
	exp->funcName = litToken.value;

	exp->datatype = { "...#" + litToken.value, 1 };

	nextToken(info);

	return exp;
}

ExpressionRef getParseValue(ProgGenInfo& info)
{
	auto exp = getParseLiteral(info);
	if (exp) return exp;

	exp = getParseVariable(info);
	if (exp) return exp;

	exp = getParseFunctionName(info);
	if (exp) return exp;

	THROW_PROG_GEN_ERROR(peekToken(info).pos, "Expected variable name or literal value!");
}

ExpressionRef getParseParenthesized(ProgGenInfo& info)
{
	auto& parenOpen = peekToken(info);
	if (!isSeparator(parenOpen, "("))
		return getParseValue(info);
	
	nextToken(info);
	auto exp = getParseExpression(info);

	auto& parenClose = nextToken(info);
	if (!isSeparator(parenClose, ")"))
		THROW_PROG_GEN_ERROR(parenClose.pos, "Expected ')'!");

	return exp;
}

ExpressionRef getParseBinaryExpression(ProgGenInfo& info, int precLvl)
{
	auto& opsLvl = opPrecLvls[precLvl];

	auto exp = getParseExpression(info, precLvl + 1);

	const Token* pOpToken = nullptr;
	std::map<std::string, Expression::ExprType>::iterator it;
	while ((it = opsLvl.ops.find((pOpToken = &peekToken(info))->value)) != opsLvl.ops.end() && isSepOpKey(*pOpToken))
	{
		{
			auto temp = std::make_shared<Expression>(pOpToken->pos);
			temp->left = exp;
			exp = temp;
		}
		exp->eType = it->second;
		nextToken(info);

		switch (exp->eType)
		{
		case Expression::ExprType::Assign:
		case Expression::ExprType::Assign_Sum:
		case Expression::ExprType::Assign_Difference:
		case Expression::ExprType::Assign_Product:
		case Expression::ExprType::Assign_Quotient:
		case Expression::ExprType::Assign_Remainder:
		case Expression::ExprType::Assign_Bw_LeftShift:
		case Expression::ExprType::Assign_Bw_RightShift:
		case Expression::ExprType::Assign_Bw_AND:
		case Expression::ExprType::Assign_Bw_XOR:
		case Expression::ExprType::Assign_Bw_OR:
			exp->right = getParseExpression(info, precLvl + 1);
			autoFixDatatypeMismatch(exp);
			if (!exp->left->isLValue)
				THROW_PROG_GEN_ERROR(exp->left->pos, "Cannot assign to non-lvalue!");
			exp->datatype = exp->left->datatype;
			exp->isLValue = true;
			break;
		case Expression::ExprType::Logical_OR:
		case Expression::ExprType::Logical_AND:
			exp->right = getParseExpression(info, precLvl + 1);
			exp->left = genConvertExpression(exp->left, { "bool" });
			exp->right = genConvertExpression(exp->right, { "bool" });
			exp->datatype = { "bool" };
			exp->isLValue = false;
			break;
		case Expression::ExprType::Bitwise_OR:
		case Expression::ExprType::Bitwise_XOR:
		case Expression::ExprType::Bitwise_AND:
			exp->right = getParseExpression(info, precLvl + 1);
			autoFixDatatypeMismatch(exp);
			exp->datatype = exp->left->datatype;
			exp->isLValue = false;
			break;
		case Expression::ExprType::Comparison_Equal:
		case Expression::ExprType::Comparison_NotEqual:
		case Expression::ExprType::Comparison_Less:
		case Expression::ExprType::Comparison_LessEqual:
		case Expression::ExprType::Comparison_Greater:
		case Expression::ExprType::Comparison_GreaterEqual:
			exp->right = getParseExpression(info, precLvl + 1);
			autoFixDatatypeMismatch(exp);
			exp->datatype = { "bool" };
			exp->isLValue = false;
			break;
		case Expression::ExprType::Shift_Left:
		case Expression::ExprType::Shift_Right:
		case Expression::ExprType::Sum:
		case Expression::ExprType::Difference:
		case Expression::ExprType::Product:
		case Expression::ExprType::Quotient:
		case Expression::ExprType::Remainder:
			exp->right = getParseExpression(info, precLvl + 1);
			autoFixDatatypeMismatch(exp);
			exp->datatype = exp->left->datatype;
			exp->isLValue = false;
			break;
		case Expression::ExprType::MemberAccess:
		case Expression::ExprType::MemberAccessDereference:
		{
			auto& memberToken = nextToken(info);
			if (!isIdentifier(memberToken))
				THROW_PROG_GEN_ERROR(pOpToken->pos, "Expected member name!");
			switch (exp->eType)
			{
			case Expression::ExprType::MemberAccess:
				if (isPointer(exp->left->datatype))
					THROW_PROG_GEN_ERROR(exp->pos, "Cannot access member of pointer!");
				if (isArray(exp->left->datatype))
					THROW_PROG_GEN_ERROR(exp->pos, "Cannot access member of array!");
				break;
			case Expression::ExprType::MemberAccessDereference:
			{
				exp->eType = Expression::ExprType::MemberAccess;
				exp->left = genAutoArrayToPtr(exp->left);
				if (!isPointer(exp->left->datatype))
					THROW_PROG_GEN_ERROR(exp->pos, "Cannot dereference non-pointer!");
				if (exp->left->datatype.ptrDepth > 1)
					THROW_PROG_GEN_ERROR(exp->pos, "Cannot access member of pointer to pointer!");

				auto temp = std::make_shared<Expression>(exp->pos);
				temp->eType = Expression::ExprType::Dereference;
				temp->left = exp->left;

				temp->datatype = temp->left->datatype;
				dereferenceDatatype(temp->datatype);
				temp->isLValue = isArray(temp->datatype) ? false : true;

				exp->left = temp;
			}
				break;
			}
			if (!isPackType(info, exp->left->datatype.name))
				THROW_PROG_GEN_ERROR(exp->left->pos, "Cannot access member of non-pack type!");

			auto& pack = info.program->packs.at(exp->left->datatype.name);
			if (!pack->isDefined)
				THROW_PROG_GEN_ERROR(exp->left->pos, "Cannot access member of declared-only pack type!");

			auto memberIt = pack->members.find(memberToken.value);
			if (memberIt == pack->members.end())
				THROW_PROG_GEN_ERROR(memberToken.pos, "Unknown member!");
			exp->datatype = memberIt->second.datatype;
			exp->memberOffset = memberIt->second.offset;
			exp->isLValue = true;
		}
			break;
		default:
			THROW_PROG_GEN_ERROR(exp->pos, "Invalid binary operator!");
		}
	}

	return exp;
}

ExpressionRef getParseUnarySuffixExpression(ProgGenInfo& info, int precLvl)
{
	auto exp = getParseExpression(info, precLvl + 1);
	
	auto& opsLvl = opPrecLvls[precLvl];

	const Token* pOpToken = nullptr;
	std::map<std::string, Expression::ExprType>::iterator it;
	while ((it = opsLvl.ops.find((pOpToken = &peekToken(info))->value)) != opsLvl.ops.end() && isSepOpKey(*pOpToken))
	{
		{
			auto temp = std::make_shared<Expression>(pOpToken->pos);
			temp->left = exp;
			exp = temp;
		}
		exp->eType = it->second;
		nextToken(info);

		switch (exp->eType)
		{
		case Expression::ExprType::Subscript:
			exp->datatype = exp->left->datatype;
			if (exp->datatype.ptrDepth == 0)
				THROW_PROG_GEN_ERROR(exp->pos, "Cannot subscript non-pointer type!");
			if (exp->datatype == Datatype{ "void", 1 })
				THROW_PROG_GEN_ERROR(exp->pos, "Cannot subscript pointer to void type!");
			dereferenceDatatype(exp->datatype);
			exp->isLValue = isArray(exp->datatype) ? false : true;
			exp->right = genConvertExpression(getParseExpression(info), { "u64" });
			parseExpected(info, Token::Type::Separator, "]");
			break;
		case Expression::ExprType::FunctionCall:
		{
			exp->isLValue = false;
			if (exp->left->datatype.ptrDepth == 0)
				THROW_PROG_GEN_ERROR(exp->pos, "Cannot call non-function!");
			while (!isSeparator(peekToken(info), ")"))
			{
				exp->paramExpr.push_back(getParseExpression(info));
				if (!isSeparator(peekToken(info), ")"))
					parseExpected(info, Token::Type::Separator, ",");
			}
			parseExpected(info, Token::Type::Separator, ")");

			if (exp->left->eType == Expression::ExprType::FunctionName)
			{
				auto overloads = getFunctions(info, exp->left->funcName);
				if (!overloads)
					THROW_PROG_GEN_ERROR(exp->pos, "Function '" + exp->left->funcName + "' not found!");
				auto func = getMatchingOverload(info, *overloads, exp->paramExpr);
				if (!func)
					THROW_PROG_GEN_ERROR(exp->pos, "No matching overload found for function '" + exp->left->funcName + "'!");
				exp->datatype = func->retType;
				exp->left->datatype = { getSignature(func), 1 };
				exp->left->funcName = getMangledName(exp->left->funcName, exp.get());

				info.program->body->usedFunctions.insert({ func->name, getSignatureNoRet(func) }); // Used to determine which functions are actually used in the executable
			}
			else
			{
				THROW_PROG_GEN_ERROR(exp->pos, "Not implemented yet!");
				// TODO: Check if signature matches (possibly through implicit conversions)
				exp->datatype = {}; // TODO: Get return type from exp->left->datatype
			}

			exp->paramSizeSum = 0;
			for (auto& param : exp->paramExpr)
				exp->paramSizeSum += getDatatypePushSize(info.program, param->datatype);

		}
			break;
		case Expression::ExprType::Suffix_Increment:
		case Expression::ExprType::Suffix_Decrement:
			exp->datatype = exp->left->datatype;
			exp->isLValue = false;
			break;
		}
	}

	return exp;
}

ExpressionRef getParseUnaryPrefixExpression(ProgGenInfo& info, int precLvl)
{
	auto& opsLvl = opPrecLvls[precLvl];

	auto opToken = peekToken(info);
	auto it = opsLvl.ops.find(opToken.value);
	if (it == opsLvl.ops.end() || !isSepOpKey(opToken))
		return getParseExpression(info, precLvl + 1);

	auto exp = std::make_shared<Expression>(opToken.pos);
	exp->eType = it->second;
	nextToken(info);

	switch (exp->eType)
	{
	case Expression::ExprType::AddressOf:
		exp->left = getParseExpression(info, precLvl);
		if (!exp->left->isLValue)
			THROW_PROG_GEN_ERROR(exp->pos, "Cannot take address of non-lvalue!");
		exp->datatype = exp->left->datatype;
		++exp->datatype.ptrDepth;
		exp->isLValue = false;
		break;
	case Expression::ExprType::Dereference:
		exp->left = getParseExpression(info, precLvl);
		exp->datatype = exp->left->datatype;
		if (exp->datatype.ptrDepth == 0)
			THROW_PROG_GEN_ERROR(opToken.pos, "Cannot dereference a non-pointer!");
		if (exp->datatype == Datatype{ "void", 1 })
			THROW_PROG_GEN_ERROR(opToken.pos, "Cannot dereference pointer to void!");
		dereferenceDatatype(exp->datatype);
		exp->isLValue = isArray(exp->datatype) ? false : true;
		break;
	case Expression::ExprType::Logical_NOT:
		exp->left = getParseExpression(info, precLvl);
		exp->datatype = { "bool" };
		exp->isLValue = false;
		break;
	case Expression::ExprType::Bitwise_NOT:
		exp->left = getParseExpression(info, precLvl);
		exp->datatype = exp->left->datatype;
		exp->isLValue = false;
		break;
	case Expression::ExprType::Prefix_Plus:
	case Expression::ExprType::Prefix_Minus:
	case Expression::ExprType::Prefix_Increment:
	case Expression::ExprType::Prefix_Decrement:
		exp->left = getParseExpression(info, precLvl);
		exp->datatype = exp->left->datatype;
		exp->isLValue = false;
		break;
	case Expression::ExprType::Explicit_Cast:
	{
		Datatype newDatatype = getParseDatatype(info);
		if (!newDatatype)
		{
			nextToken(info, -1);
			return getParseExpression(info, precLvl + 1);
		}
		parseExpected(info, Token::Type::Separator, ")");
		exp = genConvertExpression(getParseExpression(info, precLvl), newDatatype, true);
	}
		break;
	case Expression::ExprType::SizeOf:
	{
		parseExpected(info, Token::Type::Separator, "(");
		auto datatype = getParseDatatype(info);
		if (!!datatype)
			parseExpected(info, Token::Type::Separator, ")");
		else
		{
			nextToken(info, -1);
			datatype = getParseParenthesized(info)->datatype;
		}
		exp->valStr = std::to_string(getDatatypeSize(info.program, datatype));
		exp->eType = Expression::ExprType::Literal;
		exp->datatype = { "u64" };
	}
		break;
	default:
		THROW_PROG_GEN_ERROR(opToken.pos, "Unknown unary prefix expression!");
	}

	return exp;
}

ExpressionRef getParseExpression(ProgGenInfo& info, int precLvl)
{
	static const int maxPrecLvl = opPrecLvls.size() - 1;

	if (precLvl > maxPrecLvl)
		return getParseParenthesized(info);

	switch (opPrecLvls[precLvl].type)
	{
	case OpPrecLvl::Type::Binary: return getParseBinaryExpression(info, precLvl);
	case OpPrecLvl::Type::Unary_Suffix: return getParseUnarySuffixExpression(info, precLvl);
	case OpPrecLvl::Type::Unary_Prefix: return getParseUnaryPrefixExpression(info, precLvl);
	default: THROW_PROG_GEN_ERROR(peekToken(info).pos, "Unknown operator precedence level type!");
	}
}

bool parseExpression(ProgGenInfo& info)
{
	auto exp = getParseExpression(info);
	if (exp)
		pushStatement(info, exp);
	parseExpectedNewline(info);
	return !!exp;
}

bool parseExpression(ProgGenInfo& info, const Datatype& targetType)
{
	auto expr = getParseExpression(info);
	if (!expr)
		return false;

	if (expr->datatype != targetType)
		expr = genConvertExpression(expr, targetType);
	pushStatement(info, expr);

	return true;
}

Datatype getParseDatatype(ProgGenInfo& info)
{
	auto& typeToken = peekToken(info);
	if (!isBuiltinType(typeToken) && !isPackType(info, typeToken))
		return Datatype();
	nextToken(info);

	Datatype datatype;
	datatype.name = typeToken.value;

	while (isOperator(peekToken(info), "*"))
	{
		nextToken(info);
		++datatype.ptrDepth;
	}

	return datatype;
}

void parseFunctionBody(ProgGenInfo& info);

ExpressionRef getParseDeclDefVariable(ProgGenInfo& info, const Datatype& datatype, const std::string& name)
{
	Variable var;
	var.pos = peekToken(info).pos;
	var.name = name;
	var.datatype = datatype;

	while (isSeparator(peekToken(info), "["))
	{
		nextToken(info);
		parseExpected(info, Token::Type::LiteralInteger);
		++var.datatype.ptrDepth;
		var.datatype.arraySizes.push_back(std::stoull(peekToken(info, -1).value));
		if (var.datatype.arraySizes.back() <= 0)
			THROW_PROG_GEN_ERROR(peekToken(info, -1).pos, "Array size must be greater than zero!");
		parseExpected(info, Token::Type::Separator, "]");
	}

	if (isPackType(info.program, var.datatype))
		if (!info.program->packs.at(var.datatype.name)->isDefined)
			THROW_PROG_GEN_ERROR(var.pos, "Cannot use declared-only pack type!");

	addVariable(info, var);

	ExpressionRef initExpr = nullptr;
	if (currContext(info) != ProgGenInfo::Context::Pack)
	{
		if (isOperator(peekToken(info), "="))
		{
			nextToken(info, -1);
			initExpr = getParseExpression(info);
		}
	}
	
	parseExpectedNewline(info);

	return initExpr;
}

void parseExpectedDeclDefVariable(ProgGenInfo& info, const Datatype& datatype, const std::string& name)
{
	if (datatype == Datatype{ "void" })
		THROW_PROG_GEN_ERROR(peekToken(info).pos, "Cannot declare variable of type void!");

	auto initExpr = getParseDeclDefVariable(info, datatype, name);
	if (initExpr)
		pushStatement(info, initExpr);
}

void parseExpectedDeclDefFunction(ProgGenInfo& info, const Datatype& datatype, const std::string& name)
{
	FunctionRef func = std::make_shared<Function>();
	func->pos = peekToken(info).pos;
	func->body = std::make_shared<Body>();
	func->name = name;
	func->retType = datatype;

	parseExpected(info, Token::Type::Separator, "(");

	while (!isSeparator(peekToken(info), ")"))
	{
		Variable param;
		param.isLocal = true;

		param.datatype = getParseDatatype(info);
		if (!param.datatype)
			THROW_PROG_GEN_ERROR(peekToken(info).pos, "Expected datatype!");
		
		param.offset = func->retOffset;
		func->retOffset += getDatatypePushSize(info.program, param.datatype);

		if (!isIdentifier(peekToken(info)))
			THROW_PROG_GEN_ERROR(peekToken(info).pos, "Expected identifier!");

		param.name = nextToken(info).value;
		param.modName = param.name;

		func->params.push_back(param);

		if (isSeparator(peekToken(info), ")"))
			continue;

		parseExpected(info, Token::Type::Separator, ",");
	}

	parseExpected(info, Token::Type::Separator, ")");

	func->isDefined = true;
	if (isSeparator(peekToken(info), "..."))
	{
		func->isDefined = false;

		nextToken(info);
		parseExpectedNewline(info);
	}

	addFunction(info, func);

	if (func->isDefined)
	{
		parseExpectedColon(info);
		parseExpectedNewline(info);

		increaseIndent(info.indent);
		pushTempBody(info, func->body);
		pushContext(info, ProgGenInfo::Context::Function);
		pushLocalFrame(info);

		info.funcRetOffset = func->retOffset;
		info.funcRetType = func->retType;
		info.funcFrameSize = 0;

		for (auto& param : func->params)
			currLocalFrame(info).locals.insert({ param.name, param });

		parseFunctionBody(info);
		func->frameSize = info.funcFrameSize;
		
		popLocalFrame(info);
		popContext(info);
		popTempBody(info);
		decreaseIndent(info.indent);
	}
}

bool parseDeclDef(ProgGenInfo& info, bool allowFunctions)
{
	auto& typeToken = peekToken(info);

	auto datatype = getParseDatatype(info);
	if (!datatype)
		return false;

	auto& nameToken = nextToken(info);
	if (!isIdentifier(nameToken))
		THROW_PROG_GEN_ERROR(nameToken.pos, "Expected identifier!");

	if (getDatatypeSize(info.program, datatype) < 0)
		THROW_PROG_GEN_ERROR(typeToken.pos, "Unknown datatype!");

	if (isSeparator(peekToken(info), "("))
	{
		if (!allowFunctions)
			THROW_PROG_GEN_ERROR(peekToken(info).pos, "Functions are not allowed here!");
		parseExpectedDeclDefFunction(info, datatype, nameToken.value);
	}
	else
	{
		parseExpectedDeclDefVariable(info, datatype, nameToken.value);
	}

	return true;
}

bool parseStatementReturn(ProgGenInfo& info)
{
	auto& retToken = peekToken(info);
	if (!isKeyword(retToken, "return"))
		return false;

	nextToken(info);
	auto& exprBegin = peekToken(info);

	pushStatement(info, std::make_shared<Statement>(retToken.pos, Statement::Type::Return));
	lastStatement(info)->funcRetOffset = info.funcRetOffset;

	if (info.funcRetType != Datatype{ "void" })
		lastStatement(info)->subExpr = genConvertExpression(getParseExpression(info), info.funcRetType);

	parseExpectedNewline(info);

	return true;
}

bool parseSinglelineAssembly(ProgGenInfo& info)
{
	auto& asmToken = peekToken(info);
	if (!isKeyword(asmToken, "asm"))
		return false;
	nextToken(info);
	
	parseExpectedColon(info);
	
	auto& strToken = nextToken(info);

	if (!isString(strToken))
		THROW_PROG_GEN_ERROR(strToken.pos, "Expected assembly string!");

	parseExpectedNewline(info);

	pushStatement(info, std::make_shared<Statement>(asmToken.pos, Statement::Type::Assembly));
	lastStatement(info)->asmLines.push_back(preprocessAsmCode(info, strToken));


	return true;
}

bool parseMultilineAssembly(ProgGenInfo& info)
{
	auto& asmToken = peekToken(info);
	if (!isKeyword(asmToken, "assembly"))
		return false;
	nextToken(info);
	
	parseExpectedColon(info);
	parseExpectedNewline(info);

	increaseIndent(info.indent);

	while (true)
	{
		if (!parseIndent(info))
			break;
		auto& strToken = nextToken(info);
		if (!isString(strToken))
			THROW_PROG_GEN_ERROR(strToken.pos, "Expected assembly string!");

		parseExpectedNewline(info);

		pushStatement(info, std::make_shared<Statement>(asmToken.pos, Statement::Type::Assembly));
		lastStatement(info)->asmLines.push_back(preprocessAsmCode(info, strToken));
	}

	decreaseIndent(info.indent);

	return true;
}

bool parseStatementPass(ProgGenInfo& info)
{
	auto& passToken = peekToken(info);
	if (!isKeyword(passToken, "pass"))
		return false;

	nextToken(info);
	parseExpectedNewline(info);

	return true;
}

bool parseControlFlow(ProgGenInfo& info);

bool parseStatementContinue(ProgGenInfo& info)
{
	auto& contToken = peekToken(info);
	if (!isKeyword(contToken, "continue"))
		return false;
	nextToken(info);
	parseExpectedNewline(info);

	pushStatement(info, std::make_shared<Statement>(contToken.pos, Statement::Type::Continue));

	return true;
}

bool parseStatementBreak(ProgGenInfo& info)
{
	auto& breakToken = peekToken(info);
	if (!isKeyword(breakToken, "break"))
		return false;
	nextToken(info);

	parseExpectedNewline(info);

	pushStatement(info, std::make_shared<Statement>(breakToken.pos, Statement::Type::Break));

	return true;
}

void parseBody(ProgGenInfo& info)
{
	int numStatements = 0;

	auto& bodyBeginToken = peekToken(info, -1);

	while (parseIndent(info))
	{
		++numStatements;
		auto token = (*info.tokens)[info.currToken];
		if (parseEmptyLine(info)) continue;
		if (parseStatementPass(info)) continue;
		if (parseControlFlow(info)) continue;
		if (parseStatementContinue(info)) continue;
		if (parseStatementBreak(info)) continue;
		if (parseDeclDef(info, false)) continue;
		if (parseStatementReturn(info)) continue;
		if (parseSinglelineAssembly(info)) continue;
		if (parseMultilineAssembly(info)) continue;
		if (parseExpression(info)) continue;
		THROW_PROG_GEN_ERROR(token.pos, "Unexpected token: " + token.value + "!");
	}

	if (numStatements == 0)
		THROW_PROG_GEN_ERROR(bodyBeginToken.pos, "Expected non-empty body!");
}

void parseBodyEx(ProgGenInfo& info, BodyRef body)
{
	increaseIndent(info.indent);
	pushTempBody(info, body);
	pushLocalFrame(info);

	parseBody(info);

	popLocalFrame(info);
	popTempBody(info);
	decreaseIndent(info.indent);

	mergeSubUsages(info.program->body, body);
}

bool parseStatementIf(ProgGenInfo& info)
{
	auto& ifToken = peekToken(info);
	if (!isKeyword(ifToken, "if"))
		return false;

	auto statement = std::make_shared<Statement>(ifToken.pos, Statement::Type::If_Clause);

	bool parsedIndent = true;

	do
	{
		nextToken(info);

		statement->ifConditionalBodies.push_back({});
		auto& condBody = statement->ifConditionalBodies.back();
		condBody.condition = genConvertExpression(getParseExpression(info), { "bool" });
		condBody.body = std::make_shared<Body>();

		parseExpectedColon(info);
		parseExpectedNewline(info);

		parseBodyEx(info, condBody.body);

		parsedIndent = parseIndent(info);
		if (!parsedIndent)
			break;

	} while (isKeyword(peekToken(info), "elif"));

	if (parsedIndent)
	{
		auto& elseToken = peekToken(info);
		if (isKeyword(elseToken, "else"))
		{
			statement->elseBody = std::make_shared<Body>();

			nextToken(info);
			parseExpectedColon(info);
			parseExpectedNewline(info);

			parseBodyEx(info, statement->elseBody);
		}
		else
		{
			unparseIndent(info);
		}
	}

	pushStatement(info, statement);
	
	return true;
}

bool parseStatementWhile(ProgGenInfo& info)
{
	auto& whileToken = peekToken(info);
	if (!isKeyword(whileToken, "while"))
		return false;
	nextToken(info);

	auto statement = std::make_shared<Statement>(whileToken.pos, Statement::Type::While_Loop);
	statement->whileConditionalBody.body = std::make_shared<Body>();

	statement->whileConditionalBody.condition = genConvertExpression(getParseExpression(info), { "bool" });

	parseExpectedColon(info);
	parseExpectedNewline(info);

	parseBodyEx(info, statement->whileConditionalBody.body);

	pushStatement(info, statement);

	return true;
}

bool parseStatementDoWhile(ProgGenInfo& info)
{
	auto& doToken = peekToken(info);
	if (!isKeyword(doToken, "do"))
		return false;
	nextToken(info);

	auto statement = std::make_shared<Statement>(doToken.pos, Statement::Type::Do_While_Loop);
	statement->doWhileConditionalBody.body = std::make_shared<Body>();

	parseExpectedColon(info);
	parseExpectedNewline(info);

	parseBodyEx(info, statement->doWhileConditionalBody.body);

	if (!parseIndent(info))
		THROW_PROG_GEN_ERROR(doToken.pos, "Expected 'while'!");
	parseExpected(info, Token::Type::Keyword, "while");
	
	statement->doWhileConditionalBody.condition = genConvertExpression(getParseExpression(info), { "bool" });

	parseExpectedNewline(info);

	pushStatement(info, statement);

	return true;
}

bool parseControlFlow(ProgGenInfo& info)
{
	if (parseStatementIf(info)) return true;
	if (parseStatementWhile(info)) return true;
	if (parseStatementDoWhile(info)) return true;

	return false;
}

void parseFunctionBody(ProgGenInfo& info)
{
	auto& bodyBeginToken = peekToken(info, -1);
	
	parseBody(info);

	if (info.program->body->statements.empty() || lastStatement(info)->type != Statement::Type::Return)
	{
		if (info.funcRetType == Datatype{ "void" })
			pushStatement(info, std::make_shared<Statement>(Token::Position(), Statement::Type::Return));
		else
			THROW_PROG_GEN_ERROR(info.program->body->statements.empty() ? bodyBeginToken.pos : lastStatement(info)->pos, "Missing return statement!");
	}
}

bool parseStatementImport(ProgGenInfo& info);

bool parseStatementDefine(ProgGenInfo& info)
{
	auto& defToken = peekToken(info);
	if (!isKeyword(defToken, "define"))
		return false;
	nextToken(info);

	auto& nameToken = peekToken(info, 0, true);
	nextToken(info);

	if (!isIdentifier(nameToken))
		THROW_PROG_GEN_ERROR(nameToken.pos, "Expected identifier!");

	auto defIt = info.defSymbols.find(nameToken.value);
	if (defIt != info.defSymbols.end())
		THROW_PROG_GEN_ERROR(nameToken.pos, "Symbol: '" + nameToken.value + "' already defined here: '" + getPosStr(defIt->second.pos) + "'!");

	auto& valToken = nextToken(info);

	if (isNewline(valToken))
		THROW_PROG_GEN_ERROR(valToken.pos, "Unexpected newline!");

	parseExpectedNewline(info);

	info.defSymbols.insert({ nameToken.value, valToken });

	return true;
}

bool parsePack(ProgGenInfo& info)
{
	auto& packToken = peekToken(info);
	if (!isKeyword(packToken, "pack"))
		return false;
	nextToken(info);

	auto& nameToken = peekToken(info);
	if (!isIdentifier(nameToken))
		THROW_PROG_GEN_ERROR(nameToken.pos, "Expected identifier!");
	nextToken(info);

	PackRef pack = std::make_shared<Pack>();
	pack->pos = nameToken.pos;
	pack->name = nameToken.value;

	pack = addPack(info, pack);
	if (isSeparator(peekToken(info), "..."))
	{
		nextToken(info);
		parseExpectedNewline(info);
		return true;
	}
	
	parseExpectedColon(info);
	parseExpectedNewline(info);

	increaseIndent(info.indent);
	pushContext(info, ProgGenInfo::Context::Pack);
	info.currPack = pack;

	while (parseIndent(info))
	{
		if (!parseDeclDef(info, false))
			THROW_PROG_GEN_ERROR(peekToken(info).pos, "Expected member definition!");
	}

	popContext(info);
	decreaseIndent(info.indent);

	pack->isDefined = true;

	return true;
}

void parseGlobalCode(ProgGenInfo& info)
{
	while (info.currToken < info.tokens->size())
	{
		if (!parseIndent(info))
			THROW_PROG_GEN_ERROR(peekToken(info).pos, "Expected indent!");
		auto token = (*info.tokens)[info.currToken];
		if (parseEmptyLine(info)) continue;
		if (parseStatementPass(info)) continue;
		if (parseStatementImport(info)) continue;
		if (parseStatementDefine(info)) continue;
		if (parseControlFlow(info)) continue;
		if (parseStatementContinue(info)) continue;
		if (parseStatementBreak(info)) continue;
		if (parsePack(info)) continue;
		if (parseDeclDef(info, true)) continue;
		if (parseSinglelineAssembly(info)) continue;
		if (parseMultilineAssembly(info)) continue;
		if (parseExpression(info)) continue;
		THROW_PROG_GEN_ERROR(token.pos, "Unexpected token: " + token.value + "!");
	}
}

bool parseStatementImport(ProgGenInfo& info)
{
	auto& importToken = peekToken(info);
	if (!isKeyword(importToken, "import"))
		return false;

	nextToken(info);
	auto& fileToken = nextToken(info);
	if (!isString(fileToken))
		THROW_PROG_GEN_ERROR(fileToken.pos, "Expected file path!");

	std::string path;
	for (auto& dir : info.importDirs)
	{
		path = (std::filesystem::path(dir) / fileToken.value).string();
		std::string absPath;
		try
		{
			absPath = std::filesystem::canonical(path);
		}
		catch (std::filesystem::filesystem_error&)
		{
			path = "";
			continue;
		}

		if (info.imports.find(absPath) != info.imports.end())
			return true;

		if (std::filesystem::is_regular_file(path))
		{
			info.imports.insert(absPath);
			break;
		}
		path = "";
	}

	if (path.empty())
		THROW_PROG_GEN_ERROR(fileToken.pos, "Import file not found: '" + fileToken.value + "'!");

	std::string code = readTextFile(path);

	auto backup = makeProgGenInfoBackup(info);

	info.tokens = tokenize(code, path);
	info.currToken = 0;
	parseGlobalCode(info);

	loadProgGenInfoBackup(info, backup);

	return true;
}

void markReachableFunctions(ProgGenInfo& info, BodyRef body)
{
	for (auto& funcInfo : body->usedFunctions)
	{
		auto func = info.program->functions.find(funcInfo.first)->second.find(funcInfo.second)->second;
		if (func->isReachable)
			continue;

		func->isReachable = true;
		markReachableFunctions(info, func->body);
	}
}

void detectUndefinedFunctions(const std::map<std::string, FunctionOverloads>& functions)
{
	for (auto& overloads : functions)
		for (auto& func : overloads.second)
			if (!func.second->isDefined && func.second->isReachable)
				THROW_PROG_GEN_ERROR(func.second->pos, "Function: '" + func.second->name + "' is referenced but undefined!");
}

ProgramRef generateProgram(const TokenListRef tokens, const std::set<std::string>& importDirs)
{
	ProgGenInfo info = { tokens, ProgramRef(new Program()), importDirs };
	info.program->body = std::make_shared<Body>();
	info.contextStack.push(ProgGenInfo::Context::Global);

	parseGlobalCode(info);

	markReachableFunctions(info, info.program->body);

	detectUndefinedFunctions(info.program->functions);

	return info.program;
}