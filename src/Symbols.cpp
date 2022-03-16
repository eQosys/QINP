#include "Symbols.h"

#include <cassert>

#include "Errors/ProgGenError.h"

void addSymbol(SymbolRef symbol, SymbolRef curr)
{
	if (curr.find(symbol->name) != curr.end())
		THROW_PROG_GEN_ERROR(symbol->pos, "Symbol '" + symbol->name + "' already defined");
	symbol->parent = curr;
	curr[symbol->name] = symbol;
}

bool isSymType(const SymbolRef symbol, SymType type)
{
	return symbol->type == type;
}

bool isVariable(const SymbolRef symbol)
{
	return isSymType(symbol, SymType::Variable);
}

bool isFunction(const SymbolRef symbol)
{
	return isSymType(symbol, SymType::FunctionName) || isSymType(symbol, SymType::FunctionSpec);
}

bool isPack(const SymbolRef symbol)
{
	return isSymType(symbol, SymType::Pack);
}

SymbolRef getSymbol(const std::string& name, const SymbolRef curr)
{
	while (curr)
	{
		auto it = curr->find(name);
		if (it != curr->end())
			return it->second;
		curr = getParent(curr);
	}
	return nullptr;
}

SymbolRef getParent(const SymbolRef symbol)
{
	return symbol->parent.lock();
}

SymbolIterator::SymbolIterator(const SymbolRef symbol)
{
	m_stack.push(symbol);
}