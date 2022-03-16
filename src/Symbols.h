#pragma once

#include <map>
#include <string>
#include <memory>
#include <stack>

#include "Token.h"
#include "Statement.h"

struct Symbol;
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
		int frameSize = 0;
		std::vector<Variable> params;
		BodyRef body;
		bool isReachable = false;
	} func;

	struct Pack
	{
		int size = 0;
	} pack;
};

typedef Symbol::Type SymType;

void addSymbol(SymbolRef symbol, SymbolRef curr);

bool isSymType(const SymbolRef symbol, SymType type);
bool isVariable(const SymbolRef symbol);
bool isFunction(const SymbolRef symbol);
bool isPack(const SymbolRef symbol);

SymbolRef getSymbol(const std::string& name, const SymbolRef curr);
SymbolRef getParent(const SymbolRef symbol);

class SymbolIterator
{
public:
	SymbolIterator(const SymbolRef symbol);
public:
	SymbolIterator& operator++();
	SymbolIterator& operator--();
	bool operator==(const SymbolIterator& other);
	bool operator!=(const SymbolIterator& other);
	SymbolRef operator*();
private:
	std::stack<SymbolRef> m_stack;
};