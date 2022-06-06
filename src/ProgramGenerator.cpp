#include "ProgramGenerator.h"

#include <map>
#include <set>
#include <stack>
#include <algorithm>
#include <filesystem>
#include <cassert>

#include "Warning.h"
#include "Errors/ProgGenError.h"

#include "Tokenizer.h"
#include "OperatorPrecedence.h"

#define ENABLE_EXPR_ONLY_FOR_OBJ(expr) if (!expr->isObject) THROW_PROG_GEN_ERROR(expr->pos, "Expected object!")
#define ENABLE_EXPR_ONLY_FOR_NON_OBJ(expr) if (expr->isObject) THROW_PROG_GEN_ERROR(expr->pos, "Expected non-object!")
#define ENABLE_EXPR_ONLY_FOR_NON_CONST(expr) if (expr->datatype.isConst) THROW_PROG_GEN_ERROR(expr->pos, "Expected non-constant!")

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
	std::string progPath;
	TokenList::iterator currToken;
	struct Indent
	{
		int lvl = 0;
		int nPerLvl = 0;
		std::string chStr = "";
	} indent;

	std::stack<BodyRef> mainBodyBackups;
	int funcRetOffset;
	int funcFrameSize;
	Datatype funcRetType;

	std::set<std::string> imports;

	std::map<std::string, Token> defSymbols;

	std::vector<Token> deferredImports;

	int continueEnableCount = 0;
	int breakEnableCount = 0;
};

struct ProgGenInfoBackup
{
	TokenListRef tokens;
	TokenList::iterator currToken;
	ProgGenInfo::Indent indent;
	std::string progPath;
};

ProgGenInfoBackup makeProgGenInfoBackup(const ProgGenInfo& info)
{
	ProgGenInfoBackup backup;
	backup.tokens = info.tokens;
	backup.currToken = info.currToken;
	backup.indent = info.indent;
	backup.progPath = info.progPath;
	return backup;
}

void loadProgGenInfoBackup(ProgGenInfo& info, const ProgGenInfoBackup& backup)
{
	info.tokens = backup.tokens;
	info.currToken = backup.currToken;
	info.indent = backup.indent;
	info.progPath = backup.progPath;
}

SymbolRef currSym(const ProgGenInfo& info)
{
	return currSym(info.program);
}

void enterSymbol(ProgGenInfo& info, SymbolRef symbol)
{
	assert(symbol);
	info.program->symStack.push(symbol);
}

void exitSymbol(ProgGenInfo& info)
{
	assert(!info.program->symStack.empty());
	info.program->symStack.pop();
}

SymbolRef addShadowSpace(ProgGenInfo& info)
{
	static int shadowId = 0;
	std::string shadowName = "<" + std::to_string(shadowId++) + ">";
	auto symbol = std::make_shared<Symbol>();
	symbol->type = SymType::Namespace;
	symbol->name = shadowName;
	
	addSymbol(currSym(info), symbol);

	symbol->frame.totalOffset = getParent(symbol)->frame.totalOffset;

	return symbol;
}

TokenList::iterator moveTokenIterator(TokenList& list, TokenList::iterator it, int offset)
{
	if (offset > 0)
	{
		while (offset-- > 0)
		{
			++it;
			if (it == list.end())
				it = list.insert(it, list.back());
		}
	}
	else if (offset < 0)
	{
		while (offset++ < 0)
		{
			--it;
			if (it == list.begin())
				return it;
		}
	}
	
	return it;
}

void autoResizeTokenList(ProgGenInfo& info, int offset)
{
	if (offset >= 0 && std::distance(info.currToken, info.tokens->end()) <= offset)
	{
		int dist = offset - std::distance(info.currToken, info.tokens->end()) + 1;
		while (dist-- > 0)
			info.tokens->push_back(info.tokens->back());
	}
}

const Token& peekToken(ProgGenInfo& info, int offset = 0, bool ignoreSymDef = false)
{
	auto tokIt = moveTokenIterator(*info.tokens, info.currToken, offset);

	auto pTok = &*tokIt;

	if (ignoreSymDef)
		return *pTok;

	while (isIdentifier(*pTok))
	{
		auto sym = getSymbol(currSym(info), pTok->value);
		if (!sym || !isMacro(sym))
			break;

		bool updateCurrToken = (tokIt == info.currToken);

		tokIt = info.tokens->erase(tokIt);
		tokIt = info.tokens->insert(tokIt, sym->macroTokens.begin(), sym->macroTokens.end());
		pTok = &*tokIt;

		if (updateCurrToken)
			info.currToken = tokIt;
	}

	return *pTok;
}

const Token& nextToken(ProgGenInfo& info, int offset = 1, bool ignoreSymDef = false)
{
	auto& temp = peekToken(info, offset - 1, ignoreSymDef);

	info.currToken = moveTokenIterator(*info.tokens, info.currToken, offset);

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

void mergeSubUsages(BodyRef parent, BodyRef child)
{
	parent->usedFunctions.merge(child->usedFunctions);
	child->usedFunctions.clear();
}

void pushStatement(ProgGenInfo& info, StatementRef statement)
{
	info.program->body->statements.push_back(statement);
}

ExpressionRef makeSymbolExpression(Token::Position pos, SymbolRef symbol)
{
	ExpressionRef exp = std::make_shared<Expression>(pos);
	exp->eType = Expression::ExprType::Symbol;
	exp->symbol = symbol;
	exp->isLValue = true;
	exp->isObject = true;
	exp->datatype = symbol->var.datatype;

	return exp;
}

ExpressionRef makeLiteralExpression(Token::Position pos, const Datatype& datatype, EValue value)
{
	ExpressionRef exp = std::make_shared<Expression>(pos);
	exp->eType = Expression::ExprType::Literal;
	exp->isLValue = false;
	exp->isObject = true;
	exp->datatype = datatype;
	exp->value = value;

	return exp;
}

void addVariable(ProgGenInfo& info, SymbolRef sym);

void pushStaticLocalInit(ProgGenInfo& info, ExpressionRef initExpr)
{
	auto ifStatement = std::make_shared<Statement>(initExpr->pos, Statement::Type::If_Clause);

	ifStatement->ifConditionalBodies.push_back(ConditionalBody());
	auto& condBody = ifStatement->ifConditionalBodies.back();

	auto statSym = std::make_shared<Symbol>();
	statSym->pos = initExpr->pos;
	statSym->name = getStaticLocalInitName(info.program->staticLocalInitCount++);
	statSym->type = SymType::Variable;
	statSym->var.datatype = { "bool" };
	statSym->var.context = SymVarContext::Global;

	addVariable(info, statSym);

	condBody.condition = makeSymbolExpression(initExpr->pos, statSym);

	auto assignExpr = std::make_shared<Expression>(initExpr->pos);
	assignExpr->eType = Expression::ExprType::Assign;
	assignExpr->isObject = true;
	assignExpr->datatype = { "bool" };
	assignExpr->left = makeSymbolExpression(initExpr->pos, statSym);
	assignExpr->right = makeLiteralExpression(initExpr->pos, { "bool" }, { (uint64_t)0 });

	condBody.body = std::make_shared<Body>();
	condBody.body->statements.push_back(assignExpr);
	condBody.body->statements.push_back(initExpr);

	pushStatement(info, ifStatement);
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

ExpressionRef genConvertExpression(ProgGenInfo& info, ExpressionRef expToConvert, const Datatype& newDatatype, bool isExplicit = false, bool doThrow = true);

SymbolRef getMatchingOverload(ProgGenInfo& info, const SymbolRef overloads, std::vector<ExpressionRef>& paramExpr)
{
	SymbolRef match = nullptr;

	for (auto overload : overloads->subSymbols)
	{
		auto func = overload.second;

		if (func->func.params.size() != paramExpr.size())
			continue;

		bool matchFound = true;
		bool isExactMatch = true;
		for (int i = 0; i < paramExpr.size() && matchFound; ++i)
		{
			if (!dtEqual(paramExpr[i]->datatype, func->func.params[i]->var.datatype))
				isExactMatch = false;
			matchFound = !!genConvertExpression(info, paramExpr[i], func->func.params[i]->var.datatype, false, false);
		}

		if (isExactMatch)
			return func;
		
		if (matchFound)
		{
			if (match)
				THROW_PROG_GEN_ERROR(peekToken(info).pos, "Multiple overloads found for function call!");
			match = func;
		}
	}

	if (match)
	{
		for (int i = 0; i < paramExpr.size(); ++i)
			paramExpr[i] = genConvertExpression(info, paramExpr[i], match->func.params[i]->var.datatype, false);
	}

	return match;
}

SymbolRef getVariable(ProgGenInfo& info, const std::string& name)
{
	auto symbol = getSymbol(currSym(info), name);
	if (!isVariable(symbol))
		return nullptr;

	return symbol;
}

SymbolRef getFunctions(ProgGenInfo& info, const std::string& name)
{
	auto symbol = getSymbol(currSym(info), name);
	if (!isFuncName(symbol))
		return nullptr;

	return symbol;
}

SymbolRef getFunctions(ProgGenInfo& info, const std::vector<std::string>& path)
{
	auto symbol = getSymbolFromPath(info.program->symbols, path);
	if (!isFuncName(symbol))
		return nullptr;
	
	return symbol;
}

SymbolRef getEnum(ProgGenInfo& info, const std::string& name)
{
	auto symbol = getSymbol(currSym(info), name);
	if (!isEnum(symbol))
		return nullptr;

	return symbol;
}

#define ERR_ENUM_NAME_NOT_FOUND -1
#define ERR_ENUM_VALUE_NOT_FOUND -2
unsigned int getEnumValue(ProgGenInfo& info, const std::string& enumName, const std::string& memberName)
{
	auto enumSymbol = getEnum(info, enumName);
	if (!enumSymbol)
		return ERR_ENUM_NAME_NOT_FOUND;

	auto memberSymbol = getSymbol(enumSymbol, memberName, true);
	if (!memberSymbol)
		return ERR_ENUM_VALUE_NOT_FOUND;
	
	return memberSymbol->enumValue;
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
		THROW_PROG_GEN_ERROR(token.pos, "Expected token of type '" + TokenTypeToString(type) + "', but found '" + TokenTypeToString(token.type) + "'!");
}

void parseExpected(ProgGenInfo& info, Token::Type type, const std::string& value)
{
	auto& token = nextToken(info);
	if (token.type != type || token.value != value)
		THROW_PROG_GEN_ERROR(token.pos, "Expected '" + value + "', but found '" + token.value + "'!");
}

void parseExpectedNewline(ProgGenInfo& info)
{
	auto& token = nextToken(info);
	if (!isNewline(token))
		THROW_PROG_GEN_ERROR(token.pos, "Expected newline!");
}

bool parseOptionalNewline(ProgGenInfo& info)
{
	bool newlineFound = false;
	auto& token = peekToken(info);
	if ((newlineFound = (isNewline(token) || isEndOfCode(token))))
		nextToken(info);
	return newlineFound;
}

void parseExpectedColon(ProgGenInfo& info)
{
	parseExpected(info, Token::Type::Separator, ":");
}

void addVariable(ProgGenInfo& info, SymbolRef sym)
{
	auto& var = sym->var;
	var.modName = sym->name;

	if (isInPack(currSym(info)))
	{
		if (getParent(currSym(info), SymType::Pack)->pack.isUnion)
		{
			var.offset = 0;
			currSym(info)->pack.size = std::max(currSym(info)->pack.size, getDatatypeSize(info.program, var.datatype));
		}
		else
		{
			var.offset = currSym(info)->pack.size;
			currSym(info)->pack.size += getDatatypeSize(info.program, var.datatype);
		}

		addSymbol(currSym(info), sym);
		return;
	}

	int varSize = getDatatypeSize(info.program, var.datatype);

	auto symbol = getSymbol(currSym(info), sym->name, true);
	if (symbol)
		THROW_PROG_GEN_ERROR(sym->pos, "Symbol '" + sym->name + "' already exists in the same scope!");

	static int staticID = 0;
	if (isVarStatic(sym))
		var.modName = "__#static_" + std::to_string(staticID++) + "~" + sym->name;
	static int globVarID = 0;
	if (isInGlobal(currSym(info)))
		var.modName.append("_#" + std::to_string(globVarID++));

	if (isVarLocal(sym) || isVarStatic(sym))
	{
		varSize = getDatatypeSize(info.program, var.datatype);

		currSym(info)->frame.size += varSize;
		currSym(info)->frame.totalOffset -= varSize;
		var.offset = currSym(info)->frame.totalOffset;
		if (-currSym(info)->frame.totalOffset > info.funcFrameSize)
			info.funcFrameSize = -currSym(info)->frame.totalOffset;
	}

	addSymbol(currSym(info), sym);
}

SymbolRef addFunction(ProgGenInfo& info, SymbolRef func)
{
	auto funcs = getSymbol(currSym(info), func->name, true);
	if (funcs && !isFuncName(funcs))
		THROW_PROG_GEN_ERROR(func->pos, "Symbol '" + func->name + "' already exists in the same scope!");
	
	if (!funcs)
	{
		funcs = std::make_shared<Symbol>();
		funcs->name = func->name;
		funcs->type = SymType::FunctionName;

		addSymbol(currSym(info), funcs);
	}

	func->name = getSignatureNoRet(func);

	auto existingOverload = getSymbol(funcs, func->name, true);

	if (!existingOverload)
	{
		addSymbol(funcs, func);
		return func;
	}
	
	if (!dtEqual(existingOverload->func.retType, func->func.retType))
		THROW_PROG_GEN_ERROR(func->pos, "Function '" + getMangledName(existingOverload) + "' already exists with different return type!");
	if (!isDefined(func))
		return existingOverload;
	if (isDefined(existingOverload))
		THROW_PROG_GEN_ERROR(func->pos, "Function '" + getMangledName(existingOverload) + "' already defined here: " + getPosStr(existingOverload->pos));

	return replaceSymbol(existingOverload, func);
}

SymbolRef addPack(ProgGenInfo& info, SymbolRef pack)
{
	auto existingPack = getSymbol(currSym(info), pack->name, true);
	
	if (!existingPack)
	{
		addSymbol(currSym(info), pack);
		return pack;
	}

	if (!isPack(existingPack) || existingPack->pack.isUnion != pack->pack.isUnion)
		THROW_PROG_GEN_ERROR(pack->pos, "Pack '" + pack->name + "' already exists in the same scope!");

	if (!isDefined(pack))
		return existingPack;
	if (isDefined(existingPack))
		THROW_PROG_GEN_ERROR(pack->pos, "Pack already defined here: " + getPosStr(existingPack->pos));

	return replaceSymbol(existingPack, pack);
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

			auto pVar = getVariable(info, varName);
			if (!pVar)
				THROW_PROG_GEN_ERROR(asmToken.pos, "Unknown variable '" + varName + "'!");
				
			if (isVarOffset(pVar))
			{
				int offset = pVar->var.offset;
				result += (offset < 0) ? "- " : "+ ";
				result += std::to_string(std::abs(offset));
			}
			else if (isMacro(pVar))
			{
				THROW_PROG_GEN_ERROR(asmToken.pos, "Macros cannot be used in inline assembly!");
			}
			else
			{
				result += getMangledName(pVar);
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
	if (isNull(left))
		return right;
	if (isNull(right))
		return left;

	if (dtEqualNoConst(left, right))
	{
		if (preservesConstness(left, right))
			return right;
		if (preservesConstness(right, left))
			return left;
	}

	if (isDereferenceable(left) || isDereferenceable(right))
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
	ENABLE_EXPR_ONLY_FOR_OBJ(expToConvert);
	auto exp = std::make_shared<Expression>(expToConvert->pos);
	exp->eType = Expression::ExprType::Conversion;
	exp->isLValue = false;
	exp->isObject = true;
	exp->datatype = newDatatype;
	exp->left = expToConvert;
	return exp;
}

ExpressionRef genAutoArrayToPtr(ExpressionRef expToConvert)
{
	if (!isArray(expToConvert->datatype))
		return expToConvert;

	auto newDt = expToConvert->datatype;
	newDt.type = DTType::Pointer;
	return makeConvertExpression(expToConvert, newDt);
}

bool isConvPossible(ProgGenInfo& info, const Datatype& oldDt, const Datatype& newDt, bool isExplicit)
{
	if (dtEqualNoConst(oldDt, newDt) && preservesConstness(oldDt, newDt))
		return true;

	if (isBool(newDt))
	{
		if (isBool(oldDt))
			return true;
		if (isInteger(oldDt))
			return true;
		if (isPointer(oldDt))
			return true;
		if (isEnum(info.program, oldDt) && isExplicit)
			return true;
		return false;
	}
	if (isInteger(newDt))
	{
		if (isBool(oldDt))
			return true;
		if (isInteger(oldDt))
			return true;
		if (isPointer(oldDt) && isExplicit)
			return true;
		if (isPointer(oldDt) && dtEqual(newDt, Datatype("u64")))
			return true;
		if (isEnum(info.program, oldDt) && isExplicit)
			return true;
		return false;
	}
	if (isPointer(newDt))
	{
		if (isBool(oldDt) && isExplicit)
			return true;
		if (isInteger(oldDt) && isExplicit)
			return true;
		if (isPointer(oldDt) && isExplicit)
			return true;
		if (isPointer(oldDt) && isVoidPtr(newDt) && preservesConstness(oldDt, newDt))
			return true;
		if (isEnum(info.program, oldDt) && isExplicit)
			return true;
		return false;
	}
	if (isEnum(info.program, newDt))
	{
		if (isBool(oldDt) && isExplicit)
			return true;
		if (isInteger(oldDt) && isExplicit)
			return true;
		if (isPointer(oldDt) && isExplicit)
			return true;
		if (isEnum(info.program, oldDt) && isExplicit)
			return true;
		return false;
	}

	return false;
}

ExpressionRef genConvertExpression(ProgGenInfo& info, ExpressionRef expToConvert, const Datatype& newDatatype, bool isExplicit, bool doThrow)
{
	expToConvert = genAutoArrayToPtr(expToConvert);

	if (dtEqual(expToConvert->datatype, newDatatype))
		return expToConvert;

	if (isNull(expToConvert->datatype))
		return makeConvertExpression(expToConvert, newDatatype);

	if (isConvPossible(info, expToConvert->datatype, newDatatype, isExplicit))
	{
		if (
			isPointer(expToConvert->datatype) &&
			isPointer(newDatatype) &&
			!preservesConstness(expToConvert->datatype, newDatatype)
			)
			PRINT_WARNING(
				MAKE_PROG_GEN_ERROR(
					expToConvert->pos,
					"Conversion from '" + getDatatypeStr(expToConvert->datatype) + "' to '" + getDatatypeStr(newDatatype) + "' does not preserve the constness of the old type!"
				)
			);
		return makeConvertExpression(expToConvert, newDatatype);
	}

	if (doThrow)
		THROW_PROG_GEN_ERROR(expToConvert->pos, std::string("Cannot ") + (isExplicit ? "" : "implicitly ") + "convert from '" + getDatatypeStr(expToConvert->datatype) + "' to '" + getDatatypeStr(newDatatype) + "'!");

	return nullptr;
}

void autoFixDatatypeMismatch(ProgGenInfo& info, ExpressionRef exp)
{
	if (dtEqual(exp->left->datatype, exp->right->datatype))
		return;

	if (isPointer(exp->left->datatype) || isArray(exp->left->datatype))
	{
		switch (exp->eType)
		{
		case Expression::ExprType::Sum:
		case Expression::ExprType::Difference:
		case Expression::ExprType::Assign_Sum:
		case Expression::ExprType::Assign_Difference:
			exp->right = genConvertExpression(info, exp->right, { "u64" });
			{
				auto temp = std::make_shared<Expression>(exp->pos);
				temp->eType = Expression::ExprType::Product;
				temp->right = exp->right;
				temp->datatype = { "u64" };

				temp->left = makeLiteralExpression(exp->pos, { "u64" }, { (uint64_t)getDatatypePointedToSize(info.program, exp->left->datatype) });

				exp->right = temp;
			}
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
	
	exp->left = genConvertExpression(info, exp->left, newDatatype);
	exp->right = genConvertExpression(info, exp->right, newDatatype);
}

ExpressionRef getParseExpression(ProgGenInfo& info, int precLvl = 0);

ExpressionRef getParseEnumMember(ProgGenInfo& info)
{
	auto& enumToken = peekToken(info);
	if (!isIdentifier(enumToken))
		return nullptr;

	if (!getEnum(info, enumToken.value))
		return nullptr;

	nextToken(info);
	parseExpected(info, Token::Type::Operator, "::");

	auto& memberToken = nextToken(info);

	if (!isIdentifier(memberToken))
		THROW_PROG_GEN_ERROR(memberToken.pos, "Expected enum member identifier!");

	unsigned int value = getEnumValue(info, enumToken.value, memberToken.value);

	if (value == ERR_ENUM_NAME_NOT_FOUND)
		THROW_PROG_GEN_ERROR(enumToken.pos, "Enum '" + enumToken.value + "' not found!");
	if (value == ERR_ENUM_VALUE_NOT_FOUND)
		THROW_PROG_GEN_ERROR(memberToken.pos, "Enum member '" + memberToken.value + "' not found in enum '" + enumToken.value + "'!");

	return makeLiteralExpression(memberToken.pos, getMangledName(getEnum(info, enumToken.value)), EValue((uint64_t)value));
}

ExpressionRef getParseLiteral(ProgGenInfo& info)
{
	auto& litToken = peekToken(info);
	if (!isLiteral(litToken))
		return getParseEnumMember(info);

	nextToken(info);
	ExpressionRef exp = std::make_shared<Expression>(litToken.pos);

	exp->eType = Expression::ExprType::Literal;
	exp->isObject = true;

	switch (litToken.type)
	{
	case Token::Type::LiteralInteger:
		exp->datatype = { "u64" };
		exp->value = EValue((uint64_t)std::stoull(litToken.value));
		break;
	case Token::Type::LiteralChar:
		exp->datatype = { "u8" };
		exp->value = EValue((uint64_t)litToken.value[0]);
		break;
	case Token::Type::LiteralBoolean:
		exp->datatype = { "bool" };
		exp->value = EValue((uint64_t)(litToken.value == "true" ? 1 : 0));
		break;
	case Token::Type::LiteralNull:
		exp->datatype = { "null" };
		exp->value = EValue((uint64_t)0);
		break;
	case Token::Type::String:
	{
		int strID = info.program->strings.size();
		//exp->valStr = getLiteralStringName(strID);
		exp->value = EValue((uint64_t)strID);
		info.program->strings.insert({ strID, { 1, litToken.value } });
		exp->datatype = Datatype(DTType::Array, Datatype("u8"), litToken.value.size() + 1);
		exp->datatype.subType->isConst = true;
		if (isInFunction(currSym(info))) // Makes it possible to strip unused strings from the assembly code
			getParent(currSym(info), Symbol::Type::FunctionSpec)->func.instantiatedStrings.insert(strID);
	}
		break;
	default:
		THROW_PROG_GEN_ERROR(litToken.pos, "Invalid literal type!");
	}

	return exp;
}

ExpressionRef getParseSymbol(ProgGenInfo& info, bool localOnly)
{
	auto& symToken = peekToken(info);
	if (!isIdentifier(symToken))
		return nullptr;
	
	auto sym = getSymbol(currSym(info), symToken.value, localOnly);
	if (!sym)
		return nullptr;

	ExpressionRef exp = std::make_shared<Expression>(symToken.pos);

	exp->eType = Expression::ExprType::Symbol;
	exp->symbol = sym;

	if (isVariable(sym))
	{
		exp->datatype = sym->var.datatype;
		exp->isLValue = isArray(exp->datatype) ? false : true;
	}
	else if (isFuncName(sym))
	{
		bool x = true;
	}

	switch (sym->type)
	{
	case SymType::None:
	case SymType::Enum:
	case SymType::FunctionName:
	case SymType::Global:
	case SymType::Namespace:
	case SymType::ExtFunc:
	case SymType::Pack:
	case SymType::Macro:
		exp->isObject = false;
		break;
	case SymType::FunctionSpec:
	case SymType::EnumMember:
		exp->isObject = true;
		break;
	case SymType::Variable:
		switch (sym->var.context)
		{
		case SymVarContext::None:
		case SymVarContext::PackMember:
			exp->isObject = false;
			break;
		case SymVarContext::Local:
		case SymVarContext::Global:
		case SymVarContext::Static:
		case SymVarContext::Parameter:
			exp->isObject = true;
			break;
		default:
			assert(false && "Unknown SymVarContext!");
		}
		break;
	default:
		assert(false && "Unknown SymType!");
	}

	nextToken(info);

	return exp;
}

ExpressionRef getParseValue(ProgGenInfo& info, bool localOnly)
{
	ExpressionRef exp = nullptr;

	exp = getParseLiteral(info);
	if (exp) return exp;

	exp = getParseSymbol(info, localOnly);
	if (exp) return exp;

	THROW_PROG_GEN_ERROR(peekToken(info).pos, "Expected symbol name or literal value!");
}

ExpressionRef getParseParenthesized(ProgGenInfo& info)
{
	auto& parenOpen = peekToken(info);
	if (!isSeparator(parenOpen, "("))
		return getParseValue(info, false);
	
	nextToken(info);
	auto exp = getParseExpression(info);

	auto& parenClose = nextToken(info);
	if (!isSeparator(parenClose, ")"))
		THROW_PROG_GEN_ERROR(parenClose.pos, "Expected ')'!");

	return exp;
}

ExpressionRef autoSimplifyExpression(ExpressionRef expr)
{
	if (expr->eType == Expression::ExprType::Literal)
		return expr;

	bool leftLiteral = false;
	bool rightLiteral = false;
	bool farRightLiteral = false;

	if (expr->left)
	{
		expr->left = autoSimplifyExpression(expr->left);
		if (expr->left->eType == Expression::ExprType::Literal)
			leftLiteral = true;
	}
	if (expr->right)
	{
		expr->right = autoSimplifyExpression(expr->right);
		if (expr->right->eType == Expression::ExprType::Literal)
			rightLiteral = true;
	}
	if (expr->farRight)
	{
		expr->farRight = autoSimplifyExpression(expr->farRight);
		if (expr->farRight->eType == Expression::ExprType::Literal)
			farRightLiteral = true;
	}

	// TODO: Handle floating point expressions
	// TODO: Handle signed integer expressions
	// TODO: Masking

	switch (expr->eType)
	{
	case Expression::ExprType::Conversion:
		break;
	case Expression::ExprType::Logical_OR:
		if (leftLiteral && rightLiteral)
			expr = makeLiteralExpression(expr->pos, { "bool" }, EValue(uint64_t(expr->left->value.u64 || expr->right->value.u64)));
		break;
	case Expression::ExprType::Logical_AND:
		if (leftLiteral && rightLiteral)
			expr = makeLiteralExpression(expr->pos, { "bool" }, EValue(uint64_t(expr->left->value.u64 && expr->right->value.u64)));
		break;
	case Expression::ExprType::Bitwise_OR:
		if (leftLiteral && rightLiteral)
			expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(expr->left->value.u64 | expr->right->value.u64)));
		break;
	case Expression::ExprType::Bitwise_XOR:
		if (leftLiteral && rightLiteral)
			expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(expr->left->value.u64 ^ expr->right->value.u64)));
		break;
	case Expression::ExprType::Bitwise_AND:
		if (leftLiteral && rightLiteral)
			expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(expr->left->value.u64 & expr->right->value.u64)));
		break;
	case Expression::ExprType::Comparison_Equal:
		if (leftLiteral && rightLiteral)
			expr = makeLiteralExpression(expr->pos, { "bool" }, EValue(uint64_t(expr->left->value.u64 == expr->right->value.u64)));
		break;
	case Expression::ExprType::Comparison_NotEqual:
		if (leftLiteral && rightLiteral)
			expr = makeLiteralExpression(expr->pos, { "bool" }, EValue(uint64_t(expr->left->value.u64 != expr->right->value.u64)));
		break;
	case Expression::ExprType::Comparison_Less:
		if (leftLiteral && rightLiteral)
			expr = makeLiteralExpression(expr->pos, { "bool" }, EValue(uint64_t(expr->left->value.u64 < expr->right->value.u64)));
		break;
	case Expression::ExprType::Comparison_LessEqual:
		if (leftLiteral && rightLiteral)
			expr = makeLiteralExpression(expr->pos, { "bool" }, EValue(uint64_t(expr->left->value.u64 <= expr->right->value.u64)));
		break;
	case Expression::ExprType::Comparison_Greater:
		if (leftLiteral && rightLiteral)
			expr = makeLiteralExpression(expr->pos, { "bool" }, EValue(uint64_t(expr->left->value.u64 > expr->right->value.u64)));
		break;
	case Expression::ExprType::Comparison_GreaterEqual:
		if (leftLiteral && rightLiteral)
			expr = makeLiteralExpression(expr->pos, { "bool" }, EValue(uint64_t(expr->left->value.u64 >= expr->right->value.u64)));
		break;
	case Expression::ExprType::Shift_Left:
		if (leftLiteral && rightLiteral)
			expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(expr->left->value.u64 << expr->right->value.u64)));
		break;
	case Expression::ExprType::Shift_Right:
		if (leftLiteral && rightLiteral)
			expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(expr->left->value.u64 >> expr->right->value.u64)));
		break;
	case Expression::ExprType::Sum:
		if (leftLiteral && rightLiteral)
			expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(expr->left->value.u64 + expr->right->value.u64)));
		break;
	case Expression::ExprType::Difference:
		if (leftLiteral && rightLiteral)
			expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(expr->left->value.u64 - expr->right->value.u64)));
		break;
	case Expression::ExprType::Product:
		if (leftLiteral && rightLiteral)
			expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(expr->left->value.u64 * expr->right->value.u64)));
		break;
	case Expression::ExprType::Quotient:
		if (leftLiteral && rightLiteral)
			expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(expr->left->value.u64 / expr->right->value.u64)));
		break;
	case Expression::ExprType::Remainder:
		if (leftLiteral && rightLiteral)
			expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(expr->left->value.u64 % expr->right->value.u64)));
		break;
	case Expression::ExprType::Logical_NOT:
		if (leftLiteral)
			expr = makeLiteralExpression(expr->pos, { "bool" }, EValue(uint64_t(!expr->left->value.u64)));
		break;
	case Expression::ExprType::Bitwise_NOT:
		if (leftLiteral)
			expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(~expr->left->value.u64)));
		break;
	case Expression::ExprType::Prefix_Plus:
		break; // Nothing to do
	case Expression::ExprType::Prefix_Minus:
		if (leftLiteral)
			expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(-expr->left->value.u64)));
		break;
	case Expression::ExprType::Conditional_Op:
		if (leftLiteral)
			expr = expr->left->value.u64 ? expr->left : expr->right;
		break;
	}
	return expr;
}

ExpressionRef getParseBinaryExpression(ProgGenInfo& info, int precLvl)
{
	auto& opsLvl = opPrecLvls[precLvl];

	ExpressionRef baseExpr = nullptr;
	auto currExpr = getParseExpression(info, precLvl + 1);

	const Token* pOpToken = nullptr;
	std::map<std::string, Expression::ExprType>::iterator it;
	while ((it = opsLvl.ops.find((pOpToken = &peekToken(info))->value)) != opsLvl.ops.end() && isSepOpKey(*pOpToken))
	{
		switch (opsLvl.evalOrder)
		{
		case OpPrecLvl::EvalOrder::LeftToRight:
		{
			auto temp = std::make_shared<Expression>(pOpToken->pos);
			temp->left = currExpr;
			currExpr = temp;
		}
			break;
		case OpPrecLvl::EvalOrder::RightToLeft:
		{
			if (!baseExpr)
			{
				auto temp = std::make_shared<Expression>(pOpToken->pos);
				temp->left = currExpr;
				currExpr = temp;
				baseExpr = currExpr;
			}
			else
			{
				auto temp = std::make_shared<Expression>(pOpToken->pos);
				temp->left = currExpr->right;
				currExpr->right = temp;
				currExpr = temp;
			}
		}
			break;
		default:
			assert(false && "Unknown eval order!");
		}

		currExpr->eType = it->second;
		nextToken(info);

		switch (currExpr->eType)
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
			currExpr->right = getParseExpression(info, precLvl + 1);
			ENABLE_EXPR_ONLY_FOR_NON_CONST(currExpr->left);
			ENABLE_EXPR_ONLY_FOR_OBJ(currExpr->left);
			ENABLE_EXPR_ONLY_FOR_OBJ(currExpr->right);
			autoFixDatatypeMismatch(info, currExpr);
			if (!currExpr->left->isLValue)
				THROW_PROG_GEN_ERROR(currExpr->left->pos, "Cannot assign to non-lvalue!");
			currExpr->datatype = currExpr->left->datatype;
			currExpr->isLValue = true;
			currExpr->isObject = true;
			break;
		case Expression::ExprType::Conditional_Op:
		{
			ENABLE_EXPR_ONLY_FOR_OBJ(currExpr->left);
			currExpr->left = genConvertExpression(info, currExpr->left, { "bool" });

			currExpr->right = getParseExpression(info, 0);
			ENABLE_EXPR_ONLY_FOR_OBJ(currExpr->right);

			parseExpectedColon(info);

			currExpr->farRight = getParseExpression(info, 0);
			ENABLE_EXPR_ONLY_FOR_OBJ(currExpr->farRight);

			currExpr->right = genAutoArrayToPtr(currExpr->right);
			currExpr->farRight = genAutoArrayToPtr(currExpr->farRight);

			if (!dtEqual(currExpr->right->datatype, currExpr->farRight->datatype))
				THROW_PROG_GEN_ERROR(currExpr->pos, "Conditional operator operands must have the same datatype!");

			currExpr->datatype = currExpr->right->datatype;
			currExpr->isLValue = currExpr->right->isLValue;
			currExpr->isObject = currExpr->right->isObject;
		}
			break;
		case Expression::ExprType::Logical_OR:
		case Expression::ExprType::Logical_AND:
			currExpr->right = getParseExpression(info, precLvl + 1);
			ENABLE_EXPR_ONLY_FOR_OBJ(currExpr->left);
			ENABLE_EXPR_ONLY_FOR_OBJ(currExpr->right);
			currExpr->left = genConvertExpression(info, currExpr->left, { "bool" });
			currExpr->right = genConvertExpression(info, currExpr->right, { "bool" });
			currExpr->datatype = { "bool" };
			currExpr->isLValue = false;
			currExpr->isObject = true;
			break;
		case Expression::ExprType::Bitwise_OR:
		case Expression::ExprType::Bitwise_XOR:
		case Expression::ExprType::Bitwise_AND:
			currExpr->right = getParseExpression(info, precLvl + 1);
			ENABLE_EXPR_ONLY_FOR_OBJ(currExpr->left);
			ENABLE_EXPR_ONLY_FOR_OBJ(currExpr->right);
			autoFixDatatypeMismatch(info, currExpr);
			currExpr->datatype = currExpr->left->datatype;
			currExpr->isLValue = false;
			currExpr->isObject = true;
			break;
		case Expression::ExprType::Comparison_Equal:
		case Expression::ExprType::Comparison_NotEqual:
		case Expression::ExprType::Comparison_Less:
		case Expression::ExprType::Comparison_LessEqual:
		case Expression::ExprType::Comparison_Greater:
		case Expression::ExprType::Comparison_GreaterEqual:
			currExpr->right = getParseExpression(info, precLvl + 1);
			ENABLE_EXPR_ONLY_FOR_OBJ(currExpr->left);
			ENABLE_EXPR_ONLY_FOR_OBJ(currExpr->right);
			autoFixDatatypeMismatch(info, currExpr);
			currExpr->datatype = { "bool" };
			currExpr->isLValue = false;
			currExpr->isObject = true;
			break;
		case Expression::ExprType::Shift_Left:
		case Expression::ExprType::Shift_Right:
		case Expression::ExprType::Sum:
		case Expression::ExprType::Difference:
		case Expression::ExprType::Product:
		case Expression::ExprType::Quotient:
		case Expression::ExprType::Remainder:
			currExpr->right = getParseExpression(info, precLvl + 1);
			ENABLE_EXPR_ONLY_FOR_OBJ(currExpr->left);
			ENABLE_EXPR_ONLY_FOR_OBJ(currExpr->right);
			autoFixDatatypeMismatch(info, currExpr);
			currExpr->datatype = currExpr->left->datatype;
			currExpr->isLValue = false;
			currExpr->isObject = true;
			break;
		case Expression::ExprType::SpaceAccess:
		{
			if (currExpr->left->eType != Expression::ExprType::Symbol)
				THROW_PROG_GEN_ERROR(currExpr->left->pos, "Expected namespace name!");
			enterSymbol(info, currExpr->left->symbol);

			bool isObject = isSymType(SymType::Namespace, currExpr->left->symbol);

			currExpr = getParseValue(info, true);

			exitSymbol(info);
		}
			break;
		default:
			THROW_PROG_GEN_ERROR(currExpr->pos, "Invalid binary operator!");
		}
	}

	return baseExpr ? baseExpr : currExpr;
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
			ENABLE_EXPR_ONLY_FOR_OBJ(exp->left);
			exp->datatype = exp->left->datatype;
			if (!isDereferenceable(exp->datatype))
				THROW_PROG_GEN_ERROR(exp->pos, "Cannot subscript non-pointer type!");
			dereferenceDatatype(exp->datatype);
			if (!isDereferenceable(exp->datatype) && exp->datatype.name == "void")
				THROW_PROG_GEN_ERROR(exp->pos, "Cannot subscript pointer to void type!");
			exp->isLValue = isArray(exp->datatype) ? false : true;
			exp->isObject = true;
			exp->right = genConvertExpression(info, getParseExpression(info), { "u64" });
			ENABLE_EXPR_ONLY_FOR_OBJ(exp->right);
			parseExpected(info, Token::Type::Separator, "]");
			break;
		case Expression::ExprType::FunctionCall:
		{
			exp->isExtCall = isExtFunc(exp->left->symbol);

			if (exp->left->eType != Expression::ExprType::Symbol || (!isFuncName(exp->left->symbol) && !exp->isExtCall))
				THROW_PROG_GEN_ERROR(exp->pos, "Cannot call non-function!");
			exp->isLValue = false;

			while (!isSeparator(peekToken(info), ")"))
			{
				exp->paramExpr.push_back(getParseExpression(info));
				if (!isSeparator(peekToken(info), ")"))
					parseExpected(info, Token::Type::Separator, ",");
			}
			parseExpected(info, Token::Type::Separator, ")");

			SymbolRef func = nullptr;
			if (exp->isExtCall)
			{
				func = exp->left->symbol;
				for (int i = 0; i < exp->paramExpr.size(); ++i)
					exp->paramExpr[i] = genConvertExpression(info, exp->paramExpr[i], exp->left->symbol->func.params[i]->var.datatype);
			}
			else
			{
				func = getMatchingOverload(info, exp->left->symbol, exp->paramExpr);
				if (!func)
					THROW_PROG_GEN_ERROR(exp->pos, "No matching overload found for function '" + getMangledName(exp->left->symbol) + "'!");
			}
			
			exp->datatype = func->func.retType;
			exp->left->datatype = Datatype(DTType::Pointer, Datatype(getSignature(func)));
			exp->left->symbol = func;
			exp->isObject = !isVoid(func->func.retType);

			info.program->body->usedFunctions.insert(getSymbolPath(nullptr, func));

			exp->paramSizeSum = 0;
			for (auto& param : exp->paramExpr)
				exp->paramSizeSum += getDatatypePushSize(info.program, param->datatype);
		}
			break;
		case Expression::ExprType::Suffix_Increment:
		case Expression::ExprType::Suffix_Decrement:
			ENABLE_EXPR_ONLY_FOR_NON_CONST(exp->left);
			ENABLE_EXPR_ONLY_FOR_OBJ(exp->left);
			exp->datatype = exp->left->datatype;
			exp->isLValue = false;
			exp->isObject = true;
			break;
		case Expression::ExprType::MemberAccess:
		case Expression::ExprType::MemberAccessDereference:
		{
			ENABLE_EXPR_ONLY_FOR_OBJ(exp->left);
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
				if (!isDereferenceable(exp->left->datatype))
					THROW_PROG_GEN_ERROR(exp->pos, "Cannot dereference non-pointer!");
				exp->left = genAutoArrayToPtr(exp->left);

				auto temp = std::make_shared<Expression>(exp->pos);
				temp->eType = Expression::ExprType::Dereference;
				temp->left = exp->left;

				temp->datatype = temp->left->datatype;
				dereferenceDatatype(temp->datatype);
				if (isDereferenceable(temp->datatype))
					THROW_PROG_GEN_ERROR(exp->pos, "Cannot access member of pointer to pointer!");
				temp->isLValue = isArray(temp->datatype) ? false : true;

				exp->left = temp;
			}
				break;
			}

			auto packSym = getSymbolFromPath(info.program->symbols, SymPathFromString(exp->left->datatype.name));
			if (!packSym)
				THROW_PROG_GEN_ERROR(exp->pos, "Unknown type '" + exp->left->datatype.name + "'!");
			if (!isPack(packSym))
				THROW_PROG_GEN_ERROR(exp->left->pos, "Cannot access member of non-pack symbol!");
			if (!isDefined(packSym))
				THROW_PROG_GEN_ERROR(exp->left->pos, "Cannot access member of undefined pack type!");
			auto memberSym = getSymbol(packSym, memberToken.value, true);
			if (!memberSym)
				THROW_PROG_GEN_ERROR(exp->pos, "Unknown member '" + memberToken.value + "'!");

			exp->datatype = memberSym->var.datatype;
			exp->symbol = memberSym;
			exp->isLValue = true;
			exp->isObject = true;
		}
			break;
		default:
			assert(false && "Unknown operator!");
		}
	}

	return exp;
}

ExpressionRef getParseUnaryPrefixExpression(ProgGenInfo& info, int precLvl)
{
	auto& opsLvl = opPrecLvls[precLvl];

	auto& opToken = peekToken(info);
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
		ENABLE_EXPR_ONLY_FOR_OBJ(exp->left);
		if (!exp->left->isLValue)
			THROW_PROG_GEN_ERROR(exp->pos, "Cannot take address of non-lvalue!");
		exp->datatype = exp->left->datatype;
		exp->datatype = Datatype(DTType::Pointer, exp->datatype);
		exp->isLValue = false;
		exp->isObject = true;
		break;
	case Expression::ExprType::Dereference:
		exp->left = getParseExpression(info, precLvl);
		ENABLE_EXPR_ONLY_FOR_OBJ(exp->left);
		exp->datatype = exp->left->datatype;
		if (!isDereferenceable(exp->datatype))
			THROW_PROG_GEN_ERROR(exp->pos, "Cannot dereference non-pointer type!");
		dereferenceDatatype(exp->datatype);
		if (!isDereferenceable(exp->datatype) && exp->datatype.name == "void")
			THROW_PROG_GEN_ERROR(exp->pos, "Cannot dereference pointer to void type!");
		exp->isLValue = isArray(exp->datatype) ? false : true;
		exp->isObject = true;
		break;
	case Expression::ExprType::Logical_NOT:
		exp->left = genConvertExpression(info, getParseExpression(info, precLvl), Datatype("bool"));
		ENABLE_EXPR_ONLY_FOR_OBJ(exp->left);
		exp->datatype = { "bool" };
		exp->isLValue = false;
		exp->isObject = true;
		break;
	case Expression::ExprType::Bitwise_NOT:
		exp->left = getParseExpression(info, precLvl);
		ENABLE_EXPR_ONLY_FOR_OBJ(exp->left);
		exp->datatype = exp->left->datatype;
		exp->isLValue = false;
		exp->isObject = true;
		break;
	case Expression::ExprType::Prefix_Increment:
	case Expression::ExprType::Prefix_Decrement:
		exp->left = getParseExpression(info, precLvl);
		ENABLE_EXPR_ONLY_FOR_NON_CONST(exp->left);
		ENABLE_EXPR_ONLY_FOR_OBJ(exp->left);
		exp->datatype = exp->left->datatype;
		exp->isLValue = false;
		exp->isObject = true;
		break;
	case Expression::ExprType::Prefix_Plus:
	case Expression::ExprType::Prefix_Minus:
		exp->left = getParseExpression(info, precLvl);
		ENABLE_EXPR_ONLY_FOR_OBJ(exp->left);
		exp->datatype = exp->left->datatype;
		exp->isLValue = false;
		exp->isObject = true;
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
		exp = genConvertExpression(info, getParseExpression(info, precLvl), newDatatype, true);
		ENABLE_EXPR_ONLY_FOR_OBJ(exp);
		exp->isObject = true;
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
		exp->eType = Expression::ExprType::Literal;
		exp->isLValue = false;
		exp->isObject = true;
		exp->datatype = { "u64" };
		exp->value = EValue((uint64_t)getDatatypeSize(info.program, datatype));
	}
		break;
	case Expression::ExprType::SpaceAccess:
		enterSymbol(info, info.program->symbols);
		exp = getParseExpression(info, precLvl + 1);
		exitSymbol(info);
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

	ExpressionRef expr = nullptr;;
	switch (opPrecLvls[precLvl].type)
	{
	case OpPrecLvl::Type::Binary: expr = getParseBinaryExpression(info, precLvl); break;
	case OpPrecLvl::Type::Unary_Suffix: expr = getParseUnarySuffixExpression(info, precLvl); break;
	case OpPrecLvl::Type::Unary_Prefix: expr = getParseUnaryPrefixExpression(info, precLvl); break;
	default: THROW_PROG_GEN_ERROR(peekToken(info).pos, "Unknown operator precedence level type!");
	}

	return autoSimplifyExpression(expr);
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

	if (!dtEqual(expr->datatype, targetType))
		expr = genConvertExpression(info, expr, targetType);
	pushStatement(info, expr);

	return true;
}

Datatype getParseDatatype(ProgGenInfo& info)
{
	Datatype datatype;

	auto prevTokIt = info.currToken;

	auto exitEntered = [&](const Datatype& dt, bool resetTokIt)
	{
		if (resetTokIt)
			info.currToken = prevTokIt;
		return dt;
	};

	if (isBuiltinType(peekToken(info)))
	{
		datatype.name = nextToken(info).value;
		datatype.type = DTType::Name;
	}
	else
	{
		bool localOnly = false;
		SymbolRef sym = currSym(info);
		if (isOperator(peekToken(info), "::"))
		{
			nextToken(info);
			sym = info.program->symbols;
			localOnly = true;
		}

		const Token* pNameToken = nullptr;
		do
		{
			pNameToken = &nextToken(info);
			if (!isIdentifier(*pNameToken))
				return exitEntered({}, true);
			sym = getSymbol(sym, pNameToken->value, localOnly);
			if (!sym)
				return exitEntered({}, true);
			localOnly = true;
		} while (isOperator(nextToken(info), "::"));
		nextToken(info, -1);
		

		if (!isPack(sym) && !isEnum(sym))
			return exitEntered({}, true);

		datatype.name = SymPathToString(getSymbolPath(nullptr, sym));
		datatype.type = DTType::Name;
	}

	if (isKeyword(peekToken(info), "const"))
	{
		nextToken(info);
		datatype.isConst = true;
	}

	while (isOperator(peekToken(info), "*"))
	{
		nextToken(info);
		datatype = Datatype(DTType::Pointer, datatype);

		if (isKeyword(peekToken(info), "const"))
		{
			nextToken(info);
			datatype.isConst = true;
		}
	}

	return exitEntered(datatype, false);
}

void parseFunctionBody(ProgGenInfo& info, bool doParseIndent);

std::pair<SymbolRef, ExpressionRef> getParseDeclDefVariable(ProgGenInfo& info, const Datatype& datatype, bool isStatic, const std::string& name)
{
	SymbolRef sym = std::make_shared<Symbol>();
	sym->type = SymType::Variable;
	sym->pos = peekToken(info).pos;
	sym->name = name;
	sym->var.datatype = datatype;

	if (isStatic)
		sym->var.context = SymVarContext::Static;
	else if (isInFunction(currSym(info)))
		sym->var.context = SymVarContext::Local;
	else if (isInPack(currSym(info)))
		sym->var.context = SymVarContext::PackMember;
	else
		sym->var.context = SymVarContext::Global;

	while (isSeparator(peekToken(info), "["))
	{
		nextToken(info);
		parseExpected(info, Token::Type::LiteralInteger);
		sym->var.datatype = Datatype(DTType::Array, sym->var.datatype, std::stoull(peekToken(info, -1).value));
		parseExpected(info, Token::Type::Separator, "]");
	}

	if (isPackType(info.program, sym->var.datatype))
		if (!isDefined(getSymbol(currSym(info), sym->var.datatype.name)))
			THROW_PROG_GEN_ERROR(sym->pos, "Cannot use declared-only pack type!");

	addVariable(info, sym);

	ExpressionRef initExpr = nullptr;
	if (!isInPack(currSym(info)))
	{
		if (isOperator(peekToken(info), "="))
		{
			nextToken(info);
			initExpr = getParseExpression(info);
		}
	}
	
	parseExpectedNewline(info);

	return { sym, initExpr };
}

void parseExpectedDeclDefVariable(ProgGenInfo& info, const Datatype& datatype, bool isStatic, const std::string& name)
{
	if (isVoid(datatype))
		THROW_PROG_GEN_ERROR(peekToken(info).pos, "Cannot declare variable of type void!");

	auto [varSym, initExpr] = getParseDeclDefVariable(info, datatype, isStatic, name);
	if (initExpr)
	{
		auto assignExpr = std::make_shared<Expression>(initExpr->pos);
		assignExpr->eType = Expression::ExprType::Assign;
		assignExpr->isLValue = true;
		assignExpr->isObject = true;
		assignExpr->datatype = varSym->var.datatype;
		assignExpr->left = makeSymbolExpression(varSym->pos, varSym);
		assignExpr->right = genConvertExpression(info, initExpr, varSym->var.datatype);
		assignExpr->ignoreConstness = true;

		if (isStatic)
			pushStaticLocalInit(info, assignExpr);
		else
			pushStatement(info, assignExpr);
	}
}

void parseExpectedDeclDefFunction(ProgGenInfo& info, const Datatype& datatype, const std::string& name)
{
	SymbolRef funcSym = std::make_shared<Symbol>();

	funcSym->pos = peekToken(info).pos;
	funcSym->type = SymType::FunctionSpec;
	funcSym->name = name;
	funcSym->func.body = std::make_shared<Body>();
	funcSym->func.retType = datatype;

	parseExpected(info, Token::Type::Separator, "(");

	while (!isSeparator(peekToken(info), ")"))
	{
		SymbolRef paramSym = std::make_shared<Symbol>();
		paramSym->type = SymType::Variable;
		paramSym->pos = peekToken(info).pos;
		paramSym->parent = funcSym;
		auto& param = paramSym->var;
		param.context = SymVarContext::Parameter;

		param.datatype = getParseDatatype(info);
		if (!param.datatype)
			THROW_PROG_GEN_ERROR(peekToken(info).pos, "Expected datatype!");
		
		param.offset = funcSym->func.retOffset;
		funcSym->func.retOffset += getDatatypePushSize(info.program, param.datatype);

		if (!isIdentifier(peekToken(info)))
			THROW_PROG_GEN_ERROR(peekToken(info).pos, "Expected identifier!");

		param.modName = nextToken(info).value;
		paramSym->name = param.modName;

		funcSym->func.params.push_back(paramSym);

		if (isSeparator(peekToken(info), ")"))
			continue;

		parseExpected(info, Token::Type::Separator, ",");
	}

	parseExpected(info, Token::Type::Separator, ")");

	bool reqPreDecl = isOperator(peekToken(info), "!");
	if (reqPreDecl)
		nextToken(info);

	auto preDeclSym = getSymbol(currSym(info), name, true);
	if (preDeclSym)
		preDeclSym = getSymbol(preDeclSym, getSignatureNoRet(funcSym), true);
	if (reqPreDecl && !preDeclSym)
		THROW_PROG_GEN_ERROR(peekToken(info).pos, "Missing pre-declaration before explicit function definition!");
	else if (!reqPreDecl && preDeclSym)
		PRINT_WARNING(MAKE_PROG_GEN_ERROR(peekToken(info).pos, "Function '" + name + "' was pre-declared but not marked as such. You may want to do so."));

	funcSym->state = SymState::Defined;
	if (isSeparator(peekToken(info), "..."))
	{
		funcSym->state = SymState::Declared;

		nextToken(info);
		parseExpectedNewline(info);
	}

	funcSym = addFunction(info, funcSym);

	if (isDefined(funcSym))
	{
		parseExpectedColon(info);
		bool parsedNewline = parseOptionalNewline(info);

		increaseIndent(info.indent);
		pushTempBody(info, funcSym->func.body);
		enterSymbol(info, funcSym);

		info.funcRetOffset = funcSym->func.retOffset;
		info.funcRetType = funcSym->func.retType;
		info.funcFrameSize = 0;

		for (auto& param : funcSym->func.params)
			addSymbol(funcSym, param);

		parseFunctionBody(info, parsedNewline);
		funcSym->frame.size = info.funcFrameSize;
		
		exitSymbol(info);
		popTempBody(info);
		decreaseIndent(info.indent);
	}
}

bool parseDeclDef(ProgGenInfo& info)
{
	auto& typeToken = peekToken(info);

	bool isStatic = false;
	if (isKeyword(typeToken, "static"))
	{
		if (!isInFunction(currSym(info)))
			THROW_PROG_GEN_ERROR(typeToken.pos, "Static keyword can only be used in function definitions!");
		isStatic = true;
		nextToken(info);
	}

	auto datatype = getParseDatatype(info);
	if (!datatype)
		return false;

	auto& nameToken = nextToken(info);
	if (!isIdentifier(nameToken))
		THROW_PROG_GEN_ERROR(nameToken.pos, "Expected identifier!");

	if (isSeparator(peekToken(info), "("))
	{
		if (!isInGlobal(currSym(info)))
			THROW_PROG_GEN_ERROR(peekToken(info).pos, "Functions are not allowed here!");
		parseExpectedDeclDefFunction(info, datatype, nameToken.value);
	}
	else
	{
		parseExpectedDeclDefVariable(info, datatype, isStatic, nameToken.value);
	}

	return true;
}

bool parseDeclExtFunc(ProgGenInfo& info)
{
	auto& extToken = peekToken(info);

	if (!isKeyword(extToken, "extern"))
		return false;
	nextToken(info);

	auto funcSym = std::make_shared<Symbol>();

	funcSym->pos = extToken.pos;
	funcSym->type = SymType::ExtFunc;

	auto& typeToken = peekToken(info);
	funcSym->func.retType = getParseDatatype(info);

	if (!funcSym->func.retType)
		THROW_PROG_GEN_ERROR(peekToken(info).pos, "Expected datatype!");

	auto& nameToken = nextToken(info);
	if (!isIdentifier(nameToken))
		THROW_PROG_GEN_ERROR(nameToken.pos, "Expected identifier!");
	funcSym->name = nameToken.value;

	parseExpected(info, Token::Type::Separator, "(");

	while (!isSeparator(peekToken(info), ")"))
	{
		SymbolRef paramSym = std::make_shared<Symbol>();
		paramSym->type = SymType::Variable;
		paramSym->pos = peekToken(info).pos;
		paramSym->parent = funcSym;
		auto& param = paramSym->var;
		param.context = SymVarContext::Parameter;

		param.datatype = getParseDatatype(info);
		if (!param.datatype)
			THROW_PROG_GEN_ERROR(peekToken(info).pos, "Expected datatype!");

		param.offset = funcSym->func.retOffset;
		funcSym->func.retOffset += getDatatypePushSize(info.program, param.datatype);

		if (!isIdentifier(peekToken(info)))
			THROW_PROG_GEN_ERROR(peekToken(info).pos, "Expected identifier!");

		param.modName = nextToken(info).value;
		paramSym->name = param.modName;

		funcSym->func.params.push_back(paramSym);

		if (isSeparator(peekToken(info), ")"))
			continue;

		parseExpected(info, Token::Type::Separator, ",");
	}

	parseExpected(info, Token::Type::Separator, ")");

	parseExpected(info, Token::Type::Operator, "=");

	auto& asmNameTok = nextToken(info);

	if (!isString(asmNameTok))
		THROW_PROG_GEN_ERROR(asmNameTok.pos, "Expected assembly name!");
	funcSym->func.asmName = asmNameTok.value;

	parseExpectedNewline(info);

	addSymbol(currSym(info), funcSym);

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

	if (!isVoid(info.funcRetType))
		lastStatement(info)->subExpr = genConvertExpression(info, getParseExpression(info), info.funcRetType);
	else if (!isNewline(peekToken(info)))
		THROW_PROG_GEN_ERROR(exprBegin.pos, "Return statement in void function must be followed by a newline!");
	parseExpectedNewline(info);

	return true;
}

bool parseInlineAssembly(ProgGenInfo& info)
{
	auto& asmToken = peekToken(info);
	if (!isKeyword(asmToken, "asm") && !isKeyword(asmToken, "assembly"))
		return false;
	nextToken(info);
	
	parseExpectedColon(info);
	bool doParseIndent = parseOptionalNewline(info);

	increaseIndent(info.indent);

	while (!doParseIndent || parseIndent(info))
	{
		doParseIndent = true;
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

void parseBody(ProgGenInfo& info, bool doParseIndent)
{
	int numStatements = 0;

	auto& bodyBeginToken = peekToken(info, -1);

	while (!doParseIndent || parseIndent(info))
	{
		doParseIndent = true;
		++numStatements;
		auto& token = peekToken(info);
		if (parseEmptyLine(info)) continue;
		if (parseStatementPass(info)) continue;
		if (parseControlFlow(info)) continue;
		if (parseStatementContinue(info)) continue;
		if (parseStatementBreak(info)) continue;
		if (parseDeclDef(info)) continue;
		if (parseStatementReturn(info)) continue;
		if (parseInlineAssembly(info)) continue;
		if (parseExpression(info)) continue;
		THROW_PROG_GEN_ERROR(token.pos, "Unexpected token: " + token.value + "!");
	}

	if (numStatements == 0)
		THROW_PROG_GEN_ERROR(bodyBeginToken.pos, "Expected non-empty body!");
}

void parseBodyEx(ProgGenInfo& info, BodyRef body, bool doParseIndent)
{
	increaseIndent(info.indent);
	pushTempBody(info, body);
	enterSymbol(info, addShadowSpace(info));

	parseBody(info, doParseIndent);

	exitSymbol(info);
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
		condBody.condition = genConvertExpression(info, getParseExpression(info), { "bool" });
		condBody.body = std::make_shared<Body>();

		parseExpectedColon(info);

		bool parsedNewline = parseOptionalNewline(info);

		parseBodyEx(info, condBody.body, parsedNewline);

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
			bool parsedNewline = parseOptionalNewline(info);

			parseBodyEx(info, statement->elseBody, parsedNewline);
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

	statement->whileConditionalBody.condition = genConvertExpression(info, getParseExpression(info), { "bool" });

	parseExpectedColon(info);
	bool parsedNewline = parseOptionalNewline(info);

	parseBodyEx(info, statement->whileConditionalBody.body, parsedNewline);

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
	bool parsedNewline = parseOptionalNewline(info);

	parseBodyEx(info, statement->doWhileConditionalBody.body, parsedNewline);

	if (!parseIndent(info))
		THROW_PROG_GEN_ERROR(doToken.pos, "Expected 'while'!");
	parseExpected(info, Token::Type::Keyword, "while");
	
	statement->doWhileConditionalBody.condition = genConvertExpression(info, getParseExpression(info), { "bool" });

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

void parseFunctionBody(ProgGenInfo& info, bool doParseIndent)
{
	auto& bodyBeginToken = peekToken(info, -1);
	
	parseBody(info, doParseIndent);

	if (info.program->body->statements.empty() || lastStatement(info)->type != Statement::Type::Return)
	{
		if (isVoid(info.funcRetType))
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

	auto& nameToken = nextToken(info, 1, true);

	if (!isIdentifier(nameToken))
		THROW_PROG_GEN_ERROR(nameToken.pos, "Expected identifier!");

	SymbolRef sym = std::make_shared<Symbol>();
	sym->type = SymType::Macro;
	sym->pos = nameToken.pos;
	sym->name = nameToken.value;

	auto existingSymbol = getSymbol(currSym(info), sym->name, true);
	if (existingSymbol)
		THROW_PROG_GEN_ERROR(nameToken.pos, "Symbol '" + sym->name + "' already exists in the same scope!");

	while (!isNewline(peekToken(info)))
		sym->macroTokens.push_back(nextToken(info));

	parseExpectedNewline(info);

	addSymbol(currSym(info), sym);

	return true;
}

bool parseSingleGlobalCode(ProgGenInfo& info);

bool parseStatementSpace(ProgGenInfo& info)
{
	auto& spaceToken = peekToken(info);
	if (!isKeyword(spaceToken, "space"))
		return false;
	nextToken(info);

	auto& nameToken = peekToken(info);
	nextToken(info);

	if (!isIdentifier(nameToken))
		THROW_PROG_GEN_ERROR(nameToken.pos, "Expected identifier!");

	parseExpectedColon(info);
	bool doParseIndent = parseOptionalNewline(info);

	auto spaceSym = getSymbol(currSym(info), nameToken.value, true);
	if (!spaceSym)
	{
		spaceSym = std::make_shared<Symbol>();
		spaceSym->type = SymType::Namespace;
		spaceSym->pos = nameToken.pos;
		spaceSym->name = nameToken.value;
		addSymbol(currSym(info), spaceSym);
	}

	increaseIndent(info.indent);
	enterSymbol(info, spaceSym);

	int codeCount = 0;

	while (!doParseIndent || parseIndent(info))
	{
		doParseIndent = true;
		auto& token = peekToken(info);
		if (!parseSingleGlobalCode(info))
			THROW_PROG_GEN_ERROR(token.pos, "Unexpected token: " + token.value + "!");
		++codeCount;
	}

	if (codeCount == 0)
		THROW_PROG_GEN_ERROR(spaceToken.pos, "Expected at least one statement!");

	exitSymbol(info);
	decreaseIndent(info.indent);

	return true;
}

bool parsePackUnion(ProgGenInfo& info)
{
	auto& packToken = peekToken(info);
	if (!isKeyword(packToken, "pack") && !isKeyword(packToken, "union"))
		return false;
	nextToken(info);

	auto& nameToken = peekToken(info);
	if (!isIdentifier(nameToken))
		THROW_PROG_GEN_ERROR(nameToken.pos, "Expected identifier!");
	nextToken(info);

	SymbolRef packSym = std::make_shared<Symbol>();
	packSym->type = SymType::Pack;
	packSym->pos = nameToken.pos;
	packSym->name = nameToken.value;
	packSym->pack.isUnion = isKeyword(packToken, "union");
	auto& pack = packSym->pack;

	bool reqPreDecl = isOperator(peekToken(info), "!");
	if (reqPreDecl)
		nextToken(info);

	auto preDeclSym = getSymbol(currSym(info), nameToken.value, true);
	if (reqPreDecl && !preDeclSym)
		THROW_PROG_GEN_ERROR(peekToken(info).pos, "Missing pre-declaration before explicit pack/union definition!");
	else if (!reqPreDecl && preDeclSym)
		PRINT_WARNING(MAKE_PROG_GEN_ERROR(peekToken(info).pos, "Pack/Union: '" + nameToken.value + "' was pre-declared but not marked as such. You may want to do so."));

	bool doParseIndent = false;
	if (isSeparator(peekToken(info), "..."))
	{
		packSym->state = SymState::Declared;
		parseExpected(info, Token::Type::Separator, "...");
		parseExpectedNewline(info);
		addPack(info, packSym);
		return true;
	}
	else if (isSeparator(peekToken(info), ":"))
	{
		packSym->state = SymState::Defined;
		parseExpectedColon(info);
		doParseIndent = parseOptionalNewline(info);
		packSym = addPack(info, packSym);
	}
	else
		THROW_PROG_GEN_ERROR(peekToken(info).pos, "Expected '...' or ':'!");

	increaseIndent(info.indent);
	enterSymbol(info, packSym);

	while (!doParseIndent || parseIndent(info))
	{
		doParseIndent = true;
		if (parseStatementPass(info))
			continue;
		if (!parseDeclDef(info))
			THROW_PROG_GEN_ERROR(peekToken(info).pos, "Expected member definition!");
	}

	exitSymbol(info);
	decreaseIndent(info.indent);

	packSym->state = SymState::Defined;

	return true;
}

bool parseEnum(ProgGenInfo& info)
{
	auto& enumToken = peekToken(info);
	if (!isKeyword(enumToken, "enum"))
		return false;
	nextToken(info);

	auto& nameToken = peekToken(info);
	if (!isIdentifier(nameToken))
		THROW_PROG_GEN_ERROR(nameToken.pos, "Expected identifier!");
	nextToken(info);

	if (getSymbol(currSym(info), nameToken.value, true))
		THROW_PROG_GEN_ERROR(nameToken.pos, "Symbol with name '" + nameToken.value + "' already exists!");

	parseExpectedColon(info);
	bool doParseIndent = parseOptionalNewline(info);

	auto enumSym = std::make_shared<Symbol>();
	enumSym->pos = nameToken.pos;
	enumSym->name = nameToken.value;
	enumSym->type = SymType::Enum;
	enumSym->state = SymState::Defined;

	addSymbol(currSym(info), enumSym);

	int currIndex = 0;
	increaseIndent(info.indent);
	while (!doParseIndent || parseIndent(info))
	{
		doParseIndent = true;
		while (isIdentifier(peekToken(info))) {
			auto& memberToken = nextToken(info);

			if (getSymbol(enumSym, memberToken.value, true) != nullptr)
				THROW_PROG_GEN_ERROR(memberToken.pos, "Enum member '" + memberToken.value + "' already exists!");

			if (isOperator(peekToken(info), "="))
			{
				nextToken(info);
				auto& valToken = nextToken(info);
				if (valToken.type != Token::Type::LiteralInteger)
					THROW_PROG_GEN_ERROR(valToken.pos, "Expected integer literal!");

				currIndex = std::stoul(valToken.value);
			}

			auto memSym = std::make_shared<Symbol>();
			memSym->pos = memberToken.pos;
			memSym->name = memberToken.value;
			memSym->type = SymType::EnumMember;

			memSym->enumValue = currIndex++;

			addSymbol(enumSym, memSym);

			if (!isSeparator(peekToken(info), ","))
				break;
			nextToken(info);
		}

		parseExpectedNewline(info);
	}
	decreaseIndent(info.indent);

	return true;
}

void parseGlobalCode(ProgGenInfo& info)
{
	info.currToken = info.tokens->begin();

	while (!isEndOfCode(peekToken(info)))
	{
		if (!parseIndent(info))
			THROW_PROG_GEN_ERROR(peekToken(info).pos, "Expected indentation!");
		auto& token = peekToken(info);
		if (!parseSingleGlobalCode(info))
			THROW_PROG_GEN_ERROR(token.pos, "Unexpected token: " + token.value + "!");
	}
}

void importFile(ProgGenInfo& info, const Token& fileToken)
{
	auto getImportFile = [&](const std::string& dir) -> std::string
	{
		std::string path = (std::filesystem::path(dir) / fileToken.value).string();
		std::string absPath;
		try
		{
			absPath = std::filesystem::canonical(path).string();
		}
		catch (std::filesystem::filesystem_error&)
		{
			return "<notFound>";
		}

		if (info.imports.find(absPath) != info.imports.end())
			return "<alreadyImported>";

		if (std::filesystem::is_regular_file(path))
		{
			info.imports.insert(absPath);
			return path;
		}
		return "<notFound>";
	};

	std::string path = getImportFile(std::filesystem::path(info.progPath).parent_path().string());

	if (path == "<alreadyImported>")
		return;
	
	if (path == "<notFound>")
	{
		for (auto& dir : info.importDirs)
		{
			path = getImportFile(dir);

			if (path == "<alreadyImported>")
				return;

			if (path != "<notFound>")
				break;
		}

		if (path == "<notFound>")
			THROW_PROG_GEN_ERROR(fileToken.pos, "Import file not found: '" + fileToken.value + "'!");
	}

	std::string code = readTextFile(path);

	auto backup = makeProgGenInfoBackup(info);

	info.tokens = tokenize(code, path);
	info.progPath = path;
	info.indent = {};
	parseGlobalCode(info);

	loadProgGenInfoBackup(info, backup);
}

bool parseStatementImport(ProgGenInfo& info)
{
	auto& importToken = peekToken(info);
	if (!isKeyword(importToken, "import"))
		return false;
	nextToken(info);

	bool platformMatch = true;
	bool isDeferred = false;

	while (isOperator(peekToken(info), "."))
	{
		nextToken(info);
		auto& specToken = nextToken(info);
		if (!isIdentifier(specToken))
			THROW_PROG_GEN_ERROR(specToken.pos, "Expected platform identifier!");

		if (specToken.value == "defer")
		{
			isDeferred = true;
			continue;
		}

		static const std::set<std::string> platformNames = { "windows", "linux", "macos" };
		if (platformNames.find(specToken.value) == platformNames.end())
			THROW_PROG_GEN_ERROR(specToken.pos, "Unknown platform identifier: '" + specToken.value + "'!");
		if (specToken.value != info.program->platform)
		{
			platformMatch = false;
			continue;
		}
	}

	auto& fileToken = nextToken(info);
	if (!isString(fileToken))
		THROW_PROG_GEN_ERROR(fileToken.pos, "Expected file path!");

	if (!platformMatch)
		return true;

	if (isDeferred)
		info.deferredImports.push_back(fileToken);
	else
		importFile(info, fileToken);

	return true;
}

bool parseSingleGlobalCode(ProgGenInfo& info)
{
	if (parseEmptyLine(info)) return true;
	if (parseStatementPass(info)) return true;
	if (parseStatementImport(info)) return true;
	if (parseStatementDefine(info)) return true;
	if (parseStatementSpace(info)) return true;
	if (parseControlFlow(info)) return true;
	if (parseStatementContinue(info)) return true;
	if (parseStatementBreak(info)) return true;
	if (parsePackUnion(info)) return true;
	if (parseEnum(info)) return true;
	if (parseDeclDef(info)) return true;
	if (parseDeclExtFunc(info)) return true;
	if (parseInlineAssembly(info)) return true; 
	if (parseInlineAssembly(info)) return true;
	if (parseExpression(info)) return true;
	return false;
}

void markReachableFunctions(ProgGenInfo& info, BodyRef body)
{
	for (auto& funcPath : body->usedFunctions)
	{
		auto func = getSymbolFromPath(info.program->symbols, funcPath);

		if (func->func.isReachable)
			continue;

		func->func.isReachable = true;
		if (!isExtFunc(func))
			markReachableFunctions(info, func->func.body);
	}
}

void detectUndefinedFunctions(ProgGenInfo& info)
{
	for (auto sym : *info.program->symbols)
	{
		if (isFuncSpec(sym) && !isDefined(sym) && isReachable(sym))
			THROW_PROG_GEN_ERROR(sym->pos, "Cannot reference undefined function '" + getMangledName(sym) + "'!");
	}
}

void importDeferredImports(ProgGenInfo& info)
{
	for (int i = 0; i < info.deferredImports.size(); ++i)
		importFile(info, info.deferredImports[i]);
}

ProgramRef generateProgram(const TokenListRef tokens, const std::set<std::string>& importDirs, const std::string& platform, const std::string& progPath)
{
	ProgGenInfo info = { tokens, ProgramRef(new Program()), importDirs, progPath };
	info.program->platform = platform;
	info.program->body = std::make_shared<Body>();
	info.program->symbols = std::make_shared<Symbol>();
	enterSymbol(info, info.program->symbols);

	auto sym = info.program->symbols;
	sym->pos = {};
	sym->name = "<global>";
	sym->type = Symbol::Type::Global;

	parseGlobalCode(info);

	importDeferredImports(info);

	markReachableFunctions(info, info.program->body);
	detectUndefinedFunctions(info);

	return info.program;
}
