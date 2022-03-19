#include "Symbols.h"

#include <cassert>

#include "Errors/ProgGenError.h"

SymbolIterator Symbol::begin()
{
	return SymbolIterator(this, SymbolIterator::InitPos::Begin);
}

SymbolIterator Symbol::end()
{
	return SymbolIterator(this, SymbolIterator::InitPos::End);
}

void addSymbol(SymbolRef curr, SymbolRef symbol)
{
	if (curr->subSymbols.find(symbol->name) != curr->subSymbols.end())
		THROW_PROG_GEN_ERROR(symbol->pos, "Symbol '" + symbol->name + "' already defined");
	symbol->parent = curr;
	curr->subSymbols[symbol->name] = symbol;
}

bool isInType(const SymbolRef symbol, Symbol::Type type)
{
	if (!symbol)
		return false;
	if (symbol->type == type)
		return true;
	if (!symbol->parent.expired())
		return isInType(symbol->parent.lock(), type);
	return false;
}
bool isInGlobal(const SymbolRef symbol)
{
	return
		!isInPack(symbol) &&
		!isInFunction(symbol) &&
		isInType(symbol, Symbol::Type::Global);
}

bool isInFunction(const SymbolRef symbol)
{
	return isInType(symbol, Symbol::Type::FunctionSpec);
}

bool isInPack(const SymbolRef symbol)
{
	return isInType(symbol, Symbol::Type::Pack);
}

bool isInEnum(const SymbolRef symbol)
{
	return isInType(symbol, Symbol::Type::Enum);
}

bool isSymType(SymType type, const SymbolRef symbol)
{
	if (!symbol)
		return false;
	return symbol->type == type;
}

bool isVariable(const SymbolRef symbol)
{
	return isSymType(SymType::Variable, symbol);
}

bool isFuncName(const SymbolRef symbol)
{
	return isSymType(SymType::FunctionName, symbol);
}

bool isFuncSpec(const SymbolRef symbol)
{
	return isSymType(SymType::FunctionSpec, symbol);
}

bool isFunction(const SymbolRef symbol)
{
	return isFuncName(symbol) || isFuncSpec(symbol);
}

bool isPack(const SymbolRef symbol)
{
	return isSymType(SymType::Pack, symbol);
}

bool isEnum(const SymbolRef symbol)
{
	return isSymType(SymType::Enum, symbol);
}

bool isSymState(SymState state, const SymbolRef symbol)
{
	return symbol->state == state;
}

bool isDeclared(const SymbolRef symbol)
{
	return isSymState(SymState::Declared, symbol);
}

bool isDefined(const SymbolRef symbol)
{
	return isSymState(SymState::Defined, symbol);
}

bool isVarContext(const SymbolRef symbol, Symbol::Variable::Context context)
{
	return isVariable(symbol) && symbol->var.context == context;
}

bool isVarGlobal(const SymbolRef symbol)
{
	return isVarContext(symbol, Symbol::Variable::Context::Global);
}

bool isVarLocal(const SymbolRef symbol)
{
	return isVarContext(symbol, Symbol::Variable::Context::Local);
}

bool isVarStatic(const SymbolRef symbol)
{
	return isVarContext(symbol, Symbol::Variable::Context::Static);
}

bool isVarParameter(const SymbolRef symbol)
{
	return isVarContext(symbol, Symbol::Variable::Context::Parameter);
}

bool isVarPackMember(const SymbolRef symbol)
{
	return isVarContext(symbol, Symbol::Variable::Context::PackMember);
}

bool isVarLabeled(const SymbolRef symbol)
{
	return
		isVarLocal(symbol) ||
		isVarParameter(symbol) ||
		isVarPackMember(symbol);
}

bool isVarOffset(const SymbolRef symbol)
{
	return !isVarLabeled(symbol);
}

SymbolRef getSymbol(SymbolRef curr, const std::string& name, bool localOnly)
{
	while (curr)
	{
		auto it = curr->subSymbols.find(name);
		if (it != curr->subSymbols.end())
			return it->second;
		if (localOnly)
			break;
		curr = getParent(curr);
	}
	return nullptr;
}

void replaceSymbol(SymbolRef curr, const std::string& name, SymbolRef newSym, bool localOnly)
{
	while (curr)
	{
		auto it = curr->subSymbols.find(name);
		if (it != curr->subSymbols.end())
		{
			newSym->parent = it->second->parent;
			it->second = newSym;
			break;
		}
		if (localOnly)
			assert(false && "Cannot replace non-existent symbol!");
		curr = getParent(curr);
	}
}

SymbolRef getParent(const SymbolRef symbol)
{
	return symbol->parent.lock();
}

SymbolIterator::SymbolIterator(Symbol* symbol, InitPos iPos)
{
	switch (iPos)
	{
	case InitPos::Begin:
		m_stack.push({ symbol, symbol->subSymbols.begin() });
		break;
	case InitPos::End:
		m_stack.push({ symbol, symbol->subSymbols.end() });
		break;
	}
}

SymbolIterator& SymbolIterator::operator++()
{
	if (!currIt()->second->subSymbols.empty())
	{
		m_stack.push({ currIt()->second.get(), currIt()->second->subSymbols.begin() });
		return *this;
	}

	++currIt();

	while (currIt() == currSym()->subSymbols.end())
	{
		if (m_stack.size() <= 1)
			break;
		m_stack.pop();
		++currIt();
	}

	return *this;
}

bool SymbolIterator::operator==(const SymbolIterator& other)
{
	return m_stack == other.m_stack;
}

bool SymbolIterator::operator!=(const SymbolIterator& other)
{
	return !(*this == other);
}

SymbolRef SymbolIterator::operator*()
{
	return currIt()->second;
}

Symbol* SymbolIterator::currSym()
{
	return m_stack.top().first;
}

SymbolTable::iterator& SymbolIterator::currIt()
{
	return m_stack.top().second;
}

void SymbolIterator::checkoutFrontLeaf()
{
	while (!currIt()->second->subSymbols.empty())
		m_stack.push({ currIt()->second.get(), currIt()->second->subSymbols.begin() });
}

void SymbolIterator::checkoutBackLeaf()
{
	while (!currIt()->second->subSymbols.empty())
	{
		auto it = currIt()->second->subSymbols.end();
		--it;
		m_stack.push({ currIt()->second.get(), it });
	}
}