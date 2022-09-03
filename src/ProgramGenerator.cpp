#include "ProgramGenerator.h"

#include <map>
#include <set>
#include <stack>
#include <algorithm>
#include <filesystem>
#include <cassert>
#include <queue>

#include "Warning.h"
#include "Errors/ProgGenError.h"

#include "Tokenizer.h"
#include "OperatorPrecedence.h"

#define BLUEPRINT_SYMBOL_NAME "&_BLUEPRINTS_&"

#define ENABLE_EXPR_ONLY_FOR_OBJ(expr) \
	if (!expr->isObject)               \
	THROW_PROG_GEN_ERROR_POS(expr->pos, "Expected object!")
#define ENABLE_EXPR_ONLY_FOR_NON_OBJ(expr) \
	if (expr->isObject)                    \
	THROW_PROG_GEN_ERROR_POS(expr->pos, "Expected non-object!")
#define ENABLE_EXPR_ONLY_FOR_NON_CONST(expr) \
	if (expr->datatype.isConst)              \
	THROW_PROG_GEN_ERROR_POS(expr->pos, "Expected non-constant!")

ProgGenInfoBackup makeProgGenInfoBackup(const ProgGenInfo &info)
{
	ProgGenInfoBackup backup;
	backup.tokens = info.tokens;
	backup.progPath = info.progPath;
	backup.currToken = info.currToken;
	backup.indent = info.indent;
	backup.funcRetOffset = info.funcRetOffset;
	backup.funcFrameSize = info.funcFrameSize;
	backup.funcRetType = info.funcRetType;
	return backup;
}

void loadProgGenInfoBackup(ProgGenInfo &info, const ProgGenInfoBackup &backup)
{
	info.tokens = backup.tokens;
	info.progPath = backup.progPath;
	info.currToken = backup.currToken;
	info.indent = backup.indent;
	info.funcRetOffset = backup.funcRetOffset;
	info.funcFrameSize = backup.funcFrameSize;
	info.funcRetType = backup.funcRetType;
}

SymbolRef currSym(const ProgGenInfo &info)
{
	return currSym(info.program);
}

void enterSymbol(ProgGenInfo &info, SymbolRef symbol)
{
	assert(symbol);
	info.program->symStack.push(symbol);
}

void exitSymbol(ProgGenInfo &info)
{
	assert(!info.program->symStack.empty());
	info.program->symStack.pop();
}

// Creates a space with an internal, mangled name.
// Shadow spaces are used to combat name collisions in if/elif/else, while, do/while, for, ... statements.
SymbolRef addShadowSpace(ProgGenInfo &info)
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

TokenList::iterator moveTokenIterator(TokenList &list, TokenList::iterator it, int offset)
{
	if (offset > 0) // Move to next items
	{
		while (offset-- > 0)
		{
			++it;
			if (it == list.end()) // Duplicate and append the last element when moving out of bounds
				it = list.insert(it, list.back());
		}
	}
	else if (offset < 0) // Move to previous items
	{
		while (offset++ < 0)
		{
			--it;
			if (it == list.begin()) // Reached beginning
				return it;
		}
	}

	return it;
}

TokenList DatatypeToTokenList(const Datatype &datatype)
{
	TokenList tokens;

	auto insertTokenFront = [&tokens](const Token::Type type, const std::string &value)
	{
		tokens.insert(tokens.begin(), makeToken(type, value));
	};

	auto pDt = &datatype;
	while (pDt)
	{
		if (pDt->isConst)
			insertTokenFront(Token::Type::Keyword, "const");

		if (isOfType(*pDt, DTType::Name) || isOfType(*pDt, DTType::Macro))
			insertTokenFront(isBuiltinType(pDt->name) ? Token::Type::BuiltinType : Token::Type::Identifier, pDt->name);
		else if (isOfType(*pDt, DTType::Array))
			assert(false && "Array datatypes not supported!");
		else if (isOfType(*pDt, DTType::Pointer))
			insertTokenFront(Token::Type::Operator, "*");
		else if (isOfType(*pDt, DTType::Reference))
			assert(false && "Reference datatypes not supported!");
		else
			assert(false && "Unknown datatype type!");

		pDt = pDt->subType.get();
	}

	return tokens;
}

Token &kwFileToTokString(Token &tok)
{
	tok.type = Token::Type::String;
	tok.value = std::filesystem::canonical(tok.pos.file).string();
	return tok;
}

Token &kwLineToTokInteger(Token &tok)
{
	tok.type = Token::Type::LiteralInteger;
	tok.value = std::to_string(tok.pos.line);
	return tok;
}

void expandMacro(ProgGenInfo &info, TokenList::iterator &tokIt, TokenList::iterator &begin, SymbolRef sym, SymbolRef &curr)
{
	auto newPos = tokIt->pos;
	bool updateCurrToken = (begin == info.currToken);

	std::vector<TokenList> macroArgs;

	if (sym->macroIsFunctionLike) // Parse parameters of function like macro
	{
		if (!isSeparator(*++tokIt, "("))
			THROW_PROG_GEN_ERROR_TOKEN(*tokIt, "Expected '(' after function-like macro!");

		for (int i = 0; i < sym->macroParamNames.size(); ++i)
		{
			++tokIt;
			macroArgs.push_back({});

			int parenCount = 0;
			while (parenCount != 0 || (!isSeparator(*tokIt, ",") && !isSeparator(*tokIt, ")")))
			{
				if (isSeparator(*tokIt, "("))
					++parenCount;
				else if (isSeparator(*tokIt, ")"))
					--parenCount;
				macroArgs[i].push_back(*tokIt);
				++tokIt;
			}
		}

		if (!isSeparator(*tokIt, ")"))
			THROW_PROG_GEN_ERROR_TOKEN(*tokIt, "Expected ')'!");
	}

	// Replace the macro with its content
	begin = info.tokens->erase(begin, ++tokIt);
	begin = info.tokens->insert(begin, sym->macroTokens.begin(), sym->macroTokens.end());

	if (sym->macroIsFunctionLike) // Replace all occurences of the parameters with their provided values
	{
		auto it = begin;
		for (int i = 0; i < sym->macroTokens.size(); ++i, ++it)
		{
			if (!isIdentifier(*it))
				continue;

			for (int paramIndex = 0; paramIndex < sym->macroParamNames.size(); ++paramIndex)
			{
				if (sym->macroParamNames[paramIndex] != it->value)
					continue;

				bool updateBegin = (it == begin);

				it = info.tokens->erase(it);
				it = info.tokens->insert(it, macroArgs[paramIndex].begin(), macroArgs[paramIndex].end());

				if (updateBegin)
					begin = it;

				for (int i = 0; i < macroArgs[paramIndex].size(); ++i)
					++it;

				break;
			}
		}
	}

	{ // Add new file/line positions for debugging
		auto it = begin;
		int nTokens = sym->macroTokens.size() - macroArgs.size();
		for (auto &arg : macroArgs)
			nTokens += arg.size();
		for (int i = 0; i < nTokens; ++i)
			addPosition(*it++, newPos);
	}

	tokIt = begin;
	if (updateCurrToken)
		info.currToken = begin;

	curr = currSym(info);
}

const Token &peekToken(ProgGenInfo &info, int offset, bool ignoreSymDef)
{
	auto tokIt = moveTokenIterator(*info.tokens, info.currToken, offset);

	if (ignoreSymDef)
		return *tokIt;

	if (isKeyword(*tokIt, "__file__"))
		return kwFileToTokString(*tokIt);

	if (isKeyword(*tokIt, "__line__"))
		return kwLineToTokInteger(*tokIt);

	auto begin = tokIt;

	bool localOnly = false;
	auto curr = currSym(info);
	if (isOperator(*tokIt, ".")) // Begin from global space when preceded by '.'
	{
		++tokIt;
		localOnly = true;
		curr = info.program->symbols;
	}

	while (tokIt != info.tokens->end() && isIdentifier(*tokIt))
	{
		auto &tok = *tokIt;

		auto sym = getSymbol(curr, tokIt->value, localOnly);
		if (!sym)
			break;

		if (isMacro(sym))
		{
			expandMacro(info, tokIt, begin, sym, curr);
		}
		else
		{
			curr = sym;
			++tokIt;
			localOnly = true;

			if (tokIt == info.tokens->end())
				break;

			if (!isOperator(*tokIt, "."))
				break;
			++tokIt;
		}
	}

	return *begin;
}

const Token &nextToken(ProgGenInfo &info, int offset, bool ignoreSymDef)
{
	auto &temp = peekToken(info, offset - 1, ignoreSymDef);

	info.currToken = moveTokenIterator(*info.tokens, info.currToken, offset);

	return temp;
}

void enableContinue(ProgGenInfo &info)
{
	++info.continueEnableCount;
}
void disableContinue(ProgGenInfo &info)
{
	assert(info.continueEnableCount > 0 && "Cannot disable continue, continue is not enabled.");
	--info.continueEnableCount;
}
bool isContinueEnabled(const ProgGenInfo &info)
{
	return info.continueEnableCount > 0;
}

void enableBreak(ProgGenInfo &info)
{
	++info.breakEnableCount;
}
void disableBreak(ProgGenInfo &info)
{
	assert(info.breakEnableCount > 0 && "Cannot disable break, break is not enabled.");
	--info.breakEnableCount;
}
bool isBreakEnabled(const ProgGenInfo &info)
{
	return info.breakEnableCount > 0;
}

void mergeSubUsages(BodyRef parent, BodyRef child)
{
	parent->usedFunctions.merge(child->usedFunctions);
	child->usedFunctions.clear();
}

void pushStatement(ProgGenInfo &info, StatementRef statement)
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

ExpressionRef makeLiteralExpression(Token::Position pos, const Datatype &datatype, EValue value)
{
	ExpressionRef exp = std::make_shared<Expression>(pos);
	exp->eType = Expression::ExprType::Literal;
	exp->isLValue = false;
	exp->isObject = true;
	exp->datatype = datatype;
	exp->value = value;

	return exp;
}

void pushStaticLocalInit(ProgGenInfo &info, ExpressionRef initExpr)
{
	// Create the internal if statement
	auto ifStatement = std::make_shared<Statement>(initExpr->pos, Statement::Type::If_Clause);

	// Add the body to execute when the variable has not been initialized yet
	ifStatement->ifConditionalBodies.push_back(ConditionalBody());
	auto &condBody = ifStatement->ifConditionalBodies.back();

	// Create the internal static variable to check whether to variable has been initializes or not
	auto statSym = std::make_shared<Symbol>();
	statSym->pos.decl = initExpr->pos;
	statSym->pos.def = initExpr->pos;
	statSym->name = getStaticLocalInitName(info.program->staticLocalInitCount++);
	statSym->type = SymType::Variable;
	statSym->var.datatype = {"bool"};
	statSym->var.context = SymVarContext::Global;

	addVariable(info, statSym);

	// Use the internal variable for the condition of the if statement
	condBody.condition = makeSymbolExpression(initExpr->pos, statSym);

	// Generate the assignment to execute when the static variable has not been initialized yet
	auto assignExpr = std::make_shared<Expression>(initExpr->pos);
	assignExpr->eType = Expression::ExprType::Assign;
	assignExpr->isObject = true;
	assignExpr->datatype = {"bool"};
	assignExpr->left = makeSymbolExpression(initExpr->pos, statSym);
	assignExpr->right = makeLiteralExpression(initExpr->pos, {"bool"}, {(uint64_t)0});

	condBody.body = std::make_shared<Body>();
	condBody.body->statements.push_back(assignExpr);
	condBody.body->statements.push_back(initExpr);

	pushStatement(info, ifStatement);
}

StatementRef lastStatement(ProgGenInfo &info)
{
	return info.program->body->statements.back();
}

bool isPackType(ProgGenInfo &info, const std::string &name)
{
	return isPackType(info.program, name);
}

bool isPackType(ProgGenInfo &info, const Token &token)
{
	if (!isIdentifier(token))
		return false;
	return isPackType(info, token.value);
}

// Checks whether a type conversion of the left-hand operand is prohibited or not
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

SymbolRef makeMacroSymbol(const Token::Position &pos, const std::string &name)
{
	SymbolRef sym = std::make_shared<Symbol>();
	sym->type = SymType::Macro;
	sym->pos.decl = pos;
	sym->name = name;
	return sym;
}

void parseInlineTokens(ProgGenInfo &info, TokenListRef tokens, const std::string &progPath)
{
	auto backup = makeProgGenInfoBackup(info);

	info.tokens = tokens;
	info.progPath = progPath;
	info.indent = {};
	parseGlobalCode(info);

	loadProgGenInfoBackup(info, backup);
}

std::string blueprintMacroNameFromName(const std::string &name)
{
	static int id = 0;
	return "_BM_N_#" + std::to_string(++id) + "_" + name;
}

std::string variadicNameFromID(int id)
{
	return "_VA_N_#" + std::to_string(id);
}

std::string variadicTypeFromID(int id)
{
	return "_VA_T_#" + std::to_string(id);
}

TokenList genVariadicParamDeclTokenList(const std::vector<int> varParamIDs)
{
	TokenList tokens;

	bool isFirstParam = true;

	for (int id : varParamIDs)
	{
		if (!isFirstParam)
			tokens.push_back(makeToken(Token::Type::Separator, ","));
		isFirstParam = false;

		tokens.push_back(makeToken(Token::Type::Identifier, variadicTypeFromID(id)));
		tokens.push_back(makeToken(Token::Type::Identifier, variadicNameFromID(id)));
	}

	return tokens;
}

void genBlueprintSpecPreSpace(const SymPath &path, TokenListRef tokens)
{
	if (path.empty())
		return;

	int nPerIndent = 0;
	for (auto &token : *tokens)
	{
		if (!isWhitespace(token))
			break;
		++nPerIndent;
	}
	nPerIndent /= path.size();

	if (nPerIndent == 0)
		THROW_QINP_ERROR("Unable to detect indentation!");

	auto it = tokens->begin();
	auto whitespace = makeToken(Token::Type::Whitespace, it->value);

	for (int i = 0; i < path.size(); ++i)
	{
		for (int j = 0; j < nPerIndent * i; ++j)
			it = ++tokens->insert(it, whitespace);

		it = ++tokens->insert(it, makeToken(Token::Type::Keyword, "space"));
		it = ++tokens->insert(it, makeToken(Token::Type::Identifier, path[i]));
		it = ++tokens->insert(it, makeToken(Token::Type::Separator, ":"));
		it = ++tokens->insert(it, makeToken(Token::Type::Newline, "\n"));
	}
}

SymbolRef generateBlueprintSpecialization(ProgGenInfo &info, SymbolRef &bpSym, std::vector<ExpressionRef> &paramExpr, const Token::Position &generatedFrom)
{
	// Check if the parameters resolve all blueprint macros (+ without conflicts)
	BlueprintMacroMap resolvedMacros;
	info.bpVariadicParamIDStack.push({});
	for (int i = 0; i < paramExpr.size(); ++i) // Loop over all parameters (including variadic)
	{
		static int variadicParamID = 0;

		SymbolRef param;

		std::string name;
		std::string mangledName;

		if (i < bpSym->func.params.size()) // If the parameter is not variadic
		{
			param = bpSym->func.params[i];

			if (param->var.datatype.type != DTType::Macro)
				continue;

			if (resolvedMacros.find(param->var.datatype.name) != resolvedMacros.end())
			{
				if (!dtEqual(resolvedMacros[param->var.datatype.name]->var.datatype, paramExpr[i]->datatype))
					THROW_PROG_GEN_ERROR_POS(param->pos.decl, "Blueprint parameter '" + param->name + "' has conflicting macro types!");
				continue;
			}

			name = param->var.datatype.name;
			mangledName = blueprintMacroNameFromName(name);
		}
		else // If the parameter is variadic
		{
			int varID = ++variadicParamID;
			info.bpVariadicParamIDStack.top().push_back(varID);

			param = std::make_shared<Symbol>();
			param->pos.decl = getBestPos(bpSym);
			param->var.datatype.name = variadicTypeFromID(varID);
			name = param->var.datatype.name;
			mangledName = name;
		}

		auto sym = makeMacroSymbol(param->pos.decl, mangledName);
		sym->var.datatype = dtArraysToPointer(paramExpr[i]->datatype);
		sym->macroTokens = DatatypeToTokenList(sym->var.datatype);
		resolvedMacros[name] = sym;
	}
	// Check if the return type is a blueprint macro
	if (
		bpSym->func.retType.type == DTType::Macro &&
		resolvedMacros.find(bpSym->func.retType.name) == resolvedMacros.end())
		THROW_PROG_GEN_ERROR_POS(getBestPos(bpSym), "Blueprint return type has not been resolved!");

	// Enter the space the blueprint was defined in
	enterSymbol(info, info.program->symbols);

	// Add macros
	for (auto &[name, sym] : resolvedMacros)
		addSymbol(currSym(info), sym);

	// Copy the blueprint tokens
	auto tokens = std::make_shared<TokenList>();
	*tokens = *bpSym->func.blueprintTokens;

	// Replace every macro occurence with the macro's mangled name
	for (auto it = tokens->begin(); it != tokens->end(); ++it)
	{
		if (!isIdentifier(*it))
			continue;

		if (resolvedMacros.find(it->value) == resolvedMacros.end())
			continue;

		it->value = resolvedMacros[it->value]->name;

		if (tokens->begin() != it && isOperator(*std::prev(it), "?"))
			tokens->erase(std::prev(it));
	}

	{ // Remove the '!' specifier if it exists
		auto it = tokens->begin();
		while (!isSeparator(*it++, ")"))
			; // Do nothing
		if (isOperator(*it, "!"))
			it = tokens->erase(it);
	}
	// Generate 'space [name]:' tokens
	genBlueprintSpecPreSpace(getSymbolPath(nullptr, getParent(bpSym, 3)), tokens);
	auto bpFilepath = bpSym->func.blueprintTokens->front().pos.file;
	if (!info.bpVariadicParamIDStack.top().empty())
	{
		auto itVar = tokens->begin();
		while (!isSeparator(*itVar, "..."))
			++itVar;
		itVar = tokens->erase(itVar);
		auto varParamDecl = genVariadicParamDeclTokenList(info.bpVariadicParamIDStack.top());
		tokens->insert(itVar, varParamDecl.begin(), varParamDecl.end());
	}

	// Generate the blueprint specialization
	try
	{
		parseInlineTokens(info, tokens, bpFilepath);
	}
	catch (const QinpError &err)
	{
		std::string paramStr = getReadableName(bpSym->func.params, false);
		if (bpSym->func.params.size() < paramExpr.size())
			paramStr += ", ";
		paramStr += getReadableName(std::vector(paramExpr.begin() + bpSym->func.params.size(), paramExpr.end()));

		auto symPathStr = SymPathToString(getSymbolPath(nullptr, getParent(bpSym, 2)));

		RETHROW_PROG_GEN_ERROR_POS(generatedFrom, "While generating a blueprint specialization for the function '" + symPathStr + "' with parameters (" + paramStr + ")", err);
	}

	exitSymbol(info);

	info.bpVariadicParamIDStack.pop();

	auto specialization = getMatchingOverload(info, getParent(bpSym, 2), paramExpr, generatedFrom);
	specialization->func.genFromBlueprint = true;

	if (isDeclared(specialization))
		info.bpSpecsToDefine.push_back({bpSym, paramExpr, generatedFrom});

	return specialization;
}

#define CONV_SCORE_MIN -0xFF
#define CONV_SCORE_BEGIN 0x0
#define CONV_SCORE_MAX 0x7FFFFFFF
#define CONV_SCORE_NOT_POSSIBLE -0xFFFF
#define CONV_SCORE_NO_CONV 0x0
#define CONV_SCORE_EXPLICIT 0x0
#define CONV_SCORE_MAKE_CONST 0x1
#define CONV_SCORE_MACRO 0x2
#define CONV_SCORE_VARIADIC 0x4
#define CONV_SCORE_PROMITION 0x8
#define CONV_SCORE_PTR_TO_VOID_PTR 0x10
#define CONV_SCORE_NARROW_CONV 0x20

int calcConvScore(ProgGenInfo &info, Datatype from, Datatype to, bool isExplicit, bool ignoreFirstConstness)
{
	auto retCorrect = [&isExplicit](int score) -> int
	{
		return isExplicit ? CONV_SCORE_EXPLICIT : score;
	};

	from = dtArraysToPointer(from);
	to = dtArraysToPointer(to);
	if (dtEqual(from, to, ignoreFirstConstness))
		return retCorrect(CONV_SCORE_NO_CONV);

	if (dtEqualNoConst(from, to) && preservesConstness(from, to, ignoreFirstConstness))
		return retCorrect(CONV_SCORE_MAKE_CONST);

	if (isPointer(from) && isVoidPtr(to) && preservesConstness(from, to, ignoreFirstConstness))
		return retCorrect(CONV_SCORE_PTR_TO_VOID_PTR);

	if (isNull(from))
		return retCorrect(CONV_SCORE_NO_CONV);

	if (isExplicit)
	{
		if (isEnum(info.program, from) || isEnum(info.program, to))
			return CONV_SCORE_EXPLICIT;

		if (isPointer(from) || isInteger(to))
			return CONV_SCORE_EXPLICIT;
	}

	if (
		from.type == DTType::Name &&
		to.type == DTType::Name &&
		isBuiltinType(from.name) &&
		isBuiltinType(to.name))
	{
		static const std::map<std::pair<std::string, std::string>, int> convScoreMap =
			{
				{{"bool", "bool"}, CONV_SCORE_NO_CONV},
				{{"bool", "i8"}, CONV_SCORE_PROMITION},
				{{"bool", "i16"}, CONV_SCORE_PROMITION},
				{{"bool", "i32"}, CONV_SCORE_PROMITION},
				{{"bool", "i64"}, CONV_SCORE_PROMITION},
				{{"bool", "u8"}, CONV_SCORE_PROMITION},
				{{"bool", "u16"}, CONV_SCORE_PROMITION},
				{{"bool", "u32"}, CONV_SCORE_PROMITION},
				{{"bool", "u64"}, CONV_SCORE_PROMITION},

				{{"i8", "bool"}, CONV_SCORE_NARROW_CONV},
				{{"i8", "i8"}, CONV_SCORE_NO_CONV},
				{{"i8", "i16"}, CONV_SCORE_PROMITION},
				{{"i8", "i32"}, CONV_SCORE_PROMITION},
				{{"i8", "i64"}, CONV_SCORE_PROMITION},
				{{"i8", "u8"}, CONV_SCORE_NARROW_CONV},
				{{"i8", "u16"}, CONV_SCORE_NARROW_CONV},
				{{"i8", "u32"}, CONV_SCORE_NARROW_CONV},
				{{"i8", "u64"}, CONV_SCORE_NARROW_CONV},

				{{"i16", "bool"}, CONV_SCORE_NARROW_CONV},
				{{"i16", "i8"}, CONV_SCORE_NARROW_CONV},
				{{"i16", "i16"}, CONV_SCORE_NO_CONV},
				{{"i16", "i32"}, CONV_SCORE_PROMITION},
				{{"i16", "i64"}, CONV_SCORE_PROMITION},
				{{"i16", "u8"}, CONV_SCORE_NARROW_CONV},
				{{"i16", "u16"}, CONV_SCORE_NARROW_CONV},
				{{"i16", "u32"}, CONV_SCORE_NARROW_CONV},
				{{"i16", "u64"}, CONV_SCORE_NARROW_CONV},

				{{"i32", "bool"}, CONV_SCORE_NARROW_CONV},
				{{"i32", "i8"}, CONV_SCORE_NARROW_CONV},
				{{"i32", "i16"}, CONV_SCORE_NARROW_CONV},
				{{"i32", "i32"}, CONV_SCORE_NO_CONV},
				{{"i32", "i64"}, CONV_SCORE_PROMITION},
				{{"i32", "u8"}, CONV_SCORE_NARROW_CONV},
				{{"i32", "u16"}, CONV_SCORE_NARROW_CONV},
				{{"i32", "u32"}, CONV_SCORE_NARROW_CONV},
				{{"i32", "u64"}, CONV_SCORE_NARROW_CONV},

				{{"i64", "bool"}, CONV_SCORE_NARROW_CONV},
				{{"i64", "i8"}, CONV_SCORE_NARROW_CONV},
				{{"i64", "i16"}, CONV_SCORE_NARROW_CONV},
				{{"i64", "i32"}, CONV_SCORE_NARROW_CONV},
				{{"i64", "i64"}, CONV_SCORE_NO_CONV},
				{{"i64", "u8"}, CONV_SCORE_NARROW_CONV},
				{{"i64", "u16"}, CONV_SCORE_NARROW_CONV},
				{{"i64", "u32"}, CONV_SCORE_NARROW_CONV},
				{{"i64", "u64"}, CONV_SCORE_NARROW_CONV},

				{{"u8", "bool"}, CONV_SCORE_NARROW_CONV},
				{{"u8", "i8"}, CONV_SCORE_NARROW_CONV},
				{{"u8", "i16"}, CONV_SCORE_PROMITION},
				{{"u8", "i32"}, CONV_SCORE_PROMITION},
				{{"u8", "i64"}, CONV_SCORE_PROMITION},
				{{"u8", "u8"}, CONV_SCORE_NO_CONV},
				{{"u8", "u16"}, CONV_SCORE_PROMITION},
				{{"u8", "u32"}, CONV_SCORE_PROMITION},
				{{"u8", "u64"}, CONV_SCORE_PROMITION},

				{{"u16", "bool"}, CONV_SCORE_NARROW_CONV},
				{{"u16", "i8"}, CONV_SCORE_NARROW_CONV},
				{{"u16", "i16"}, CONV_SCORE_NARROW_CONV},
				{{"u16", "i32"}, CONV_SCORE_PROMITION},
				{{"u16", "i64"}, CONV_SCORE_PROMITION},
				{{"u16", "u8"}, CONV_SCORE_NARROW_CONV},
				{{"u16", "u16"}, CONV_SCORE_NO_CONV},
				{{"u16", "u32"}, CONV_SCORE_PROMITION},
				{{"u16", "u64"}, CONV_SCORE_PROMITION},

				{{"u32", "bool"}, CONV_SCORE_NARROW_CONV},
				{{"u32", "i8"}, CONV_SCORE_NARROW_CONV},
				{{"u32", "i16"}, CONV_SCORE_NARROW_CONV},
				{{"u32", "i32"}, CONV_SCORE_NARROW_CONV},
				{{"u32", "i64"}, CONV_SCORE_PROMITION},
				{{"u32", "u8"}, CONV_SCORE_NARROW_CONV},
				{{"u32", "u16"}, CONV_SCORE_NARROW_CONV},
				{{"u32", "u32"}, CONV_SCORE_NO_CONV},
				{{"u32", "u64"}, CONV_SCORE_PROMITION},

				{{"u64", "bool"}, CONV_SCORE_NARROW_CONV},
				{{"u64", "i8"}, CONV_SCORE_NARROW_CONV},
				{{"u64", "i16"}, CONV_SCORE_NARROW_CONV},
				{{"u64", "i32"}, CONV_SCORE_NARROW_CONV},
				{{"u64", "i64"}, CONV_SCORE_NARROW_CONV},
				{{"u64", "u8"}, CONV_SCORE_NARROW_CONV},
				{{"u64", "u16"}, CONV_SCORE_NARROW_CONV},
				{{"u64", "u32"}, CONV_SCORE_NARROW_CONV},
				{{"u64", "u64"}, CONV_SCORE_NO_CONV},
			};

		auto it = convScoreMap.find({from.name, to.name});
		if (it == convScoreMap.end())
			return CONV_SCORE_NOT_POSSIBLE;
		return retCorrect(it->second);
	}

	return CONV_SCORE_NOT_POSSIBLE;
}

int calcFuncScore(ProgGenInfo &info, SymbolRef func, const std::vector<ExpressionRef> &paramExpr)
{
	if (!isFuncSpec(func))
		return CONV_SCORE_NOT_POSSIBLE;

	int score = CONV_SCORE_BEGIN;
	for (int i = 0; i < paramExpr.size(); ++i)
	{
		if (i < func->func.params.size()) // If the parameter is not variadic
		{
			auto &expectedType = func->func.params[i]->var.datatype;
			auto &actualType = paramExpr[i]->datatype;

			if (expectedType.type == DTType::Macro) // If the parameter is a blueprint macro
			{
				score += CONV_SCORE_MACRO;
			}
			else
			{
				int convScore = calcConvScore(info, actualType, expectedType, false, true);
				if (convScore == CONV_SCORE_NOT_POSSIBLE)
					return CONV_SCORE_NOT_POSSIBLE;
				score += convScore;
			}
		}
		else // If the parameter is variadic
		{
			score += CONV_SCORE_VARIADIC;
		}
	}

	return score;
}

void addPossibleCandidates(ProgGenInfo &info, std::map<SymbolRef, int> &candidates, SymbolRef overloads, std::vector<ExpressionRef> &paramExpr)
{
	if (!overloads)
		return;

	for (auto &[fName, fSym] : overloads->subSymbols)
	{
		if (fName == BLUEPRINT_SYMBOL_NAME)
		{
			addPossibleCandidates(info, candidates, fSym, paramExpr);
		}
		else
		{
			if (fSym->func.isVariadic && paramExpr.size() <= fSym->func.params.size())
				continue;
			if (!fSym->func.isVariadic && paramExpr.size() != fSym->func.params.size())
				continue;

			int score = calcFuncScore(info, fSym, paramExpr);
			if (score != CONV_SCORE_NOT_POSSIBLE)
				candidates[fSym] = score;
		}
	}
}

SymbolRef getMatchingOverload(ProgGenInfo &info, SymbolRef overloads, std::vector<ExpressionRef> &paramExpr, const Token::Position &searchedFrom)
{
	std::map<SymbolRef, int> candidates;

	addPossibleCandidates(info, candidates, overloads, paramExpr);

	if (candidates.empty())
		return nullptr;

	auto itBest = candidates.begin();
	for (auto itCurr = candidates.begin(); itCurr != candidates.end();)
	{
		if (itCurr == itBest)
		{
			++itCurr;
			continue;
		}

		if (itCurr->second < itBest->second)
		{
			candidates.erase(itBest, itCurr);
			itBest = itCurr;
		}
		else if (itCurr->second > itBest->second)
		{
			itCurr = candidates.erase(itCurr);
		}
		else
		{
			++itCurr;
		}
	}

	if (candidates.size() > 1)
	{
		auto paramStr = getReadableName(paramExpr);
		std::string candidatesStr;
		for (auto &[fSym, _] : candidates)
			candidatesStr += "  " + getPosStr(getBestPos(fSym)) + ": " + overloads->name + "(" + getReadableName(fSym->func.params, fSym->func.isVariadic) + ")\n";
		candidatesStr.pop_back();
		THROW_PROG_GEN_ERROR_POS(searchedFrom, "Ambiguous function call: " + overloads->name + "(" + paramStr + ")\nPossible candidates are:\n" + candidatesStr);
	}

	auto bestCandidate = itBest->first;

	if (bestCandidate->func.isBlueprint)
		bestCandidate = generateBlueprintSpecialization(info, bestCandidate, paramExpr, searchedFrom);

	for (int i = 0; i < paramExpr.size(); ++i)
	{
		if (!dtEqual(paramExpr[i]->datatype, bestCandidate->func.params[i]->var.datatype))
			paramExpr[i] = genConvertExpression(info, paramExpr[i], bestCandidate->func.params[i]->var.datatype, false, true, true);
	}

	return bestCandidate;
}

SymbolRef getVariable(ProgGenInfo &info, const std::string &name)
{
	auto symbol = getSymbol(currSym(info), name);
	if (!isVariable(symbol))
		return nullptr;

	return symbol;
}

SymbolRef getFunctions(ProgGenInfo &info, const std::string &name)
{
	auto symbol = getSymbol(currSym(info), name);
	if (!isFuncName(symbol))
		return nullptr;

	return symbol;
}

SymbolRef getFunctions(ProgGenInfo &info, const std::vector<std::string> &path)
{
	auto symbol = getSymbolFromPath(info.program->symbols, path);
	if (!isFuncName(symbol))
		return nullptr;

	return symbol;
}

SymbolRef getEnum(ProgGenInfo &info, const std::string &name)
{
	auto symbol = getSymbol(currSym(info), name);
	if (!isEnum(symbol))
		return nullptr;

	return symbol;
}

uint64_t getEnumValue(ProgGenInfo &info, const std::string &enumName, const std::string &memberName)
{
	auto enumSymbol = getEnum(info, enumName);
	if (!enumSymbol)
		THROW_QINP_ERROR("Enum '" + enumName + "' not found!");

	auto memberSymbol = getSymbol(enumSymbol, memberName, true);
	if (!memberSymbol)
		THROW_QINP_ERROR("Enum member '" + memberName + "' not found!");

	return memberSymbol->enumValue;
}

bool parseEmptyLine(ProgGenInfo &info)
{
	if (peekToken(info).type != Token::Type::Newline)
		return false;
	nextToken(info);
	return true;
}

void parseExpected(ProgGenInfo &info, Token::Type type)
{
	auto &token = nextToken(info);
	if (token.type != type)
		THROW_PROG_GEN_ERROR_TOKEN(token, "Expected token of type '" + TokenTypeToString(type) + "', but found '" + TokenTypeToString(token.type) + "'!");
}

void parseExpected(ProgGenInfo &info, Token::Type type, const std::string &value)
{
	auto &token = nextToken(info);
	if (token.type != type || token.value != value)
		THROW_PROG_GEN_ERROR_TOKEN(token, "Expected '" + value + "', but found '" + token.value + "'!");
}

void parseExpectedNewline(ProgGenInfo &info)
{
	auto &token = nextToken(info);
	if (!isNewline(token))
		THROW_PROG_GEN_ERROR_TOKEN(token, "Expected newline!");
}

bool parseOptionalNewline(ProgGenInfo &info)
{
	bool newlineFound = false;
	auto &token = peekToken(info);
	if ((newlineFound = (isNewline(token) || isEndOfCode(token))))
		nextToken(info);
	return newlineFound;
}

void parseExpectedColon(ProgGenInfo &info)
{
	parseExpected(info, Token::Type::Separator, ":");
}

void addVariable(ProgGenInfo &info, SymbolRef sym)
{
	auto &var = sym->var;
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
		THROW_PROG_GEN_ERROR_POS(sym->pos.decl, "Symbol with name '" + sym->name + "' already declared here: " + getPosStr(symbol->pos.decl));

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

SymbolRef addFunction(ProgGenInfo &info, SymbolRef func)
{
	auto funcs = getSymbol(currSym(info), func->name, true);
	if (funcs && !isFuncName(funcs))
		THROW_PROG_GEN_ERROR_POS(func->pos.decl, "Symbol with name '" + func->name + "' already declared here " + getPosStr(funcs->pos.decl));

	if (!funcs)
	{
		funcs = std::make_shared<Symbol>();
		funcs->name = func->name;
		funcs->type = SymType::FunctionName;

		addSymbol(currSym(info), funcs);
	}

	if (func->func.isBlueprint)
	{
		auto bpFuncs = getSymbol(funcs, BLUEPRINT_SYMBOL_NAME, true);

		if (!bpFuncs)
		{
			bpFuncs = std::make_shared<Symbol>();
			bpFuncs->name = BLUEPRINT_SYMBOL_NAME;
			bpFuncs->type = SymType::FunctionName;

			addSymbol(funcs, bpFuncs);
		}
		funcs = bpFuncs;
	}

	func->name = getSignatureNoRet(func);

	auto existingOverload = getSymbol(funcs, func->name, true);

	if (!existingOverload)
	{
		addSymbol(funcs, func);
		return func;
	}

	if (!dtEqual(existingOverload->func.retType, func->func.retType))
		THROW_PROG_GEN_ERROR_POS(getBestPos(func), "Function '" + getReadableName(existingOverload) + "' exists with different return type: '" + getPosStr(getBestPos(existingOverload)) + "'!");
	if (!isDefined(func))
		return existingOverload;
	if (isDefined(existingOverload))
		THROW_PROG_GEN_ERROR_POS(getBestPos(func), "Function '" + getReadableName(existingOverload) + "' already defined here: " + getPosStr(getBestPos(existingOverload)));

	return replaceSymbol(existingOverload, func);
}

SymbolRef addPack(ProgGenInfo &info, SymbolRef pack)
{
	auto existingPack = getSymbol(currSym(info), pack->name, true);

	if (!existingPack)
	{
		addSymbol(currSym(info), pack);
		return pack;
	}

	if (!isPack(existingPack))
		THROW_PROG_GEN_ERROR_POS(getBestPos(pack), "Symbol with name '" + pack->name + "' already declared here: " + getPosStr(existingPack->pos.decl));

	if (existingPack->pack.isUnion != pack->pack.isUnion)
		THROW_PROG_GEN_ERROR_POS(getBestPos(pack), std::string(existingPack->pack.isUnion ? "Union" : "Pack") + " with name '" + pack->name + "' already declared here: " + getPosStr(existingPack->pos.decl));

	if (!isDefined(pack))
		return existingPack;
	if (isDefined(existingPack))
		THROW_PROG_GEN_ERROR_POS(getBestPos(pack), "Pack already defined here: " + getPosStr(getBestPos(existingPack)));

	return replaceSymbol(existingPack, pack);
}

std::string preprocessAsmCode(ProgGenInfo &info, const Token &asmToken)
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
					THROW_PROG_GEN_ERROR_TOKEN(asmToken, "Expected '(' after '$'!");
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
				THROW_PROG_GEN_ERROR_TOKEN(asmToken, "Unknown variable '" + varName + "'!");

			if (isVarOffset(pVar))
			{
				int offset = pVar->var.offset;
				result += (offset < 0) ? "- " : "+ ";
				result += std::to_string(std::abs(offset));
			}
			else if (isMacro(pVar))
			{
				THROW_PROG_GEN_ERROR_TOKEN(asmToken, "Macros cannot be used in inline assembly!");
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
		THROW_PROG_GEN_ERROR_TOKEN(asmToken, "Missing ')'!");
	return result;
}

void increaseIndent(ProgGenInfo::Indent &indent)
{
	++indent.lvl;
}

bool parseIndent(ProgGenInfo &info, bool ignoreLeadingWhitespaces)
{
	if (info.indent.chStr.empty())
	{
		if (info.indent.lvl == 0)
		{
			if (isWhitespace(peekToken(info)))
				THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Unexpected indentation!");
			return true;
		}

		if (info.indent.lvl != 1)
			THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Indentation level must be 1 for the first used indentation!");

		auto &firstToken = peekToken(info);
		if (!isWhitespace(firstToken))
			return false;

		nextToken(info);
		info.indent.chStr = firstToken.value;
		info.indent.nPerLvl = 1;

		auto pIndentToken = &peekToken(info);

		while (isWhitespace(*pIndentToken))
		{
			if (pIndentToken->value != info.indent.chStr)
				THROW_PROG_GEN_ERROR_TOKEN(*pIndentToken, "Inconsistent indentation!");
			nextToken(info);
			++info.indent.nPerLvl;
			pIndentToken = &peekToken(info);
		}

		return true;
	}

	int indentCount = 0;
	for (int i = 0; i < info.indent.nPerLvl * info.indent.lvl; ++i)
	{
		auto &token = peekToken(info, indentCount);
		if (!isWhitespace(token) || token.value != info.indent.chStr)
			return false;
		++indentCount;
	}

	nextToken(info, indentCount);

	if (!ignoreLeadingWhitespaces && peekToken(info).type == Token::Type::Whitespace)
		THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Inconsistent indentation!");

	return true;
}

void unparseIndent(ProgGenInfo &info)
{
	nextToken(info, -info.indent.nPerLvl * info.indent.lvl);
}

void decreaseIndent(ProgGenInfo::Indent &indent)
{
	if (indent.lvl == 0)
		THROW_PROG_GEN_ERROR_POS(Token::Position(), "Indent level already 0!");
	--indent.lvl;
}

void pushTempBody(ProgGenInfo &info, BodyRef body)
{
	info.mainBodyBackups.push(info.program->body);
	info.program->body = body;
}

void popTempBody(ProgGenInfo &info)
{
	if (info.mainBodyBackups.empty())
		THROW_PROG_GEN_ERROR_POS(Token::Position(), "No backup body to pop!");

	info.program->body = info.mainBodyBackups.top();
	info.mainBodyBackups.pop();
}

Datatype getBestConvDatatype(const Datatype &left, const Datatype &right)
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
			"bool",
			"i8",
			"u8",
			"i16",
			"u16",
			"i32",
			"u32",
			"i64",
			"u64",
		};

	auto leftIt = std::find(typeOrder.begin(), typeOrder.end(), left.name);
	auto rightIt = std::find(typeOrder.begin(), typeOrder.end(), right.name);
	if (leftIt == typeOrder.end() || rightIt == typeOrder.end())
		return Datatype();

	return (leftIt > rightIt) ? left : right;
}

ExpressionRef makeConvertExpression(ExpressionRef expToConvert, const Datatype &newDatatype)
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

bool isConvPossible(ProgGenInfo &info, const Datatype &oldDt, const Datatype &newDt, bool isExplicit)
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

ExpressionRef genConvertExpression(ProgGenInfo &info, ExpressionRef expToConvert, const Datatype &newDatatype, bool isExplicit, bool doThrow, bool ignoreFirstConstness)
{
	expToConvert = genAutoArrayToPtr(expToConvert);

	if (dtEqual(expToConvert->datatype, newDatatype))
		return expToConvert;

	if (dtEqual(expToConvert->datatype, newDatatype, ignoreFirstConstness))
		return makeConvertExpression(expToConvert, newDatatype);

	if (isNull(expToConvert->datatype))
		return makeConvertExpression(expToConvert, newDatatype);

	if (isConvPossible(info, expToConvert->datatype, newDatatype, isExplicit))
	{
		if (
			isPointer(expToConvert->datatype) &&
			isPointer(newDatatype) &&
			!preservesConstness(expToConvert->datatype, newDatatype))
			PRINT_WARNING(
				MAKE_PROG_GEN_ERROR_POS(
					expToConvert->pos,
					"Conversion from '" + getReadableName(expToConvert->datatype) + "' to '" + getReadableName(newDatatype) + "' does not preserve the constness of the old type!"));
		return makeConvertExpression(expToConvert, newDatatype);
	}

	if (doThrow)
		THROW_PROG_GEN_ERROR_POS(expToConvert->pos, std::string("Cannot ") + (isExplicit ? "" : "implicitly ") + "convert from '" + getReadableName(expToConvert->datatype) + "' to '" + getReadableName(newDatatype) + "'!");

	return nullptr;
}

void autoFixDatatypeMismatch(ProgGenInfo &info, ExpressionRef exp)
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
			exp->right = genConvertExpression(info, exp->right, {"u64"}, true);
			{
				auto temp = std::make_shared<Expression>(exp->pos);
				temp->eType = Expression::ExprType::Product;
				temp->right = exp->right;
				temp->datatype = {"u64"};

				temp->left = makeLiteralExpression(exp->pos, {"u64"}, {(uint64_t)getDatatypePointedToSize(info.program, exp->left->datatype)});

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
		THROW_PROG_GEN_ERROR_POS(
			exp->pos, "Cannot convert " +
						  getReadableName(exp->left->datatype) + " and " +
						  getReadableName(exp->right->datatype) + " to a common datatype!");

	exp->left = genConvertExpression(info, exp->left, newDatatype);
	exp->right = genConvertExpression(info, exp->right, newDatatype);
}

ExpressionRef getParseEnumMember(ProgGenInfo &info)
{
	auto &enumToken = peekToken(info);
	if (!isIdentifier(enumToken))
		return nullptr;

	if (!getEnum(info, enumToken.value))
		return nullptr;

	nextToken(info);
	parseExpected(info, Token::Type::Operator, ".");

	auto &memberToken = nextToken(info);

	if (!isIdentifier(memberToken))
		THROW_PROG_GEN_ERROR_TOKEN(memberToken, "Expected enum member identifier!");

	uint64_t value;

	try
	{
		value = getEnumValue(info, enumToken.value, memberToken.value);
	}
	catch (const QinpError &e)
	{
		THROW_PROG_GEN_ERROR_TOKEN(memberToken, e.what());
	}

	return makeLiteralExpression(memberToken.pos, getMangledName(getEnum(info, enumToken.value)), EValue(value));
}

ExpressionRef getParseLiteral(ProgGenInfo &info)
{
	auto &litToken = peekToken(info);
	if (!isLiteral(litToken))
		return getParseEnumMember(info);

	nextToken(info);
	ExpressionRef exp = std::make_shared<Expression>(litToken.pos);

	exp->eType = Expression::ExprType::Literal;
	exp->isObject = true;

	switch (litToken.type)
	{
	case Token::Type::LiteralInteger:
		exp->datatype = {"u64"};
		exp->value = EValue((uint64_t)std::stoull(litToken.value));
		break;
	case Token::Type::LiteralChar:
		exp->datatype = {"u8"};
		exp->value = EValue((uint64_t)litToken.value[0]);
		break;
	case Token::Type::LiteralBoolean:
		exp->datatype = {"bool"};
		exp->value = EValue((uint64_t)(litToken.value == "true" ? 1 : 0));
		break;
	case Token::Type::LiteralNull:
		exp->datatype = {"null"};
		exp->value = EValue((uint64_t)0);
		break;
	case Token::Type::String:
	{
		int strID = info.program->strings.size();

		{
			auto it = info.program->strings.find(litToken.value);
			if (it == info.program->strings.end())
				info.program->strings.insert({litToken.value, strID});
			else
				strID = it->second;
		}

		exp->value = EValue((uint64_t)strID);
		exp->datatype = Datatype(DTType::Array, Datatype("u8"), litToken.value.size() + 1);
		exp->datatype.subType->isConst = true;
	}
	break;
	default:
		THROW_PROG_GEN_ERROR_TOKEN(litToken, "Invalid literal type!");
	}

	return exp;
}

ExpressionRef getParseSymbol(ProgGenInfo &info, bool localOnly)
{
	auto &symToken = peekToken(info);
	if (!isIdentifier(symToken))
		return nullptr;

	auto sym = getSymbol(currSym(info), symToken.value, localOnly);
	if (!sym)
		return nullptr;

	while (isAlias(sym))
		sym = sym->aliasedSymbol;

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

ExpressionRef getParseValue(ProgGenInfo &info, bool localOnly)
{
	ExpressionRef exp = nullptr;

	exp = getParseLiteral(info);
	if (exp)
		return exp;

	exp = getParseSymbol(info, localOnly);
	if (exp)
		return exp;

	THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Expected symbol name or literal value!");
}

ExpressionRef getParseParenthesized(ProgGenInfo &info)
{
	auto &parenOpen = peekToken(info);
	if (!isSeparator(parenOpen, "("))
		return getParseValue(info, false);

	nextToken(info);
	auto exp = getParseExpression(info);

	auto &parenClose = nextToken(info);
	if (!isSeparator(parenClose, ")"))
		THROW_PROG_GEN_ERROR_TOKEN(parenClose, "Expected ')'!");

	return exp;
}

ExpressionRef autoSimplifyExpression(ExpressionRef expr)
{
	if (expr->eType == Expression::ExprType::Literal)
		return expr;

	bool leftLiteral = false;
	bool rightLiteral = false;
	bool farRightLiteral = false;

	// Simplify subexpression before simplifying the current expression
	// Check every operand whether it is a literal or not
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
	// TODO: Handle Conversion
	// TODO: Masking

	if (leftLiteral)
	{
		if (rightLiteral) // leftLiteral == true && rightLiteral == true
		{
			switch (expr->eType)
			{
			case Expression::ExprType::Logical_OR:
				expr = makeLiteralExpression(expr->pos, {"bool"}, EValue(uint64_t(expr->left->value.u64 || expr->right->value.u64)));
				break;
			case Expression::ExprType::Logical_AND:
				expr = makeLiteralExpression(expr->pos, {"bool"}, EValue(uint64_t(expr->left->value.u64 && expr->right->value.u64)));
				break;
			case Expression::ExprType::Bitwise_OR:
				expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(expr->left->value.u64 | expr->right->value.u64)));
				break;
			case Expression::ExprType::Bitwise_XOR:
				expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(expr->left->value.u64 ^ expr->right->value.u64)));
				break;
			case Expression::ExprType::Bitwise_AND:
				expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(expr->left->value.u64 & expr->right->value.u64)));
				break;
			case Expression::ExprType::Comparison_Equal:
				expr = makeLiteralExpression(expr->pos, {"bool"}, EValue(uint64_t(expr->left->value.u64 == expr->right->value.u64)));
				break;
			case Expression::ExprType::Comparison_NotEqual:
				expr = makeLiteralExpression(expr->pos, {"bool"}, EValue(uint64_t(expr->left->value.u64 != expr->right->value.u64)));
				break;
			case Expression::ExprType::Comparison_Less:
				expr = makeLiteralExpression(expr->pos, {"bool"}, EValue(uint64_t(expr->left->value.u64 < expr->right->value.u64)));
				break;
			case Expression::ExprType::Comparison_LessEqual:
				expr = makeLiteralExpression(expr->pos, {"bool"}, EValue(uint64_t(expr->left->value.u64 <= expr->right->value.u64)));
				break;
			case Expression::ExprType::Comparison_Greater:
				expr = makeLiteralExpression(expr->pos, {"bool"}, EValue(uint64_t(expr->left->value.u64 > expr->right->value.u64)));
				break;
			case Expression::ExprType::Comparison_GreaterEqual:
				expr = makeLiteralExpression(expr->pos, {"bool"}, EValue(uint64_t(expr->left->value.u64 >= expr->right->value.u64)));
				break;
			case Expression::ExprType::Shift_Left:
				expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(expr->left->value.u64 << expr->right->value.u64)));
				break;
			case Expression::ExprType::Shift_Right:
				expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(expr->left->value.u64 >> expr->right->value.u64)));
				break;
			case Expression::ExprType::Sum:
				expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(expr->left->value.u64 + expr->right->value.u64)));
				break;
			case Expression::ExprType::Difference:
				expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(expr->left->value.u64 - expr->right->value.u64)));
				break;
			case Expression::ExprType::Product:
				expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(expr->left->value.u64 * expr->right->value.u64)));
				break;
			case Expression::ExprType::Quotient:
				expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(expr->left->value.u64 / expr->right->value.u64)));
				break;
			case Expression::ExprType::Remainder:
				expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(expr->left->value.u64 % expr->right->value.u64)));
				break;
			}
		}
		else // leftLiteral == true && rightLiteral == false
		{
			;
		}

		switch (expr->eType) // leftLiteral == true
		{
		case Expression::ExprType::Logical_NOT:
			expr = makeLiteralExpression(expr->pos, {"bool"}, EValue(uint64_t(!expr->left->value.u64)));
			break;
		case Expression::ExprType::Bitwise_NOT:
			expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(~expr->left->value.u64)));
			break;
		case Expression::ExprType::Prefix_Minus:
			expr = makeLiteralExpression(expr->pos, expr->datatype, EValue(uint64_t(-expr->left->value.u64)));
			break;
		case Expression::ExprType::Conditional_Op:
			expr = expr->left->value.u64 ? expr->right : expr->farRight;
			break;
		}
	}
	else
	{
		if (rightLiteral) // leftLiteral == false && rightLiteral == true
		{
			;
		}
		else // leftLiteral == false && rightLiteral == false
		{
			;
		}
	}

	switch (expr->eType)
	{
	case Expression::ExprType::Conversion:
		break;
	case Expression::ExprType::Prefix_Plus:
		break; // Nothing to do
	}

	return expr;
}

ExpressionRef getParseBinaryExpression(ProgGenInfo &info, int precLvl)
{
	auto &opsLvl = opPrecLvls[precLvl];

	ExpressionRef baseExpr = nullptr;
	auto currExpr = getParseExpression(info, precLvl + 1);

	const Token *pOpToken = nullptr;
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
				THROW_PROG_GEN_ERROR_POS(currExpr->left->pos, "Cannot assign to non-lvalue!");
			currExpr->datatype = currExpr->left->datatype;
			currExpr->isLValue = true;
			currExpr->isObject = true;
			break;
		case Expression::ExprType::Conditional_Op:
		{
			ENABLE_EXPR_ONLY_FOR_OBJ(currExpr->left);
			currExpr->left = genConvertExpression(info, currExpr->left, {"bool"});

			currExpr->right = getParseExpression(info, 0);
			ENABLE_EXPR_ONLY_FOR_OBJ(currExpr->right);

			parseExpectedColon(info);

			currExpr->farRight = getParseExpression(info, 0);
			ENABLE_EXPR_ONLY_FOR_OBJ(currExpr->farRight);

			currExpr->right = genAutoArrayToPtr(currExpr->right);
			currExpr->farRight = genAutoArrayToPtr(currExpr->farRight);

			if (!dtEqual(currExpr->right->datatype, currExpr->farRight->datatype))
				THROW_PROG_GEN_ERROR_POS(currExpr->pos, "Conditional operator operands must have the same datatype!");

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
			currExpr->left = genConvertExpression(info, currExpr->left, {"bool"});
			currExpr->right = genConvertExpression(info, currExpr->right, {"bool"});
			currExpr->datatype = {"bool"};
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
			currExpr->datatype = {"bool"};
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
		default:
			THROW_PROG_GEN_ERROR_POS(currExpr->pos, "Invalid binary operator!");
		}
	}

	return baseExpr ? baseExpr : currExpr;
}

ExpressionRef getParseUnarySuffixExpression(ProgGenInfo &info, int precLvl)
{
	auto exp = getParseExpression(info, precLvl + 1);

	auto &opsLvl = opPrecLvls[precLvl];

	const Token *pOpToken = nullptr;
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
				THROW_PROG_GEN_ERROR_POS(exp->pos, "Cannot subscript non-pointer type!");
			dereferenceDatatype(exp->datatype);
			if (!isDereferenceable(exp->datatype) && exp->datatype.name == "void")
				THROW_PROG_GEN_ERROR_POS(exp->pos, "Cannot subscript pointer to void type!");
			exp->isLValue = isArray(exp->datatype) ? false : true;
			exp->isObject = true;
			exp->right = getParseExpression(info, 0, {"u64"});
			ENABLE_EXPR_ONLY_FOR_OBJ(exp->right);
			parseExpected(info, Token::Type::Separator, "]");
			break;
		case Expression::ExprType::FunctionCall:
		{
			exp->isExtCall = isExtFunc(exp->left->symbol);

			if (exp->left->eType != Expression::ExprType::Symbol || (!isFuncName(exp->left->symbol) && !exp->isExtCall))
				THROW_PROG_GEN_ERROR_POS(exp->pos, "Cannot call non-function!");
			exp->isLValue = false;

			while (!isSeparator(peekToken(info), ")"))
			{
				if (isSeparator(peekToken(info), "..."))
				{
					if (info.bpVariadicParamIDStack.empty() || info.bpVariadicParamIDStack.top().empty())
						THROW_PROG_GEN_ERROR_POS(exp->pos, "Cannot use variadic expansion outside of variadic blueprint function!");
					nextToken(info);
					for (int id : info.bpVariadicParamIDStack.top())
						exp->paramExpr.push_back(makeSymbolExpression(exp->pos, getSymbol(currSym(info), variadicNameFromID(id))));
				}
				else
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
				func = getMatchingOverload(info, exp->left->symbol, exp->paramExpr, exp->pos);
				if (!func)
					THROW_PROG_GEN_ERROR_POS(exp->pos, "No matching overload found for function '" + getReadableName(exp->left->symbol) + "'! Provided parameters: (" + getReadableName(exp->paramExpr) + ")");
			}

			exp->datatype = func->func.retType;
			exp->left->datatype = Datatype(DTType::Pointer, Datatype(getSignature(func)));
			exp->left->symbol = func;
			exp->isObject = !isVoid(func->func.retType);

			info.program->body->usedFunctions.insert(getSymbolPath(nullptr, func));

			exp->paramSizeSum = 0;
			for (auto &param : exp->paramExpr)
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
		case Expression::ExprType::MemberAccessDereference:
		{
			exp->eType = Expression::ExprType::MemberAccess;
			if (!exp->left->isObject)
				THROW_PROG_GEN_ERROR_POS(exp->pos, "Expected object!");
			if (!isDereferenceable(exp->left->datatype))
				THROW_PROG_GEN_ERROR_POS(exp->pos, "Cannot dereference non-pointer!");
			exp->left = genAutoArrayToPtr(exp->left);

			auto temp = std::make_shared<Expression>(exp->pos);
			temp->eType = Expression::ExprType::Dereference;
			temp->left = exp->left;

			temp->datatype = temp->left->datatype;
			dereferenceDatatype(temp->datatype);
			if (isDereferenceable(temp->datatype))
				THROW_PROG_GEN_ERROR_POS(exp->pos, "Cannot access member of pointer to pointer!");
			temp->isLValue = isArray(temp->datatype) ? false : true;
			temp->isObject = true;

			exp->left = temp;
			// fallthrough
		}
		case Expression::ExprType::MemberAccess:
		{
			if (isPointer(exp->left->datatype))
				THROW_PROG_GEN_ERROR_POS(exp->pos, "Cannot access member of pointer!");
			if (isArray(exp->left->datatype))
				THROW_PROG_GEN_ERROR_POS(exp->pos, "Cannot access member of array!");

			auto baseSymbol = getSymbolFromPath(info.program->symbols, SymPathFromString(exp->left->datatype.name));
			if (!baseSymbol)
				baseSymbol = exp->left->symbol;

			assert(baseSymbol && "Unable to detect base symbol from left symbol and datatype!");

			enterSymbol(info, baseSymbol);
			exp->right = getParseSymbol(info, !!exp->left->symbol);
			exitSymbol(info);
			if (!exp->right)
				THROW_PROG_GEN_ERROR_POS(exp->pos, "Expected member name!");

			if (isPack(baseSymbol) && !isDefined(baseSymbol))
				THROW_PROG_GEN_ERROR_POS(exp->pos, "Cannot access member of undefined pack type!");
			if (isPack(baseSymbol) && !exp->left->isObject)
				THROW_PROG_GEN_ERROR_POS(exp->pos, "Expected object!");

			exp->datatype = exp->right->symbol->var.datatype;
			exp->isLValue = exp->right->isLValue;
			exp->isObject = exp->right->isObject;

			if (isPack(baseSymbol))
			{
				exp->isObject = exp->left->isObject;
				exp->isLValue = true;
			}

			if (!exp->left->isObject)
				exp = exp->right;
		}
		break;
		default:
			assert(false && "Unknown operator!");
		}
	}

	return exp;
}

ExpressionRef getParseUnaryPrefixExpression(ProgGenInfo &info, int precLvl)
{
	auto &opsLvl = opPrecLvls[precLvl];

	auto &opToken = peekToken(info);
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
			THROW_PROG_GEN_ERROR_POS(exp->pos, "Cannot take address of non-lvalue!");
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
			THROW_PROG_GEN_ERROR_POS(exp->pos, "Cannot dereference non-pointer type!");
		dereferenceDatatype(exp->datatype);
		if (!isDereferenceable(exp->datatype) && exp->datatype.name == "void")
			THROW_PROG_GEN_ERROR_POS(exp->pos, "Cannot dereference pointer to void type!");
		exp->isLValue = isArray(exp->datatype) ? false : true;
		exp->isObject = true;
		break;
	case Expression::ExprType::Logical_NOT:
		exp->left = getParseExpression(info, precLvl, {"bool"});
		ENABLE_EXPR_ONLY_FOR_OBJ(exp->left);
		exp->datatype = {"bool"};
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
		Datatype newDatatype = getParseDatatype(info, {});
		if (!newDatatype)
		{
			nextToken(info, -1);
			return getParseExpression(info, precLvl + 1);
		}
		parseExpected(info, Token::Type::Separator, ")");
		exp = getParseExpression(info, precLvl, newDatatype, true);
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
		exp->datatype = {"u64"};
		exp->value = EValue((uint64_t)getDatatypeSize(info.program, datatype));
	}
	break;
	case Expression::ExprType::MemberAccess:
		enterSymbol(info, info.program->symbols);
		exp = getParseExpression(info, precLvl + 1);
		exitSymbol(info);
		break;
	default:
		THROW_PROG_GEN_ERROR_TOKEN(opToken, "Unknown unary prefix expression!");
	}

	return exp;
}

ExpressionRef getParseExpression(ProgGenInfo &info, int precLvl)
{
	static const int maxPrecLvl = opPrecLvls.size() - 1;

	if (precLvl > maxPrecLvl)
		return getParseParenthesized(info);

	ExpressionRef expr = nullptr;
	switch (opPrecLvls[precLvl].type)
	{
	case OpPrecLvl::Type::Binary:
		expr = getParseBinaryExpression(info, precLvl);
		break;
	case OpPrecLvl::Type::Unary_Suffix:
		expr = getParseUnarySuffixExpression(info, precLvl);
		break;
	case OpPrecLvl::Type::Unary_Prefix:
		expr = getParseUnaryPrefixExpression(info, precLvl);
		break;
	default:
		THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Unknown operator precedence level type!");
	}

	return autoSimplifyExpression(expr);
}

ExpressionRef getParseExpression(ProgGenInfo &info, int precLvl, const Datatype &targetType, bool isExplicit, bool doThrow, bool ignoreFirstConstness)
{
	auto expr = getParseExpression(info, precLvl);
	if (!expr)
		return nullptr;

	if (!dtEqual(expr->datatype, targetType))
		expr = genConvertExpression(info, expr, targetType, isExplicit, doThrow, ignoreFirstConstness);

	return expr;
}

bool parseExpression(ProgGenInfo &info)
{
	auto expr = getParseExpression(info);
	if (!expr)
		return false;

	pushStatement(info, expr);
	parseExpectedNewline(info);
	return true;
}

bool parseExpression(ProgGenInfo &info, const Datatype &targetType)
{
	auto expr = getParseExpression(info, 0, targetType);
	if (!expr)
		return false;

	pushStatement(info, expr);
	return true;
}

Datatype getParseDatatype(ProgGenInfo &info, std::vector<Token> *pBlueprintMacroTokens)
{
	Datatype datatype;

	auto prevTokIt = info.currToken;

	auto exitEntered = [&](const Datatype &dt, bool resetTokIt)
	{
		if (resetTokIt)
			info.currToken = prevTokIt;
		return dt;
	};

	auto &dtBeginTok = peekToken(info);

	if (isOperator(dtBeginTok, "?"))
	{
		if (!pBlueprintMacroTokens)
			THROW_QINP_ERROR("Parsing blueprint macro but no macro list was provided!");

		nextToken(info);
		auto &tok = nextToken(info);
		if (!isIdentifier(tok))
			THROW_PROG_GEN_ERROR_TOKEN(tok, "Expected identifier!");

		if (
			std::find_if(
				pBlueprintMacroTokens->begin(), pBlueprintMacroTokens->end(),
				[&](const Token &tok)
				{ return tok.value == tok.value; }) == pBlueprintMacroTokens->end())
		{
			pBlueprintMacroTokens->push_back(tok);
		}

		datatype.type = DTType::Macro;
		datatype.name = tok.value;

		return exitEntered(datatype, false);
	}

	if (isBuiltinType(dtBeginTok))
	{
		datatype.name = nextToken(info).value;
		datatype.type = DTType::Name;
	}
	else
	{
		bool localOnly = false;
		SymbolRef sym = currSym(info);
		if (isOperator(peekToken(info), "."))
		{
			nextToken(info);
			sym = info.program->symbols;
			localOnly = true;
		}

		const Token *pNameToken = nullptr;
		do
		{
			pNameToken = &nextToken(info);
			if (!isIdentifier(*pNameToken))
				return exitEntered({}, true);
			sym = getSymbol(sym, pNameToken->value, localOnly);
			if (!sym)
				return exitEntered({}, true);
			localOnly = true;
		} while (isOperator(nextToken(info), "."));
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

std::pair<SymbolRef, ExpressionRef> getParseDeclDefVariable(ProgGenInfo &info)
{
	bool isStatic = isKeyword(peekToken(info), "static");

	if (isStatic && !isInFunction(currSym(info)))
		THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Static keyword can only be used inside of function definitions!");

	auto &varToken = peekToken(info, isStatic);
	if (!isKeyword(varToken, "var"))
		return {nullptr, nullptr};
	nextToken(info, 1 + isStatic);

	parseExpected(info, Token::Type::Operator, "<");
	auto datatype = getParseDatatype(info);
	if (isVoid(datatype))
		THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Cannot declare variable of type void!");
	parseExpected(info, Token::Type::Operator, ">");

	auto &nameToken = nextToken(info);
	if (!isIdentifier(nameToken))
		THROW_PROG_GEN_ERROR_TOKEN(nameToken, "Expected identifier!");
	auto &name = nameToken.value;

	SymbolRef sym = std::make_shared<Symbol>();
	sym->type = SymType::Variable;
	sym->pos.decl = peekToken(info).pos;
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

	if (isPackType(info.program, sym->var.datatype) && !isDefined(getSymbol(currSym(info), sym->var.datatype.name)))
		THROW_PROG_GEN_ERROR_POS(getBestPos(sym), "Cannot use declared-only pack type!");

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

	return {sym, initExpr};
}

bool parseDeclDefVariable(ProgGenInfo &info)
{
	auto [varSym, initExpr] = getParseDeclDefVariable(info);
	if (initExpr)
	{
		auto assignExpr = std::make_shared<Expression>(initExpr->pos);
		assignExpr->eType = Expression::ExprType::Assign;
		assignExpr->isLValue = true;
		assignExpr->isObject = true;
		assignExpr->datatype = varSym->var.datatype;
		assignExpr->left = makeSymbolExpression(varSym->pos.decl, varSym);
		assignExpr->right = genConvertExpression(info, initExpr, varSym->var.datatype);
		assignExpr->ignoreConstness = true;

		if (isVarContext(varSym, SymVarContext::Static))
			pushStaticLocalInit(info, assignExpr);
		else
			pushStatement(info, assignExpr);
	}

	return !!varSym;
}

bool parseDeclDefFunction(ProgGenInfo &info)
{
	// Check if it's a function declaration/definition

	auto &varToken = peekToken(info);
	if (!isKeyword(varToken, "fn"))
		return false;
	
	TokenList::iterator itFuncBegin = info.currToken;
	while (
		info.tokens->begin() != itFuncBegin &&
		isWhitespace(*std::prev(itFuncBegin)) &&
		!isNewline(*std::prev(itFuncBegin))
	)
		--itFuncBegin;

	nextToken(info);

	std::vector<Token> blueprintMacroTokens;

	// Parse the return type
	Datatype datatype("void");
	if (isOperator(peekToken(info), "<"))
	{
		parseExpected(info, Token::Type::Operator, "<");
		if (!isOperator(peekToken(info), ">")) // Makes 'fn<>' possible (same as 'fn' and 'fn<void>')
			datatype = getParseDatatype(info, &blueprintMacroTokens);
		parseExpected(info, Token::Type::Operator, ">");
	}

	// Parse the function name
	auto &nameToken = nextToken(info);
	if (!isIdentifier(nameToken))
		THROW_PROG_GEN_ERROR_TOKEN(nameToken, "Expected identifier!");
	auto &name = nameToken.value;

	if (!isSeparator(peekToken(info), "("))
		THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Missing parameter list for function declaration!");

	if (!isInGlobal(currSym(info)))
		THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Functions may only appear in global scope!");

	// ----- Previous Source -----

	auto declDefPos = peekToken(info).pos;

	SymbolRef funcSym = std::make_shared<Symbol>();

	funcSym->pos.decl = declDefPos;
	funcSym->type = SymType::FunctionSpec;
	funcSym->name = name;
	funcSym->func.body = std::make_shared<Body>();
	funcSym->func.retType = datatype;

	parseExpected(info, Token::Type::Separator, "(");

	while (!isSeparator(peekToken(info), ")"))
	{
		if (isSeparator(peekToken(info), "..."))
		{
			funcSym->func.isVariadic = true;
			nextToken(info);
			break;
		}

		SymbolRef paramSym = std::make_shared<Symbol>();
		paramSym->type = SymType::Variable;
		paramSym->pos.decl = peekToken(info).pos;
		paramSym->parent = funcSym;
		auto &param = paramSym->var;
		param.context = SymVarContext::Parameter;

		param.datatype = getParseDatatype(info, &blueprintMacroTokens);
		if (!param.datatype)
			THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Expected datatype!");

		param.offset = funcSym->func.retOffset;
		funcSym->func.retOffset += getDatatypePushSize(info.program, param.datatype);

		if (!isIdentifier(peekToken(info)))
			THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Expected identifier!");

		param.modName = nextToken(info).value;
		paramSym->name = param.modName;

		funcSym->func.params.push_back(paramSym);

		if (isSeparator(peekToken(info), ")"))
			continue;

		parseExpected(info, Token::Type::Separator, ",");
	}

	parseExpected(info, Token::Type::Separator, ")");

	if (!blueprintMacroTokens.empty() || funcSym->func.isVariadic)
		funcSym->func.isBlueprint = true;

	bool reqPreDecl = isOperator(peekToken(info), "!");
	if (reqPreDecl)
		nextToken(info);

	auto preDeclSym = getSymbol(currSym(info), name, true);
	if (preDeclSym && funcSym->func.isBlueprint)
		preDeclSym = getSymbol(preDeclSym, BLUEPRINT_SYMBOL_NAME, true);
	if (preDeclSym)
		preDeclSym = getSymbol(preDeclSym, getSignatureNoRet(funcSym), true);
	if (reqPreDecl && !preDeclSym)
		THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Missing pre-declaration before explicit function definition!");
	else if (!reqPreDecl && preDeclSym && !preDeclSym->func.genFromBlueprint)
		PRINT_WARNING(MAKE_PROG_GEN_ERROR_TOKEN(peekToken(info), "Function '" + name + "' was pre-declared but not marked as such. You may want to do so."));

	funcSym->state = SymState::Defined;
	if (isSeparator(peekToken(info), "..."))
	{
		funcSym->state = SymState::Declared;

		nextToken(info);
		parseExpectedNewline(info);
	}

	if (isDefined(funcSym))
		funcSym->pos.def = declDefPos;

	funcSym = addFunction(info, funcSym);

	if (isDefined(funcSym))
		funcSym->pos.def = declDefPos;

	if (funcSym->func.isBlueprint)
	{
		funcSym->func.blueprintTokens = std::make_shared<TokenList>();

		if (isDefined(funcSym))
		{
			parseExpectedColon(info);
			bool doParseIndent = parseOptionalNewline(info);

			increaseIndent(info.indent);

			while (!doParseIndent || parseIndent(info, true))
			{
				doParseIndent = true;

				while (!isNewline(peekToken(info)))
					nextToken(info);
				parseExpectedNewline(info);
			}

			decreaseIndent(info.indent);
		}

		*funcSym->func.blueprintTokens = TokenList(itFuncBegin, info.currToken);
		funcSym->func.blueprintTokens->push_back(makeToken(Token::Type::EndOfCode, "<end-of-code>"));
	}
	else if (isDefined(funcSym))
	{
		parseExpectedColon(info);
		bool parsedNewline = parseOptionalNewline(info);

		increaseIndent(info.indent);
		pushTempBody(info, funcSym->func.body);
		enterSymbol(info, funcSym);

		info.funcRetOffset = funcSym->func.retOffset;
		info.funcRetType = funcSym->func.retType;
		info.funcFrameSize = 0;

		for (auto &param : funcSym->func.params)
			addSymbol(funcSym, param);

		parseFunctionBody(info, parsedNewline);
		funcSym->frame.size = info.funcFrameSize;

		exitSymbol(info);
		popTempBody(info);
		decreaseIndent(info.indent);
	}

	return true;
}

bool parseDeclDef(ProgGenInfo &info)
{
	if (parseDeclDefVariable(info))
		return true;
	if (parseDeclDefFunction(info))
		return true;
	return false;

	// ----------------------
}

bool parseDeclExtFunc(ProgGenInfo &info)
{
	auto &extToken = peekToken(info);

	if (!isKeyword(extToken, "extern"))
		return false;
	nextToken(info);

	auto funcSym = std::make_shared<Symbol>();

	funcSym->pos.decl = extToken.pos;
	funcSym->type = SymType::ExtFunc;

	auto &typeToken = peekToken(info);
	funcSym->func.retType = getParseDatatype(info);

	if (!funcSym->func.retType)
		THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Expected datatype!");

	auto &nameToken = nextToken(info);
	if (!isIdentifier(nameToken))
		THROW_PROG_GEN_ERROR_TOKEN(nameToken, "Expected identifier!");
	funcSym->name = nameToken.value;

	parseExpected(info, Token::Type::Separator, "(");

	while (!isSeparator(peekToken(info), ")"))
	{
		SymbolRef paramSym = std::make_shared<Symbol>();
		paramSym->type = SymType::Variable;
		paramSym->pos.decl = peekToken(info).pos;
		paramSym->parent = funcSym;
		auto &param = paramSym->var;
		param.context = SymVarContext::Parameter;

		param.datatype = getParseDatatype(info);
		if (!param.datatype)
			THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Expected datatype!");

		param.offset = funcSym->func.retOffset;
		funcSym->func.retOffset += getDatatypePushSize(info.program, param.datatype);

		if (!isIdentifier(peekToken(info)))
			THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Expected identifier!");

		param.modName = nextToken(info).value;
		paramSym->name = param.modName;

		funcSym->func.params.push_back(paramSym);

		if (isSeparator(peekToken(info), ")"))
			continue;

		parseExpected(info, Token::Type::Separator, ",");
	}

	parseExpected(info, Token::Type::Separator, ")");

	parseExpected(info, Token::Type::Operator, "=");

	auto &asmNameTok = nextToken(info);

	if (!isString(asmNameTok))
		THROW_PROG_GEN_ERROR_TOKEN(asmNameTok, "Expected assembly name!");
	funcSym->func.asmName = asmNameTok.value;

	parseExpectedNewline(info);

	addSymbol(currSym(info), funcSym);

	return true;
}

bool parseStatementReturn(ProgGenInfo &info)
{
	auto &retToken = peekToken(info);
	if (!isKeyword(retToken, "return"))
		return false;

	nextToken(info);
	auto &exprBegin = peekToken(info);

	pushStatement(info, std::make_shared<Statement>(retToken.pos, Statement::Type::Return));
	lastStatement(info)->funcRetOffset = info.funcRetOffset;

	if (!isVoid(info.funcRetType))
		lastStatement(info)->subExpr = getParseExpression(info, 0, info.funcRetType);
	else if (!isNewline(peekToken(info)))
		THROW_PROG_GEN_ERROR_TOKEN(exprBegin, "Return statement in void function must be followed by a newline!");
	parseExpectedNewline(info);

	return true;
}

SymbolRef makeAlias(const std::string &name, Token::Position pos, SymbolRef sym)
{
	auto aliasSym = std::make_shared<Symbol>();
	aliasSym->pos.decl = pos;
	aliasSym->name = name;
	aliasSym->type = SymType::Alias;
	aliasSym->aliasedSymbol = sym;

	return aliasSym;
}

bool parseStatementAlias(ProgGenInfo &info)
{
	auto &aliasToken = peekToken(info);
	if (!isKeyword(aliasToken, "alias"))
		return false;

	nextToken(info);
	auto &nameToken = nextToken(info);
	if (!isIdentifier(nameToken))
		THROW_PROG_GEN_ERROR_TOKEN(nameToken, "Expected identifier!");

	parseExpected(info, Token::Type::Operator, "=");

	auto curr = currSym(info);
	bool localOnly = false;
	if (isSeparator(peekToken(info), "."))
	{
		nextToken(info);
		curr = info.program->symbols;
		localOnly = true;
	}

	while (isIdentifier(peekToken(info)))
	{
		curr = getSymbol(curr, peekToken(info).value, localOnly);

		if (!curr)
			THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Symbol '" + peekToken(info).value + "' not found!");

		nextToken(info);

		if (!isNewline(peekToken(info)))
			parseExpected(info, Token::Type::Operator, ".");

		localOnly = true;
	}

	parseExpectedNewline(info);

	addSymbol(currSym(info), makeAlias(nameToken.value, nameToken.pos, curr));

	return true;
}

bool parseStatementDefer(ProgGenInfo &info)
{
	auto &deferToken = peekToken(info);
	if (!isKeyword(deferToken, "defer"))
		return false;

	nextToken(info);
	parseExpectedNewline(info);

	if (currSym(info) != info.program->symbols)
		THROW_PROG_GEN_ERROR_TOKEN(deferToken, "Defer statement must be in global scope!");

	info.currToken = info.tokens->insert(info.currToken, makeToken(Token::Type::EndOfCode, "<defer>"));

	info.deferredCompilations.push(makeProgGenInfoBackup(info));

	return true;
}

bool parseInlineAssembly(ProgGenInfo &info)
{
	auto &asmToken = peekToken(info);
	if (!isKeyword(asmToken, "asm") && !isKeyword(asmToken, "assembly"))
		return false;
	nextToken(info);

	parseExpectedColon(info);
	bool doParseIndent = parseOptionalNewline(info);

	increaseIndent(info.indent);

	while (!doParseIndent || parseIndent(info))
	{
		doParseIndent = true;
		auto &strToken = nextToken(info);
		if (!isString(strToken))
			THROW_PROG_GEN_ERROR_TOKEN(strToken, "Expected assembly string!");

		parseExpectedNewline(info);

		pushStatement(info, std::make_shared<Statement>(asmToken.pos, Statement::Type::Assembly));
		lastStatement(info)->asmLines.push_back(preprocessAsmCode(info, strToken));
	}

	decreaseIndent(info.indent);

	return true;
}

bool parseStatementPass(ProgGenInfo &info)
{
	auto &passToken = peekToken(info);
	if (!isKeyword(passToken, "pass"))
		return false;

	nextToken(info);
	parseExpectedNewline(info);

	return true;
}

bool parseStatementContinue(ProgGenInfo &info)
{
	auto &contToken = peekToken(info);
	if (!isKeyword(contToken, "continue"))
		return false;
	nextToken(info);
	parseExpectedNewline(info);

	pushStatement(info, std::make_shared<Statement>(contToken.pos, Statement::Type::Continue));

	return true;
}

bool parseStatementBreak(ProgGenInfo &info)
{
	auto &breakToken = peekToken(info);
	if (!isKeyword(breakToken, "break"))
		return false;
	nextToken(info);

	parseExpectedNewline(info);

	pushStatement(info, std::make_shared<Statement>(breakToken.pos, Statement::Type::Break));

	return true;
}

void parseBody(ProgGenInfo &info, bool doParseIndent)
{
	int numStatements = 0;

	auto &bodyBeginToken = peekToken(info, -1);

	while (!doParseIndent || parseIndent(info))
	{
		doParseIndent = true;
		++numStatements;
		auto &token = peekToken(info);
		if (parseEmptyLine(info))
			continue;
		if (parseStatementAlias(info))
			continue;
		if (parseStatementPass(info))
			continue;
		if (parseControlFlow(info))
			continue;
		if (parseStatementContinue(info))
			continue;
		if (parseStatementBreak(info))
			continue;
		if (parseDeclDef(info))
			continue;
		if (parseStatementReturn(info))
			continue;
		if (parseInlineAssembly(info))
			continue;
		if (parseExpression(info))
			continue;
		THROW_PROG_GEN_ERROR_TOKEN(token, "Unexpected token: " + token.value + "!");
	}

	if (numStatements == 0)
		THROW_PROG_GEN_ERROR_TOKEN(bodyBeginToken, "Expected non-empty body!");
}

void parseBodyEx(ProgGenInfo &info, BodyRef body, bool doParseIndent)
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

bool parseStatementIf(ProgGenInfo &info)
{
	auto &ifToken = peekToken(info);
	if (!isKeyword(ifToken, "if"))
		return false;

	auto statement = std::make_shared<Statement>(ifToken.pos, Statement::Type::If_Clause);

	bool parsedIndent = true;

	do
	{
		nextToken(info);

		statement->ifConditionalBodies.push_back({});
		auto &condBody = statement->ifConditionalBodies.back();
		condBody.condition = getParseExpression(info, 0, {"bool"});
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
		auto &elseToken = peekToken(info);
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

bool parseStatementWhile(ProgGenInfo &info)
{
	auto &whileToken = peekToken(info);
	if (!isKeyword(whileToken, "while"))
		return false;
	nextToken(info);

	auto statement = std::make_shared<Statement>(whileToken.pos, Statement::Type::While_Loop);
	statement->whileConditionalBody.body = std::make_shared<Body>();

	statement->whileConditionalBody.condition = getParseExpression(info, 0, {"bool"});

	parseExpectedColon(info);
	bool parsedNewline = parseOptionalNewline(info);

	parseBodyEx(info, statement->whileConditionalBody.body, parsedNewline);

	pushStatement(info, statement);

	return true;
}

bool parseStatementDoWhile(ProgGenInfo &info)
{
	auto &doToken = peekToken(info);
	if (!isKeyword(doToken, "do"))
		return false;
	nextToken(info);

	auto statement = std::make_shared<Statement>(doToken.pos, Statement::Type::Do_While_Loop);
	statement->doWhileConditionalBody.body = std::make_shared<Body>();

	parseExpectedColon(info);
	bool parsedNewline = parseOptionalNewline(info);

	parseBodyEx(info, statement->doWhileConditionalBody.body, parsedNewline);

	if (!parseIndent(info))
		THROW_PROG_GEN_ERROR_TOKEN(doToken, "Expected 'while'!");
	parseExpected(info, Token::Type::Keyword, "while");

	statement->doWhileConditionalBody.condition = getParseExpression(info, 0, {"bool"});

	parseExpectedNewline(info);

	pushStatement(info, statement);

	return true;
}

bool parseControlFlow(ProgGenInfo &info)
{
	if (parseStatementIf(info))
		return true;
	if (parseStatementWhile(info))
		return true;
	if (parseStatementDoWhile(info))
		return true;

	return false;
}

void parseFunctionBody(ProgGenInfo &info, bool doParseIndent)
{
	auto &bodyBeginToken = peekToken(info, -1);

	parseBody(info, doParseIndent);

	if (info.program->body->statements.empty() || lastStatement(info)->type != Statement::Type::Return)
	{
		if (isVoid(info.funcRetType))
			pushStatement(info, std::make_shared<Statement>(Token::Position(), Statement::Type::Return));
		else
			THROW_PROG_GEN_ERROR_POS(info.program->body->statements.empty() ? bodyBeginToken.pos : lastStatement(info)->pos, "Missing return statement!");
	}
}

bool parseStatementDefine(ProgGenInfo &info)
{
	auto &defToken = peekToken(info, 0, true);
	if (!isKeyword(defToken, "define"))
		return false;
	nextToken(info, 1, true);

	auto &nameToken = nextToken(info, 1, true);

	if (!isIdentifier(nameToken))
		THROW_PROG_GEN_ERROR_TOKEN(nameToken, "Expected identifier!");

	auto sym = makeMacroSymbol(nameToken.pos, nameToken.value);

	if (isSeparator(peekToken(info, 0, true), "(") && peekToken(info, 0, true).pos.column - nameToken.pos.column == nameToken.value.size())
	{
		nextToken(info, 1, true);
		sym->macroIsFunctionLike = true;

		while (!isSeparator(peekToken(info, 0, true), ")"))
		{
			auto &argToken = nextToken(info, 1, true);
			if (!isIdentifier(argToken))
				THROW_PROG_GEN_ERROR_TOKEN(argToken, "Expected identifier!");
			sym->macroParamNames.push_back(argToken.value);
			if (isSeparator(peekToken(info, 0, true), ","))
				nextToken(info);
		}

		nextToken(info, 1, true);
	}

	auto existingSymbol = getSymbol(currSym(info), sym->name, true);
	if (existingSymbol)
		THROW_PROG_GEN_ERROR_TOKEN(nameToken, "Symbol '" + sym->name + "' already exists in the same scope!");

	while (!isNewline(peekToken(info, 0, true)))
		sym->macroTokens.push_back(nextToken(info, 1, true));

	parseExpectedNewline(info);

	addSymbol(currSym(info), sym);

	return true;
}

bool parseStatementSpace(ProgGenInfo &info)
{
	auto &spaceToken = peekToken(info);
	if (!isKeyword(spaceToken, "space"))
		return false;
	nextToken(info);

	auto &nameToken = peekToken(info);
	nextToken(info);

	if (!isIdentifier(nameToken))
		THROW_PROG_GEN_ERROR_TOKEN(nameToken, "Expected identifier!");

	parseExpectedColon(info);
	bool doParseIndent = parseOptionalNewline(info);

	auto spaceSym = getSymbol(currSym(info), nameToken.value, true);
	if (!spaceSym)
	{
		spaceSym = std::make_shared<Symbol>();
		spaceSym->type = SymType::Namespace;
		spaceSym->pos.decl = nameToken.pos;
		spaceSym->name = nameToken.value;
		addSymbol(currSym(info), spaceSym);
	}

	increaseIndent(info.indent);
	enterSymbol(info, spaceSym);

	int codeCount = 0;

	while (!doParseIndent || parseIndent(info))
	{
		doParseIndent = true;
		auto &token = peekToken(info);
		if (!parseSingleGlobalCode(info))
			THROW_PROG_GEN_ERROR_TOKEN(token, "Unexpected token: " + token.value + "!");
		++codeCount;
	}

	if (codeCount == 0)
		THROW_PROG_GEN_ERROR_TOKEN(spaceToken, "Expected at least one statement!");

	exitSymbol(info);
	decreaseIndent(info.indent);

	return true;
}

bool parsePackUnion(ProgGenInfo &info)
{
	auto &packToken = peekToken(info);
	if (!isKeyword(packToken, "pack") && !isKeyword(packToken, "union"))
		return false;
	nextToken(info);

	auto &nameToken = peekToken(info);
	if (!isIdentifier(nameToken))
		THROW_PROG_GEN_ERROR_TOKEN(nameToken, "Expected identifier!");
	nextToken(info);

	SymbolRef packSym = std::make_shared<Symbol>();
	packSym->type = SymType::Pack;
	packSym->pos.decl = nameToken.pos;
	packSym->name = nameToken.value;
	packSym->pack.isUnion = isKeyword(packToken, "union");
	auto &pack = packSym->pack;

	bool reqPreDecl = isOperator(peekToken(info), "!");
	if (reqPreDecl)
		nextToken(info);

	auto preDeclSym = getSymbol(currSym(info), nameToken.value, true);
	if (reqPreDecl && !preDeclSym)
		THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Missing pre-declaration before explicit pack/union definition!");
	else if (!reqPreDecl && preDeclSym)
		PRINT_WARNING(MAKE_PROG_GEN_ERROR_TOKEN(peekToken(info), "Pack/Union: '" + nameToken.value + "' was pre-declared but not marked as such. You may want to do so."));

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
		THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Expected '...' or ':'!");

	if (isDefined(packSym))
		packSym->pos.def = nameToken.pos;

	increaseIndent(info.indent);
	enterSymbol(info, packSym);

	while (!doParseIndent || parseIndent(info))
	{
		doParseIndent = true;
		if (parseStatementPass(info))
			continue;
		if (!parseDeclDef(info))
			THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Expected member definition!");
	}

	exitSymbol(info);
	decreaseIndent(info.indent);

	packSym->state = SymState::Defined;

	return true;
}

bool parseEnum(ProgGenInfo &info)
{
	auto &enumToken = peekToken(info);
	if (!isKeyword(enumToken, "enum"))
		return false;
	nextToken(info);

	auto &nameToken = peekToken(info);
	if (!isIdentifier(nameToken))
		THROW_PROG_GEN_ERROR_TOKEN(nameToken, "Expected identifier!");
	nextToken(info);

	if (getSymbol(currSym(info), nameToken.value, true))
		THROW_PROG_GEN_ERROR_TOKEN(nameToken, "Symbol with name '" + nameToken.value + "' already exists!");

	parseExpectedColon(info);
	bool doParseIndent = parseOptionalNewline(info);

	auto enumSym = std::make_shared<Symbol>();
	enumSym->pos.decl = nameToken.pos;
	enumSym->name = nameToken.value;
	enumSym->type = SymType::Enum;
	enumSym->state = SymState::Defined;

	addSymbol(currSym(info), enumSym);

	int64_t currIndex = 0;
	increaseIndent(info.indent);
	while (!doParseIndent || parseIndent(info))
	{
		doParseIndent = true;
		while (isIdentifier(peekToken(info)))
		{
			auto &memberToken = nextToken(info);

			if (getSymbol(enumSym, memberToken.value, true) != nullptr)
				THROW_PROG_GEN_ERROR_TOKEN(memberToken, "Enum member '" + memberToken.value + "' already exists!");

			if (isOperator(peekToken(info), "="))
			{
				nextToken(info);

				auto valExpr = getParseExpression(info);
				if (valExpr->eType != Expression::ExprType::Literal)
					THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Expected literal expression!");
				if (!isInteger(valExpr->datatype))
					THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Expected integer literal!");

				currIndex = valExpr->value.u64;
			}

			auto memSym = std::make_shared<Symbol>();
			memSym->pos.decl = memberToken.pos;
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

void parseGlobalCode(ProgGenInfo &info, bool fromBeginning)
{
	if (fromBeginning)
		info.currToken = info.tokens->begin();

	while (!isEndOfCode(peekToken(info)))
	{
		if (!parseIndent(info))
			THROW_PROG_GEN_ERROR_TOKEN(peekToken(info), "Expected indentation!");
		auto &token = peekToken(info);
		if (!parseSingleGlobalCode(info))
			THROW_PROG_GEN_ERROR_TOKEN(token, "Unexpected token: " + token.value + "!");
	}
}

void importFile(ProgGenInfo &info, const Token &fileToken)
{
	auto getImportFile = [&](const std::string &dir) -> std::string
	{
		std::string absPath;
		try
		{
			absPath = std::filesystem::canonical(std::filesystem::path(dir) / fileToken.value).string();
		}
		catch (std::filesystem::filesystem_error &)
		{
			return "<notFound>";
		}

		if (info.imports.find(absPath) != info.imports.end())
			return "<alreadyImported>";

		if (std::filesystem::is_regular_file(absPath))
		{
			info.imports.insert(absPath);
			return absPath;
		}
		return "<notFound>";
	};

	// Seach for a matching file relative to the current file's directory
	std::string path = getImportFile(std::filesystem::path(info.progPath).parent_path().string());

	if (path == "<alreadyImported>")
		return;

	if (path == "<notFound>")
	{
		// Search for a matching file in the specified import directories
		for (auto &dir : info.importDirs)
		{
			path = getImportFile(dir);

			if (path == "<alreadyImported>")
				return;

			if (path != "<notFound>")
				break;
		}

		if (path == "<notFound>")
			THROW_PROG_GEN_ERROR_TOKEN(fileToken, "Import file not found: '" + fileToken.value + "'!");
	}

	std::string code = readTextFile(path);

	parseInlineTokens(
		info,
		tokenize(code, std::filesystem::relative(path, std::filesystem::current_path()).string()),
		path);
}

bool parseStatementImport(ProgGenInfo &info)
{
	auto &importToken = peekToken(info);
	if (!isKeyword(importToken, "import"))
		return false;
	nextToken(info);

	bool platformMatch = true;
	bool isDeferred = false;

	while (isOperator(peekToken(info), "."))
	{
		nextToken(info);
		auto &specToken = nextToken(info);

		if (isKeyword(specToken, "defer"))
		{
			isDeferred = true;
			continue;
		}

		if (!isIdentifier(specToken))
			THROW_PROG_GEN_ERROR_TOKEN(specToken, "Expected platform identifier!");

		static const std::set<std::string> platformNames = {"windows", "linux", "macos"};
		if (platformNames.find(specToken.value) == platformNames.end())
			THROW_PROG_GEN_ERROR_TOKEN(specToken, "Unknown platform identifier: '" + specToken.value + "'!");
		if (specToken.value != info.program->platform)
		{
			platformMatch = false;
			continue;
		}
	}

	auto &fileToken = nextToken(info);
	if (!isString(fileToken))
		THROW_PROG_GEN_ERROR_TOKEN(fileToken, "Expected file path!");

	if (!platformMatch)
		return true;

	if (isDeferred)
		info.deferredImports.push_back(fileToken);
	else
		importFile(info, fileToken);

	return true;
}

bool parseSingleGlobalCode(ProgGenInfo &info)
{
	if (parseEmptyLine(info))
		return true;
	if (parseStatementAlias(info))
		return true;
	if (parseStatementDefer(info))
		return true;
	if (parseStatementPass(info))
		return true;
	if (parseStatementImport(info))
		return true;
	if (parseStatementDefine(info))
		return true;
	if (parseStatementSpace(info))
		return true;
	if (parseControlFlow(info))
		return true;
	if (parseStatementContinue(info))
		return true;
	if (parseStatementBreak(info))
		return true;
	if (parsePackUnion(info))
		return true;
	if (parseEnum(info))
		return true;
	if (parseDeclDef(info))
		return true;
	if (parseDeclExtFunc(info))
		return true;
	if (parseInlineAssembly(info))
		return true;
	if (parseInlineAssembly(info))
		return true;
	if (parseExpression(info))
		return true;
	return false;
}

void markReachableFunctions(ProgGenInfo &info, BodyRef body)
{
	for (auto &funcPath : body->usedFunctions)
	{
		auto func = getSymbolFromPath(info.program->symbols, funcPath);

		if (func->func.isReachable)
			continue;

		func->func.isReachable = true;
		if (!isExtFunc(func))
			markReachableFunctions(info, func->func.body);
	}
}

void detectUndefinedFunctions(ProgGenInfo &info)
{
	for (auto sym : *info.program->symbols)
	{
		if (isFuncSpec(sym) && !isDefined(sym) && isReachable(sym))
			THROW_PROG_GEN_ERROR_POS(sym->pos.decl, "Cannot reference undefined function '" + getReadableName(sym) + "'!");
	}
}

void importDeferredImports(ProgGenInfo &info)
{
	for (int i = 0; i < info.deferredImports.size(); ++i)
		importFile(info, info.deferredImports[i]);
}

void parseDeferredCompilations(ProgGenInfo &info)
{
	while (!info.deferredCompilations.empty())
	{
		loadProgGenInfoBackup(info, info.deferredCompilations.front());
		++info.currToken;
		parseGlobalCode(info, false);
		info.deferredCompilations.pop();
	}
}

void genDeclaredOnlyBpSpecs(ProgGenInfo &info)
{
	for (auto &spec : info.bpSpecsToDefine)
	{
		if (!isDefined(spec.bpSym))
			THROW_PROG_GEN_ERROR_POS(spec.generatedFrom, "Cannot generate blueprint specialization of undefined blueprint '" + getReadableName(spec.bpSym) + "'!");

		generateBlueprintSpecialization(info, spec.bpSym, spec.paramExpr, spec.generatedFrom);
	}
}

ProgramRef generateProgram(const TokenListRef tokens, const std::set<std::string> &importDirs, const std::string &platform, const std::string &progPath)
{
	ProgGenInfo info = {tokens, ProgramRef(new Program()), importDirs, progPath};
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

	parseDeferredCompilations(info);

	genDeclaredOnlyBpSpecs(info);

	markReachableFunctions(info, info.program->body);
	detectUndefinedFunctions(info);

	return info.program;
}
