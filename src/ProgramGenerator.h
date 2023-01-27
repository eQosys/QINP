#pragma once

#include <set>
#include <queue>

#include "Program.h"

ProgramRef generateProgram(const TokenListRef tokens, TokenListRef comments, const std::set<std::string>& importDirs, const std::string& platform, const std::string& progPath);

struct LocalStackFrame
{
	std::map<std::string, Symbol::Variable> locals;
	int totalOffset = 0; // The offset of the last local variable pushed onto the stack.
	int frameSize = 0; // The size of the stack frame. (Sum of the push sizes of all locals)
};

typedef std::map<std::string, SymbolRef> BlueprintMacroMap; // Name -> macro

struct BpSpecToDefine
{
	SymbolRef bpSym;
	std::vector<ExpressionRef> paramExpr;
	std::vector<TokenListRef> bpExplicitMacros;
	Token::Position generatedFrom;
};

struct ProgGenInfoBackup
{
	TokenListRef tokens;
	std::string progPath;
	TokenList::iterator currToken;
	int indentLvl;
	int funcRetOffset;
	int funcFrameSize;
	Datatype funcRetType;
};

struct ProgGenInfo
{
	TokenListRef tokens;
	TokenListRef comments;
	ProgramRef program;
	std::set<std::string> importDirs;
	std::string progPath;
	TokenList::iterator currToken;

	int indentLvl = 0;

	std::stack<BodyRef> mainBodyBackups;
	int funcRetOffset;
	int funcFrameSize;
	Datatype funcRetType;

	std::set<std::string> imports;

	std::vector<Token> deferredImports;
	std::queue<ProgGenInfoBackup> deferredCompilations;

	std::stack<std::vector<int>> bpVariadicParamIDStack;
	std::vector<BpSpecToDefine> bpSpecsToDefine;

	int continueEnableCount = 0;
	int breakEnableCount = 0;
};


ProgGenInfoBackup makeProgGenInfoBackup(const ProgGenInfo& info);
void loadProgGenInfoBackup(ProgGenInfo& info, const ProgGenInfoBackup& backup);

SymbolRef currSym(const ProgGenInfo& info);

void enterSymbol(ProgGenInfo& info, SymbolRef symbol);
void exitSymbol(ProgGenInfo& info);

SymbolRef addShadowSpace(ProgGenInfo& info);

TokenList::iterator moveTokenIterator(TokenList& list, TokenList::iterator it, int offset);

TokenListRef DatatypeToTokenList(const Datatype& datatype);

Token& kwFileToTokString(Token& tok);
Token& kwLineToTokInteger(Token& tok);

const Token& peekToken(ProgGenInfo& info, int offset = 0, bool ignoreSymDef = false);
const Token& nextToken(ProgGenInfo& info, int offset = 1, bool ignoreSymDef = false);

void enableContinue(ProgGenInfo& info);
void disableContinue(ProgGenInfo& info);
bool isContinueEnabled(const ProgGenInfo& info);

void enableBreak(ProgGenInfo& info);
void disableBreak(ProgGenInfo& info);
bool isBreakEnabled(const ProgGenInfo& info);

void mergeSubUsages(BodyRef parent, BodyRef child);

void pushStatement(ProgGenInfo& info, StatementRef statement);

ExpressionRef makeSymbolExpression(Token::Position pos, SymbolRef symbol);

ExpressionRef makeLiteralExpression(Token::Position pos, const Datatype& datatype, EValue value);

void pushStaticLocalInit(ProgGenInfo& info, ExpressionRef initExpr);

StatementRef lastStatement(ProgGenInfo& info);

bool isPackType(ProgGenInfo& info, const std::string& name);
bool isPackType(ProgGenInfo& info, const Token& token);

bool leftConversionIsProhibited(Expression::ExprType eType);

SymbolRef makeMacroSymbol(const Token::Position& pos, const std::string& name);

ExpressionRef genConvertExpression(ProgGenInfo& info, ExpressionRef expToConvert, const Datatype& newDatatype, bool isExplicit = false, bool doThrow = true);

void parseGlobalCode(ProgGenInfo& info, bool fromBeginning = true);

void parseInlineTokens(ProgGenInfo& info, TokenListRef tokens, const std::string& progPath);

std::string blueprintMacroNameFromName(const std::string& name);

std::string variadicNameFromID(int id);

std::string variadicTypeFromID(int id);

TokenList genVariadicParamDeclTokenList(const std::vector<int> varParamIDs);

void genBlueprintSpecPreSpace(const SymPath& path, TokenListRef tokens);

SymbolRef getMatchingOverload(ProgGenInfo& info, SymbolRef overloads, std::vector<ExpressionRef>& paramExpr, const std::vector<TokenListRef>& explicitMacros, const Token::Position& searchedFrom);

SymbolRef generateBlueprintSpecialization(ProgGenInfo& info, SymbolRef& bpSym, std::vector<ExpressionRef>& paramExpr, const std::vector<TokenListRef>& explicitMacros, const Token::Position& generatedFrom);

int calcConvScore(ProgGenInfo& info, Datatype from, Datatype to, bool isExplicit);

int calcFuncScore(ProgGenInfo& info, SymbolRef func, const std::vector<ExpressionRef>& paramExpr);

void addPossibleCandidates(ProgGenInfo& info, std::map<SymbolRef, int>& candidates, SymbolRef overloads, std::vector<ExpressionRef>& paramExpr);

SymbolRef getVariable(ProgGenInfo& info, const std::string& name);

SymbolRef getFunctions(ProgGenInfo& info, const std::string& name);

SymbolRef getFunctions(ProgGenInfo& info, const std::vector<std::string>& path);

SymbolRef getEnum(ProgGenInfo& info, const std::string& name);

uint64_t getEnumValue(ProgGenInfo& info, const std::string& enumName, const std::string& memberName);

Datatype getParseDatatype(ProgGenInfo& info, std::vector<Token>* pBlueprintMacroTokens = nullptr);

bool parseEmptyLine(ProgGenInfo& info);

void parseExpected(ProgGenInfo& info, Token::Type type);

void parseExpected(ProgGenInfo& info, Token::Type type, const std::string& value);

void parseExpectedNewline(ProgGenInfo& info);

bool parseOptionalNewline(ProgGenInfo& info);

void parseExpectedColon(ProgGenInfo& info);

void addVariable(ProgGenInfo& info, SymbolRef sym);

SymbolRef addFunction(ProgGenInfo& info, SymbolRef func);

SymbolRef addPack(ProgGenInfo& info, SymbolRef pack);

std::string preprocessAsmCode(ProgGenInfo& info, const Token& asmToken);

void increaseIndent(ProgGenInfo& info);

bool parseIndent(ProgGenInfo& info, bool ignoreLeadingWhitespaces = false);

void unparseIndent(ProgGenInfo& info);

void decreaseIndent(ProgGenInfo& info);

void pushTempBody(ProgGenInfo& info, BodyRef body);

void popTempBody(ProgGenInfo& info);

Datatype getBestConvDatatype(const Datatype& left, const Datatype& right);

ExpressionRef makeConvertExpression(ExpressionRef expToConvert, const Datatype& newDatatype);

ExpressionRef genAutoArrayToPtr(ExpressionRef expToConvert);

bool isConvPossible(ProgGenInfo& info, const Datatype& oldDt, const Datatype& newDt, bool isExplicit);

void autoFixDatatypeMismatch(ProgGenInfo& info, ExpressionRef exp);

ExpressionRef getParseExpression(ProgGenInfo& info, int precLvl = 0);

ExpressionRef getParseExpression(ProgGenInfo& info, int precLvl, const Datatype& targetType, bool isExplicit = false, bool doThrow = true);

ExpressionRef getParseEnumMember(ProgGenInfo& info);

ExpressionRef getParseLiteral(ProgGenInfo& info);

ExpressionRef getParseSymbol(ProgGenInfo& info, bool localOnly);

ExpressionRef getParseValue(ProgGenInfo& info, bool localOnly);

ExpressionRef getParseParenthesized(ProgGenInfo& info);

ExpressionRef autoSimplifyExpression(ExpressionRef expr);

ExpressionRef getParseBinaryExpression(ProgGenInfo& info, int precLvl);

ExpressionRef getParseUnarySuffixExpression(ProgGenInfo& info, int precLvl);

ExpressionRef getParseUnaryPrefixExpression(ProgGenInfo& info, int precLvl);

bool parseExpression(ProgGenInfo& info);

bool parseExpression(ProgGenInfo& info, const Datatype& targetType);

void parseFunctionBody(ProgGenInfo& info, bool doParseIndent);

std::pair<SymbolRef, ExpressionRef> getParseDeclDefVariable(ProgGenInfo& info);

bool parseDeclDefVariable(ProgGenInfo& info);

bool parseDeclDefFunction(ProgGenInfo& info);

bool parseDeclDef(ProgGenInfo& info);

bool parseDeclExtFunc(ProgGenInfo& info);

bool parseStatementReturn(ProgGenInfo& info);

SymbolRef makeAlias(const std::string& name, Token::Position pos, SymbolRef sym);

bool parseStatementAlias(ProgGenInfo& info);

bool parseStatementDefer(ProgGenInfo& info);

bool parseInlineAssembly(ProgGenInfo& info);

bool parseStatementPass(ProgGenInfo& info);

bool parseStatementContinue(ProgGenInfo& info);

bool parseStatementBreak(ProgGenInfo& info);

void parseBody(ProgGenInfo& info, bool doParseIndent);

void parseBodyEx(ProgGenInfo& info, BodyRef body, bool doParseIndent);

bool parseStatementIf(ProgGenInfo& info);

bool parseStatementWhile(ProgGenInfo& info);

bool parseStatementDoWhile(ProgGenInfo& info);

bool parseControlFlow(ProgGenInfo& info);

void parseFunctionBody(ProgGenInfo& info, bool doParseIndent);

bool parseStatementDefine(ProgGenInfo& info);

bool parseStatementSpace(ProgGenInfo& info);

bool parsePackUnion(ProgGenInfo& info);

bool parseEnum(ProgGenInfo& info);

void parseGlobalCode(ProgGenInfo& info, bool fromBeginning);

void importFile(ProgGenInfo& info, const Token& fileToken);

bool parseStatementImport(ProgGenInfo& info);

bool parseSingleGlobalCode(ProgGenInfo& info);

void markReachableFunctions(ProgGenInfo& info, BodyRef body);

void detectUndefinedFunctions(ProgGenInfo& info);

void importDeferredImports(ProgGenInfo& info);

void parseDeferredCompilations(ProgGenInfo& info);

void genDeclaredOnlyBpSpecs(ProgGenInfo& info);