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
	} var;

	struct Function
	{
		Datatype retType;
		int retOffset = 16;
		std::vector<Variable> params;
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

void addSymbol(SymbolRef curr, SymbolRef symbol);

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

SymbolRef getSymbol(const SymbolRef curr, const std::string& name, bool localOnly = false);
SymbolRef getParent(const SymbolRef symbol);

class SymbolIterator
{
public:
	SymbolIterator(const SymbolRef symbol);
public:
	SymbolIterator& operator++();
	bool operator==(const SymbolIterator& other);
	bool operator!=(const SymbolIterator& other);
	SymbolRef operator*();
private:
	SymbolRef currSym();
	SymbolTable::iterator currIt();
	void checkoutFrontLeaf();
	void checkoutBackLeaf();
private:
	std::stack<std::pair<SymbolRef, SymbolTable::iterator>> m_stack;
};