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
	Token::Position pos;
	std::string name;
	SymbolWeakRef parent;
	SymbolTable subSymbols;

	enum class Type
	{
		None,
		Global,
		Variable,
		FunctionName, // The general function name
		FunctionSpec, // Any specialization of the function (overloads)
		Pack,
		Enum,
	} type = Type::None;

	enum class State
	{
		None,
		Declared,
		Defined,
	} state = State::None;

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
			EnumMember,
		} context = Context::None;
	} var;

	struct Function
	{
		Datatype retType;
		int retOffset = 16;
		std::vector<SymbolRef> params;
		BodyRef body;
		bool isReachable = false;
	} func;

	struct Frame
	{
		int size = 0;
		int totalOffset = 0;
	} frame;

	struct Pack
	{
		int size = 0;
	} pack;

	int enumValue;

public:
	SymbolIterator begin();
	SymbolIterator end();
};

typedef Symbol::Type SymType;
typedef Symbol::State SymState;
typedef Symbol::Variable::Context SymVarContext;

void addSymbol(SymbolRef curr, SymbolRef symbol);

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
bool isFunction(const SymbolRef symbol);
bool isPack(const SymbolRef symbol);
bool isEnum(const SymbolRef symbol);

bool isSymState(SymState state, const SymbolRef symbol);
bool isDeclared(const SymbolRef symbol);
bool isDefined(const SymbolRef symbol);

bool isVarContext(const SymbolRef symbol, Symbol::Variable::Context context);
bool isVarGlobal(const SymbolRef symbol);
bool isVarLocal(const SymbolRef symbol);
bool isVarStatic(const SymbolRef symbol);
bool isVarParameter(const SymbolRef symbol);
bool isVarPackMember(const SymbolRef symbol);

SymbolRef getSymbol(SymbolRef curr, const std::string& name, bool localOnly = false);
void replaceSymbol(SymbolRef curr, const std::string& name, SymbolRef newSym, bool localOnly = false);
SymbolRef getParent(const SymbolRef symbol);

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
	SymbolTable::iterator currIt();
	void checkoutFrontLeaf();
	void checkoutBackLeaf();
private:
	std::stack<std::pair<Symbol*, SymbolTable::iterator>> m_stack;
};