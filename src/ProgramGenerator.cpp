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

	std::stack<BodyRef> mainBodyBackups;
	int funcRetOffset;
	int funcFrameSize;
	Datatype funcRetType;

	std::set<std::string> imports;

	std::map<std::string, Token> defSymbols;

	int continueEnableCount = 0;
	int breakEnableCount = 0;

	std::map<std::string, std::map<std::string, uint>> enums;
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

void mergeSubUsages(BodyRef parent, BodyRef child)
{
	parent->usedFunctions.merge(child->usedFunctions);
	child->usedFunctions.clear();
}

void pushStatement(ProgGenInfo& info, StatementRef statement)
{
	info.program->body->statements.push_back(statement);
}

void pushStaticLocalInit(ProgGenInfo& info, ExpressionRef initExpr)
{
	auto ifStatement = std::make_shared<Statement>(initExpr->pos, Statement::Type::If_Clause);

	ifStatement->ifConditionalBodies.push_back(ConditionalBody());
	auto& condBody = ifStatement->ifConditionalBodies.back();
	
	condBody.condition = std::make_shared<Expression>(initExpr->pos);
	condBody.condition->eType = Expression::ExprType::LabeledVariable;
	condBody.condition->isLValue = true;
	condBody.condition->datatype = { "bool" };
	condBody.condition->globName = getStaticLocalInitName(info.program->staticLocalInitCount);

	auto assignExpr = std::make_shared<Expression>(initExpr->pos);
	assignExpr->eType = Expression::ExprType::Assign;
	
	assignExpr->left = std::make_shared<Expression>(initExpr->pos);
	assignExpr->left->eType = Expression::ExprType::LabeledVariable;
	assignExpr->left->isLValue = true;
	assignExpr->left->datatype = { "bool" };
	assignExpr->left->globName = getStaticLocalInitName(info.program->staticLocalInitCount);
	
	assignExpr->right = std::make_shared<Expression>(initExpr->pos);
	assignExpr->right->eType = Expression::ExprType::Literal;
	assignExpr->right->isLValue = false;
	assignExpr->right->datatype = { "bool" };
	assignExpr->right->valStr = "0";

	condBody.body = std::make_shared<Body>();
	condBody.body->statements.push_back(assignExpr);
	condBody.body->statements.push_back(initExpr);

	pushStatement(info, ifStatement);

	++info.program->staticLocalInitCount;
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
			if (paramExpr[i]->datatype != func->func.params[i]->var.datatype)
				isExactMatch = false;
			matchFound = !!genConvertExpression(paramExpr[i], func->func.params[i]->var.datatype, false, false);
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
			paramExpr[i] = genConvertExpression(paramExpr[i], match->func.params[i]->var.datatype, false);
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

SymbolRef getEnum(ProgGenInfo& info, const std::string& name)
{
	auto symbol = getSymbol(currSym(info), name);
	if (!isEnum(symbol))
		return nullptr;

	return symbol;
}

#define ERR_ENUM_NAME_NOT_FOUND -1
#define ERR_ENUM_VALUE_NOT_FOUND -2
uint getEnumValue(ProgGenInfo& info, const std::string& enumName, const std::string& memberName)
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

void addVariable(ProgGenInfo& info, SymbolRef sym)
{
	auto& var = sym->var;
	var.modName = sym->name;

	if (isInPack(currSym(info)))
	{
		var.offset = currSym(info)->pack.size;
		currSym(info)->pack.size += getDatatypeSize(info.program, var.datatype);

		addSymbol(currSym(info), sym);
		return;
	}

	int varSize = getDatatypeSize(info.program, var.datatype);

	auto symbol = getSymbol(currSym(info), sym->name, true);
	if (symbol)
		THROW_PROG_GEN_ERROR(sym->pos, "Symbol '" + sym->name + "' already exists in the same scope!");

	static int staticID = 0;
	if (isVarStatic(sym))
		var.modName = "static_" + std::to_string(staticID++) + "~" + sym->name;
	static int globVarID = 0;
	if (isInGlobal(currSym(info)))
		var.modName.append("~" + std::to_string(globVarID++));

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

void addFunction(ProgGenInfo& info, SymbolRef func)
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
		return;
	}
	
	if (existingOverload->func.retType != func->func.retType)
		THROW_PROG_GEN_ERROR(func->pos, "Function '" + func->name + "' already exists with different return type!");
	if (!isDefined(func))
		return;
	if (isDefined(existingOverload))
		THROW_PROG_GEN_ERROR(func->pos, "Function already defined here: " + getPosStr(existingOverload->pos));

	replaceSymbol(funcs, func->name, func, true);
}

void addPack(ProgGenInfo& info, SymbolRef pack)
{
	auto existingPack = getSymbol(currSym(info), pack->name, true);
	
	if (!existingPack)
	{
		addSymbol(currSym(info), pack);
		return;
	}

	if (!isDefined(pack))
		return;
	if (isDefined(existingPack))
		THROW_PROG_GEN_ERROR(pack->pos, "Pack already defined here: " + getPosStr(existingPack->pos));

	*existingPack = *pack;
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
				
				if (isVarOffset(pVar))
				{
					int offset = pVar->var.offset;
					result += (offset < 0) ? "- " : "+ ";
					result += std::to_string(std::abs(offset));
				}
				else
				{
					result += getMangledName(pVar);
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

void autoFixDatatypeMismatch(ProgGenInfo& info, ExpressionRef exp)
{
	if (exp->left->datatype == exp->right->datatype)
		return;

	if (isPointer(exp->left->datatype) || isArray(exp->left->datatype))
	{
		switch (exp->eType)
		{
		case Expression::ExprType::Sum:
		case Expression::ExprType::Difference:
		case Expression::ExprType::Assign_Sum:
		case Expression::ExprType::Assign_Difference:
			exp->right = genConvertExpression(exp->right, { "u64" });
			if (isArray(exp->left->datatype))
			{
				auto temp = std::make_shared<Expression>(exp->pos);
				temp->eType = Expression::ExprType::Product;
				temp->right = exp->right;
				temp->datatype = { "u64" };
				
				temp->left = std::make_shared<Expression>(exp->pos);
				temp->left->eType = Expression::ExprType::Literal;
				temp->left->valStr = std::to_string(getDatatypePointedToSize(info.program, exp->left->datatype));
				temp->left->datatype = { "u64" };
				temp->left->isLValue = false;

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
	
	exp->left = genConvertExpression(exp->left, newDatatype);
	exp->right = genConvertExpression(exp->right, newDatatype);
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
	parseExpected(info, Token::Type::Separator, "::");

	auto& memberToken = nextToken(info);

	if (!isIdentifier(memberToken))
		THROW_PROG_GEN_ERROR(memberToken.pos, "Expected enum member identifier!");

	uint value = getEnumValue(info, enumToken.value, memberToken.value);

	if (value == ERR_ENUM_NAME_NOT_FOUND)
		THROW_PROG_GEN_ERROR(enumToken.pos, "Enum '" + enumToken.value + "' not found!");
	if (value == ERR_ENUM_VALUE_NOT_FOUND)
		THROW_PROG_GEN_ERROR(memberToken.pos, "Enum member '" + memberToken.value + "' not found in enum '" + enumToken.value + "'!");

	auto expr = std::make_shared<Expression>(memberToken.pos);
	expr->eType = Expression::ExprType::Literal;
	expr->datatype = { "u32" };
	expr->valStr = std::to_string(value);
	expr->isLValue = false;

	return expr;
}

ExpressionRef getParseLiteral(ProgGenInfo& info)
{
	auto& litToken = peekToken(info);
	if (!isLiteral(litToken))
		return getParseEnumMember(info);

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
		exp->valStr = getLiteralStringName(info.program->strings.size());
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

	exp->eType = isVarLabeled(pVar) ? Expression::ExprType::LabeledVariable : Expression::ExprType::OffsetVariable;
	exp->localOffset = pVar->var.offset;
	exp->datatype = pVar->var.datatype;
	exp->globName = pVar->var.modName;
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

ExpressionRef getParseSpaceName(ProgGenInfo& info, bool localOnly)
{
	auto& litToken = peekToken(info);
	if (!isIdentifier(litToken))
		return nullptr;

	auto spaceSymbol = getSymbol(currSym(info), litToken.value, localOnly);
	if (!spaceSymbol)
		return nullptr;

	ExpressionRef exp = std::make_shared<Expression>(litToken.pos);
	exp->eType = Expression::ExprType::SpaceName;
	exp->spaceName = litToken.value;
	exp->spaceSymbol = spaceSymbol;

	nextToken(info);

	return exp;
}

ExpressionRef getParseValue(ProgGenInfo& info, bool localOnly = false)
{
	auto exp = getParseLiteral(info);
	if (exp) return exp;

	exp = getParseVariable(info);
	if (exp) return exp;

	exp = getParseFunctionName(info);
	if (exp) return exp;

	exp = getParseSpaceName(info, localOnly);
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
			autoFixDatatypeMismatch(info, exp);
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
			autoFixDatatypeMismatch(info, exp);
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
			autoFixDatatypeMismatch(info, exp);
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
			autoFixDatatypeMismatch(info, exp);
			exp->datatype = exp->left->datatype;
			exp->isLValue = false;
			break;
		case Expression::ExprType::Namespace:
			if (exp->left->eType != Expression::ExprType::SpaceName)
				THROW_PROG_GEN_ERROR(exp->left->pos, "Expected namespace name!");
			enterSymbol(info, exp->left->spaceSymbol);

			exp = getParseValue(info, true);

			exitSymbol(info);
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
				auto func = getMatchingOverload(info, overloads, exp->paramExpr);
				if (!func)
					THROW_PROG_GEN_ERROR(exp->pos, "No matching overload found for function '" + exp->left->funcName + "'!");
				exp->datatype = func->func.retType;
				exp->left->datatype = { getSignature(func), 1 };
				exp->left->funcName = getMangledName(exp->left->funcName, exp.get());

				info.program->body->usedFunctions.insert(getSymbolPath(nullptr, func, true));
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

			auto packSym = getSymbol(currSym(info), exp->left->datatype.name);
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
			exp->memberOffset = memberSym->var.offset;
			exp->isLValue = true;
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
	case Expression::ExprType::Namespace:
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

ExpressionRef getParseDeclDefVariable(ProgGenInfo& info, const Datatype& datatype, bool isStatic, const std::string& name)
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
	else if (isInEnum(currSym(info)))
		sym->var.context = SymVarContext::EnumMember;
	else
		sym->var.context = SymVarContext::Global;

	while (isSeparator(peekToken(info), "["))
	{
		nextToken(info);
		parseExpected(info, Token::Type::LiteralInteger);
		++sym->var.datatype.ptrDepth;
		sym->var.datatype.arraySizes.push_back(std::stoull(peekToken(info, -1).value));
		if (sym->var.datatype.arraySizes.back() <= 0)
			THROW_PROG_GEN_ERROR(peekToken(info, -1).pos, "Array size must be greater than zero!");
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
			nextToken(info, -1);
			initExpr = getParseExpression(info);
		}
	}
	
	parseExpectedNewline(info);

	return initExpr;
}

void parseExpectedDeclDefVariable(ProgGenInfo& info, const Datatype& datatype, bool isStatic, const std::string& name)
{
	if (datatype == Datatype{ "void" })
		THROW_PROG_GEN_ERROR(peekToken(info).pos, "Cannot declare variable of type void!");

	auto initExpr = getParseDeclDefVariable(info, datatype, isStatic, name);
	if (initExpr)
	{
		if (isStatic)
			pushStaticLocalInit(info, initExpr);
		else
			pushStatement(info, initExpr);
	}
}

void parseExpectedDeclDefFunction(ProgGenInfo& info, const Datatype& datatype, const std::string& name)
{
	SymbolRef funcSym = std::make_shared<Symbol>();
	auto& func = funcSym->func;

	funcSym->pos = peekToken(info).pos;
	funcSym->type = SymType::FunctionSpec;
	funcSym->name = name;
	func.body = std::make_shared<Body>();
	func.retType = datatype;

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
		
		param.offset = func.retOffset;
		func.retOffset += getDatatypePushSize(info.program, param.datatype);

		if (!isIdentifier(peekToken(info)))
			THROW_PROG_GEN_ERROR(peekToken(info).pos, "Expected identifier!");

		param.modName = nextToken(info).value;
		paramSym->name = param.modName;

		func.params.push_back(paramSym);

		if (isSeparator(peekToken(info), ")"))
			continue;

		parseExpected(info, Token::Type::Separator, ",");
	}

	parseExpected(info, Token::Type::Separator, ")");

	funcSym->state = SymState::Defined;
	if (isSeparator(peekToken(info), "..."))
	{
		funcSym->state = SymState::Declared;

		nextToken(info);
		parseExpectedNewline(info);
	}

	addFunction(info, funcSym);

	if (isDefined(funcSym))
	{
		parseExpectedColon(info);
		parseExpectedNewline(info);

		increaseIndent(info.indent);
		pushTempBody(info, func.body);
		enterSymbol(info, funcSym);

		info.funcRetOffset = func.retOffset;
		info.funcRetType = func.retType;
		info.funcFrameSize = 0;

		for (auto& param : func.params)
			addSymbol(funcSym, param);

		parseFunctionBody(info);
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

	if (getDatatypeSize(info.program, datatype) < 0)
		THROW_PROG_GEN_ERROR(typeToken.pos, "Unknown datatype!");

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
		if (parseDeclDef(info)) continue;
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
	enterSymbol(info, addShadowSpace(info));

	parseBody(info);

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
	parseExpectedNewline(info);

	auto spaceSym = std::make_shared<Symbol>();
	spaceSym->type = SymType::Namespace;
	spaceSym->pos = nameToken.pos;
	spaceSym->name = nameToken.value;

	increaseIndent(info.indent);
	addSymbol(currSym(info), spaceSym);
	enterSymbol(info, spaceSym);

	int codeCount = 0;

	while (parseIndent(info))
	{
		auto token = (*info.tokens)[info.currToken];
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

	SymbolRef packSym = std::make_shared<Symbol>();
	packSym->type = SymType::Pack;
	packSym->pos = nameToken.pos;
	packSym->name = nameToken.value;
	auto& pack = packSym->pack;

	packSym->state = SymState::Defined;
	if (isSeparator(peekToken(info), "..."))
	{
		packSym->state = SymState::Declared;
		nextToken(info);
		parseExpectedNewline(info);
		return true;
	}

	addPack(info, packSym);
	
	parseExpectedColon(info);
	parseExpectedNewline(info);

	increaseIndent(info.indent);
	enterSymbol(info, packSym);

	while (parseIndent(info))
	{
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

	if (getEnum(info, nameToken.value))
		THROW_PROG_GEN_ERROR(nameToken.pos, "Enum with name '" + nameToken.value + "' already exists!");

	if (getVariable(info, nameToken.value))
		THROW_PROG_GEN_ERROR(nameToken.pos, "Variable with name '" + nameToken.value + "' already exists!");
	
	if (getFunctions(info, nameToken.value))
		THROW_PROG_GEN_ERROR(nameToken.pos, "Function with name '" + nameToken.value + "' already exists!");

	if (getPackSize(info.program, nameToken.value) >= 0)
		THROW_PROG_GEN_ERROR(nameToken.pos, "Pack with name '" + nameToken.value + "' already exists!");

	parseExpectedColon(info);
	parseExpectedNewline(info);

	info.enums.insert({ nameToken.value, {} });
	auto& enumRef = info.enums.find(nameToken.value)->second;

	int currIndex = 0;
	increaseIndent(info.indent);
	while (parseIndent(info))
	{
		while (isIdentifier(peekToken(info))) {
			auto& memberToken = nextToken(info);

			if (enumRef.find(memberToken.value) != enumRef.end())
				THROW_PROG_GEN_ERROR(memberToken.pos, "Enum member '" + memberToken.value + "' already exists!");

			if (isOperator(peekToken(info), "="))
			{
				nextToken(info);
				auto& valToken = nextToken(info);
				if (valToken.type != Token::Type::LiteralInteger)
					THROW_PROG_GEN_ERROR(valToken.pos, "Expected integer literal!");

				currIndex = std::stoul(valToken.value);
			}

			enumRef.insert({ memberToken.value, currIndex++ });

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
	while (info.currToken < info.tokens->size())
	{
		if (!parseIndent(info))
			THROW_PROG_GEN_ERROR(peekToken(info).pos, "Expected indent!");
		auto token = (*info.tokens)[info.currToken];
		if (!parseSingleGlobalCode(info))
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
	if (parsePack(info)) return true;
	if (parseEnum(info)) return true;
	if (parseDeclDef(info)) return true;
	if (parseSinglelineAssembly(info)) return true;
	if (parseMultilineAssembly(info)) return true;
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
		markReachableFunctions(info, func->func.body);
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
	info.program->symbols = std::make_shared<Symbol>();
	enterSymbol(info, info.program->symbols);

	auto sym = info.program->symbols;
	sym->pos = {};
	sym->name = "<global>";
	sym->type = Symbol::Type::Global;

	parseGlobalCode(info);

	markReachableFunctions(info, info.program->body);
	//detectUndefinedFunctions(info.program->functions);

	return info.program;
}