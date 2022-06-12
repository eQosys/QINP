#pragma once

#include <map>
#include <string>
#include <memory>
#include <stack>

#include "Token.h"
#include "Statement.h"

struct Symbol;
class SymbolIterator;

typedef std::shared_ptr<Symbol> SymbolRef;
typedef std::weak_ptr<Symbol> SymbolWeakRef;

typedef std::map<std::string, SymbolRef> SymbolTable;

struct Symbol
{
	struct DeclDefPos
	{
		Token::Position decl, def;
	} pos;
	std::string name;
	SymbolWeakRef parent;
	SymbolTable subSymbols;

	enum class Type
	{
		None,
		Namespace,
		Global,
		Variable,
		FunctionName, // The general function name
		FunctionSpec, // Any specialization of the function (overloads)
		ExtFunc,
		Pack,
		Enum,
		EnumMember,
		Macro,
	} type = Type::None;

	enum class State
	{
		Declared,
		Defined,
	} state = State::Declared;

	struct Variable
	{
		std::string modName;
		int offset = -1;
		Datatype datatype;

		enum class Context
		{
			None,
			Global,
			Local,
			Static,
			Parameter,
			PackMember,
		} context = Context::None;
	} var;

	struct Function
	{
		Datatype retType;
		int retOffset = 16; // Stack pointer + return address (2x 8 bytes)
		std::vector<SymbolRef> params;
		BodyRef body;
		bool isReachable = false;
		std::string asmName;
		bool isBlueprint = false;
		bool isVariadic = false;
		bool genFromBlueprint = false;
		TokenListRef blueprintTokens;
	} func;

	struct Frame
	{
		int size = 0;
		int totalOffset = 0;
	} frame;

	struct Pack
	{
		int size = 0;
		bool isUnion = false;
	} pack;

	int64_t enumValue;

	TokenList macroTokens;

public:
	SymbolIterator begin();
	SymbolIterator end();
};

typedef Symbol::Type SymType;
typedef Symbol::State SymState;
typedef Symbol::Variable::Context SymVarContext;

void addSymbol(SymbolRef root, SymbolRef symbol);

Token::Position& getBestPos(SymbolRef symbol);

// isIn* functions return true if the symbol itself or any of its parent is of the given type
bool isInType(const SymbolRef symbol, Symbol::Type type);
bool isInGlobal(const SymbolRef symbol);
bool isInFunction(const SymbolRef symbol);
bool isInPack(const SymbolRef symbol);
bool isInEnum(const SymbolRef symbol);

bool isSymType(SymType type, const SymbolRef symbol);
bool isVariable(const SymbolRef symbol);
bool isFuncName(const SymbolRef symbol);
bool isFuncSpec(const SymbolRef symbol);
bool isExtFunc(const SymbolRef symbol);
bool isFunction(const SymbolRef symbol);
bool isPack(const SymbolRef symbol);
bool isEnum(const SymbolRef symbol);
bool isMacro(const SymbolRef symbol);

bool isSymState(SymState state, const SymbolRef symbol);
bool isDeclared(const SymbolRef symbol);
bool isDefined(const SymbolRef symbol);

bool isVarContext(const SymbolRef symbol, Symbol::Variable::Context context);
bool isVarGlobal(const SymbolRef symbol);
bool isVarLocal(const SymbolRef symbol);
bool isVarStatic(const SymbolRef symbol);
bool isVarParameter(const SymbolRef symbol);
bool isVarPackMember(const SymbolRef symbol);

bool isVarLabeled(const SymbolRef symbol);
bool isVarOffset(const SymbolRef symbol);

bool isReachable(const SymbolRef symbol);

typedef std::vector<std::string> SymPath;

SymbolRef getSymbolFromPath(SymbolRef root, const SymPath& path);
SymPath getSymbolPath(const SymbolRef root, const SymbolRef symbol);
std::string SymPathToString(const SymPath& path);
SymPath SymPathFromString(const std::string& pathStr);

SymbolRef getSymbol(SymbolRef root, const std::string& name, bool localOnly = false);
SymbolRef replaceSymbol(SymbolRef curr, SymbolRef newSym);
SymbolRef getParent(const SymbolRef symbol);
SymbolRef getParent(SymbolRef curr, Symbol::Type type);


std::string SymStateToString(SymState state);
std::string SymTypeToString(SymType type);
std::string SymVarContextToString(SymVarContext context);

class SymbolIterator
{
public:
	enum class InitPos { Begin, End };
public:
	SymbolIterator(Symbol* symbol, InitPos iPos = InitPos::Begin);
public:
	SymbolIterator& operator++();
	bool operator==(const SymbolIterator& other);
	bool operator!=(const SymbolIterator& other);
	SymbolRef operator*();
private:
	Symbol* currSym();
	SymbolTable::iterator& currIt();
	void checkoutFrontLeaf();
	void checkoutBackLeaf();
private:
	std::stack<std::pair<Symbol*, SymbolTable::iterator>> m_stack;
};