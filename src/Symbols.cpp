#include "Symbols.h"

#include <cassert>
#include <algorithm>

#include "Errors/ProgGenError.h"

SymbolIterator Symbol::begin()
{
	return SymbolIterator(this, SymbolIterator::InitPos::Begin);
}

SymbolIterator Symbol::end()
{
	return SymbolIterator(this, SymbolIterator::InitPos::End);
}

void addSymbol(SymbolRef root, SymbolRef symbol)
{
	if (root->subSymbols.find(symbol->name) != root->subSymbols.end())
		THROW_PROG_GEN_ERROR_POS(symbol->pos.decl, "Symbol with name '" + symbol->name + "' already declared here " + getPosStr(root->subSymbols.at(symbol->name)->pos.decl));
	symbol->parent = root;
	root->subSymbols[symbol->name] = symbol;
}

Token::Position& getBestPos(SymbolRef symbol)
{
	return isDefined(symbol) ? symbol->pos.def : symbol->pos.decl;
}

bool isInType(const SymbolRef symbol, Symbol::Type type, bool directOnly)
{
	return getParent(symbol, type, directOnly) != nullptr;
}

bool isInGlobal(const SymbolRef symbol, bool directOnly)
{
	return
		!isInPack(symbol) &&
		!isInFunction(symbol) &&
		isInType(symbol, Symbol::Type::Global, directOnly);
}

bool isInFunction(const SymbolRef symbol, bool directOnly)
{
	return isInType(symbol, Symbol::Type::FunctionSpec, directOnly);
}

bool isInPack(const SymbolRef symbol, bool directOnly)
{
	return isInType(symbol, Symbol::Type::Pack, directOnly);
}

bool isInEnum(const SymbolRef symbol, bool directOnly)
{
	return isInType(symbol, Symbol::Type::Enum, directOnly);
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

bool isExtFunc(const SymbolRef symbol)
{
	return isSymType(SymType::ExtFunc, symbol);
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

bool isMacro(const SymbolRef symbol)
{
	return isSymType(SymType::Macro, symbol);
}

bool isAlias(const SymbolRef symbol)
{
	return isSymType(SymType::Alias, symbol);
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
	return isVariable(symbol) && !isVarOffset(symbol);
}

bool isVarOffset(const SymbolRef symbol)
{
	return
		isVarLocal(symbol) ||
		isVarParameter(symbol) ||
		isVarPackMember(symbol);
}


bool isReachable(const SymbolRef symbol)
{
	assert(isFuncSpec(symbol) || isExtFunc(symbol));
	return symbol->func.isReachable;
}

SymbolRef getSymbolFromPath(SymbolRef root, const SymPath& path)
{
	SymbolRef curr = root;
	for (auto& name : path)
	{
		curr = getSymbol(curr, name, true);
		if (!curr)
			return nullptr;
	}
	return curr;
}

SymPath getSymbolPath(SymbolRef root, SymbolRef symbol)
{
	SymPath path;

	while (symbol != root)
	{
		path.push_back(symbol->name);
		symbol = getParent(symbol);
	}

	if (!root)
		path.pop_back();

	std::reverse(path.begin(), path.end());

	return path;
}

std::string SymPathToString(const SymPath& path)
{
	std::string str;
	for (int i = 0; i < path.size(); ++i)
	{
		if (i > 0) str += ".";
		str += path[i];
	}
	return str;
}

SymPath SymPathFromString(const std::string& pathStr)
{
	SymPath path;
	std::size_t end = -1;
	do
	{
		auto begin = end + 1;
		end = pathStr.find('.', begin);
		path.push_back(pathStr.substr(begin, end - begin));
	} while (end != std::string::npos);
	
	return path;
}

SymbolRef getSymbol(SymbolRef root, const std::string& name, bool localOnly)
{
	while (root)
	{
		auto it = root->subSymbols.find(name);
		if (it != root->subSymbols.end())
			return it->second;
		if (localOnly)
			break;
		root = getParent(root);
	}
	return nullptr;
}

SymbolRef replaceSymbol(SymbolRef currSym, SymbolRef newSym)
{
	newSym->parent = currSym->parent;
	auto oldPos = currSym->pos;
	*currSym = *newSym;
	currSym->pos = oldPos;
	return currSym;
}

SymbolRef getParent(SymbolRef symbol)
{
	return symbol->parent.lock();
}

SymbolRef getParent(SymbolRef symbol, uint64_t num)
{
	while (num-- > 0)
		symbol = getParent(symbol);
	return symbol;
}

SymbolRef getParent(SymbolRef curr, Symbol::Type type, bool directOnly)
{
	while (curr)
	{
		if (curr->type == type)
			return curr;
		if (directOnly)
			return nullptr;
		curr = getParent(curr);
	}
	return curr;
}

std::string SymStateToString(SymState state)
{
	switch (state)
	{
	case SymState::Declared: return "Declared";
	case SymState::Defined: return "Defined";
	}

	assert(false && "Invalid SymState");
	return "<unknown>";
}

std::string SymTypeToString(SymType type)
{
	switch (type)
	{
	case SymType::None: return "None";
	case SymType::Namespace: return "Namespace";
	case SymType::Global: return "Global";
	case SymType::Variable: return "Variable";
	case SymType::FunctionName: return "FuncName";
	case SymType::FunctionSpec: return "FuncSpec";
	case SymType::ExtFunc: return "ExtFunc";
	case SymType::Pack: return "Pack";
	case SymType::Enum: return "Enum";
	case SymType::EnumMember: return "EnumMember";
	case SymType::Macro: return "Macro";
	case SymType::Alias: return "Alias";
	}

	assert(false && "Unknown symbol type");

	return "Unknown";
}

std::string SymVarContextToString(Symbol::Variable::Context context)
{
	switch (context)
	{
	case Symbol::Variable::Context::None: return "None";
	case Symbol::Variable::Context::Global: return "Global";
	case Symbol::Variable::Context::Local: return "Local";
	case Symbol::Variable::Context::Static: return "Static";
	case Symbol::Variable::Context::Parameter: return "Parameter";
	case Symbol::Variable::Context::PackMember: return "PackMember";
	}

	assert(false && "Unknown variable context");

	return "Unknown";
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