#include "ProgramGenerator.h"

#include <map>
#include <set>
#include <algorithm>
#include <filesystem>

#include "Errors/ProgGenError.h"

#include "Tokenizer.h"

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

	std::vector<LocalStackFrame> localStack;

	BodyRef __mainBodyBackup;
	int funcRetOffset;
	int funcFrameSize;
	Datatype funcRetType;

	std::set<std::string> imports;
};

struct ProgGenInfoBackup
{
	TokenListRef tokens;
	int currToken;
};

ProgGenInfoBackup makeProgGenInfoBackup(const ProgGenInfo& info)
{
	ProgGenInfoBackup backup;
	backup.tokens = info.tokens;
	backup.currToken = info.currToken;
	return backup;
}

void loadProgGenInfoBackup(ProgGenInfo& info, const ProgGenInfoBackup& backup)
{
	info.tokens = backup.tokens;
	info.currToken = backup.currToken;
}

struct OpPrecLvl
{
	std::map<std::string, Expression::ExprType> ops;
	enum class Type
	{
		Unary_Prefix,
		Unary_Suffix,
		Binary,
	} type;
};

std::vector<OpPrecLvl> opPrecLvls = 
{
	{
		{
			{ "=", Expression::ExprType::Assign },
			{ "+=", Expression::ExprType::Assign_Sum },
			{ "-=", Expression::ExprType::Assign_Difference },
			{ "*=", Expression::ExprType::Assign_Product },
			{ "/=", Expression::ExprType::Assign_Quotient },
			{ "%=", Expression::ExprType::Assign_Remainder },
			{ "<<=", Expression::ExprType::Assign_Bw_LeftShift },
			{ ">>=", Expression::ExprType::Assign_Bw_RightShift },
			{ "&=", Expression::ExprType::Assign_Bw_AND },
			{ "^=", Expression::ExprType::Assign_Bw_XOR },
			{ "|=", Expression::ExprType::Assign_Bw_OR },
		},
		OpPrecLvl::Type::Binary,
	},
	{
		{
			{ "||", Expression::ExprType::Logical_OR },
		},
		OpPrecLvl::Type::Binary,
	},
	{
		{
			{ "&&", Expression::ExprType::Logical_AND },
		},
		OpPrecLvl::Type::Binary,
	},
	{
		{
			{ "|", Expression::ExprType::Bitwise_OR },
		},
		OpPrecLvl::Type::Binary,
	},
	{
		{
			{ "^", Expression::ExprType::Bitwise_XOR },
		},
		OpPrecLvl::Type::Binary,
	},
	{
		{
			{ "&", Expression::ExprType::Bitwise_AND },
		},
		OpPrecLvl::Type::Binary,
	},
	{
		{
			{ "==", Expression::ExprType::Comparison_Equal },
			{ "!=", Expression::ExprType::Comparison_NotEqual },
		},
		OpPrecLvl::Type::Binary,
	},
	{
		{
			{ "<", Expression::ExprType::Comparison_Less },
			{ "<=", Expression::ExprType::Comparison_LessEqual },
			{ ">", Expression::ExprType::Comparison_Greater },
			{ ">=", Expression::ExprType::Comparison_GreaterEqual },
		},
		OpPrecLvl::Type::Binary,
	},
	{
		{
			{ "<<", Expression::ExprType::Shift_Left },
			{ ">>", Expression::ExprType::Shift_Right },
		},
		OpPrecLvl::Type::Binary,
	},
	{
		{
			{ "+", Expression::ExprType::Sum },
			{ "-", Expression::ExprType::Difference },
		},
		OpPrecLvl::Type::Binary,
	},
	{
		{
			{ "*", Expression::ExprType::Product },
			{ "/", Expression::ExprType::Quotient },
			{ "%", Expression::ExprType::Remainder },
		},
		OpPrecLvl::Type::Binary,
	},
	{
		{
			{ "&", Expression::ExprType::AddressOf },
			{ "*", Expression::ExprType::Dereference },
			{ "!", Expression::ExprType::Logical_NOT },
			{ "~", Expression::ExprType::Bitwise_NOT },
			{ "+", Expression::ExprType::Prefix_Plus },
			{ "-", Expression::ExprType::Prefix_Minus },
			{ "++", Expression::ExprType::Prefix_Increment },
			{ "--", Expression::ExprType::Prefix_Decrement },
			{ "(", Expression::ExprType::Explicit_Cast }
		},
		OpPrecLvl::Type::Unary_Prefix,
	},
	{
		{
			{ "[", Expression::ExprType::Subscript },
			{ "(", Expression::ExprType::FunctionCall },
			{ "++", Expression::ExprType::Suffix_Increment },
			{ "--", Expression::ExprType::Suffix_Decrement },
		},
		OpPrecLvl::Type::Unary_Suffix,
	},
};

bool isKeyword(const Token& token, const std::string& name)
{
	return
		token.type == Token::Type::Keyword &&
		token.value == name;
}

bool isSeparator(const Token& token, const std::string& name)
{
	return
		token.type == Token::Type::Separator &&
		token.value == name;
}

bool isOperator(const Token& token, const std::string& name)
{
	return
		token.type == Token::Type::Operator &&
		token.value == name;
}

bool isLiteral(const Token& token)
{
	return
		token.type == Token::Type::LiteralInteger ||
		token.type == Token::Type::LiteralChar ||
		token.type == Token::Type::LiteralBoolean ||
		token.type == Token::Type::String;
}

bool isNewline(const Token& token)
{
	return token.type == Token::Type::Newline;
}

bool isEndOfCode(const Token& token)
{
	return token.type == Token::Type::EndOfCode;
}

bool isBuiltinType(const Token& token)
{
	return token.type == Token::Type::BuiltinType;
}

bool isIdentifier(const Token& token)
{
	return token.type == Token::Type::Identifier;
}

bool isString(const Token& token)
{
	return token.type == Token::Type::String;
}

bool isWhitespace(const Token& token)
{
	return token.type == Token::Type::Whitespace;
}

const Token& peekToken(ProgGenInfo& info, int offset = 0)
{
	static const Token emptyToken = { { "", 0, 0 }, Token::Type::EndOfCode, "" };
	int index = info.currToken + offset;
	return (index < info.tokens->size()) ? (*info.tokens)[index] : emptyToken;
}

const Token& nextToken(ProgGenInfo& info, int offset = 1)
{
	auto& temp = peekToken(info, offset - 1);
	info.currToken += offset;
	return temp;
}

ExpressionRef genConvertExpression(ExpressionRef expToConvert, const Datatype& newDatatype, bool isExplicit = false, bool doThrow = true);

FunctionRef getMatchingOverload(ProgGenInfo& info, const FunctionOverloads& overloads, std::vector<ExpressionRef>& paramExpr)
{
	FunctionRef match = nullptr;

	for (auto& overload : overloads)
	{
		auto& func = overload.second;

		if (func->params.size() != paramExpr.size())
			continue;

		bool matchFound = true;
		bool isExactMatch = true;
		for (int i = 0; i < paramExpr.size() && matchFound; ++i)
		{
			if (paramExpr[i]->datatype != func->params[i].datatype)
				isExactMatch = false;
			matchFound = !!genConvertExpression(paramExpr[i], func->params[i].datatype, false, false);
		}

		if (isExactMatch)
			return func;
		
		if (matchFound)
		{
			if (match)
				throw ProgGenError(peekToken(info).pos, "Multiple overloads found for function call");
			else
				match = func;
		}
	}

	if (match)
	{
		for (int i = 0; i < paramExpr.size(); ++i)
			paramExpr[i] = genConvertExpression(paramExpr[i], match->params[i].datatype, false);
	}

	return match;
}

const Variable* getVariable(ProgGenInfo& info, const std::string& name)
{
	// First check for locals, then for globals (shadowing)
	for (auto frameIt = info.localStack.rbegin(); frameIt != info.localStack.rend(); ++frameIt)
	{
		auto locIt = frameIt->locals.find(name);
		if (locIt != frameIt->locals.end())
			return &locIt->second;
	}

	auto& globals = info.program->globals;
	auto globIt = globals.find(name);
	if (globIt != globals.end())
		return &globIt->second;

	return nullptr;
}

const FunctionOverloads* getFunctions(ProgGenInfo& info, const std::string& name)
{
	auto& functions = info.program->functions;
	auto it = functions.find(name);
	if (it != functions.end())
		return &it->second;

	return nullptr;
}

void addVariable(ProgGenInfo& info, Variable var)
{
	if (info.localStack.empty())
	{
		auto varIt = info.program->globals.find(var.name);
		if (varIt != info.program->globals.end())
			throw ProgGenError(var.pos, "Variable with name '" + var.name + "' already declared globally: " + getPosStr(varIt->second.pos));
		auto funcs = getFunctions(info, var.name);
		if (funcs)
			throw ProgGenError(var.pos, "Function with name '" + var.name + "' already declared: " + getPosStr(funcs->begin()->second->pos));

		var.isLocal = false;
		info.program->globals.insert({ var.name, var });
	}
	else
	{
		auto varIt = info.localStack.back().locals.find(var.name);
		if (varIt != info.localStack.back().locals.end())
			throw ProgGenError(var.pos, "Variable with name '" + var.name + "' already declared in the same frame: " + getPosStr(varIt->second.pos));

		int varSize = getDatatypeSize(var.datatype);
		var.isLocal = true;
		var.offset = info.funcRetOffset;

		info.localStack.back().frameSize += varSize;
		if (info.localStack.back().frameSize > info.funcFrameSize)
			info.funcFrameSize = info.localStack.back().frameSize;
		info.localStack.back().totalOffset -= varSize;
		var.offset = info.localStack.back().totalOffset;

		info.localStack.back().locals.insert({ var.name, var });
	}
}

void addFunction(ProgGenInfo& info, FunctionRef func)
{
	auto varIt = info.program->globals.find(func->name);
	if (varIt != info.program->globals.end())
		throw ProgGenError(func->pos, "Variable with name '" + func->name + "' already declared here: " + getPosStr(varIt->second.pos));

	auto sig = getSignatureNoRet(func);

	auto nameIt = info.program->functions.find(func->name);
	if (nameIt == info.program->functions.end())
	{
		info.program->functions[func->name].insert({ sig, func });
		return;
	}

	auto sigIt = nameIt->second.find(sig);

	if (sigIt == nameIt->second.end())
	{
		nameIt->second.insert({ sig, func });
		return;
	}

	if (func->retType != sigIt->second->retType)
		throw ProgGenError(func->pos, "Function with name '" + func->name + "' already declared with different return type: " + getPosStr(sigIt->second->pos));

	if (!func->isDefined)
		return;

	if (sigIt->second->isDefined && func->isDefined)
		throw ProgGenError(func->pos, "Function already defined here: " + getPosStr(sigIt->second->pos));

	sigIt->second = func;
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
					throw ProgGenError(asmToken.pos, "Expected '(' after '$'!");
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
				throw ProgGenError(asmToken.pos, "Unknown variable '" + varName + "'!");
				
			if (pVar->isLocal)
			{
				int offset = pVar->offset;
				result += (offset < 0) ? "- " : "+ ";
				result += std::to_string(std::abs(offset));
			}
			else
			{
				result += getMangledName(*pVar);
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
		throw ProgGenError(asmToken.pos, "Missing ')'!");
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
				throw ProgGenError(peekToken(info).pos, "Unexpected indentation!");
			return true;
		}

		if (info.indent.lvl != 1)
			throw ProgGenError(peekToken(info).pos, "Indentation level must be 1 for the first used indentation!");

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
				throw ProgGenError(pIndentToken->pos, "Inconsistent indentation!");
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
			throw ProgGenError(peekToken(info).pos, "Inconsistent indentation!");

	return true;
}

void decreaseIndent(ProgGenInfo::Indent& indent)
{
	if (indent.lvl == 0)
		throw ProgGenError(Token::Position(), "Indent level already 0!");
	--indent.lvl;
}

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
		throw ProgGenError(token.pos, "Unexpected token '" + token.value + "'!");
}

void parseExpected(ProgGenInfo& info, Token::Type type, const std::string& value)
{
	auto& token = nextToken(info);
	if (token.type != type || token.value != value)
		throw ProgGenError(token.pos, "Unexpected token '" + token.value + "'!");
}

void parseExpectedNewline(ProgGenInfo& info)
{
	auto& token = nextToken(info);
	if (!isNewline(token) && !isEndOfCode(token))
		throw ProgGenError(token.pos, "Expected newline!");
}

void parseExpectedColon(ProgGenInfo& info)
{
	parseExpected(info, Token::Type::Separator, ":");
}

void setTempBody(ProgGenInfo& info, BodyRef body)
{
	if (info.__mainBodyBackup)
		throw ProgGenError(peekToken(info).pos, "Cannot set temp body when a temp body is already set!");

	info.__mainBodyBackup = info.program->body;
	info.program->body = body;
}

void unsetTempBody(ProgGenInfo& info)
{
	if (!info.__mainBodyBackup)
		throw ProgGenError(peekToken(info).pos, "Cannot unset temp body when no temp body is set!");

	info.program->body = info.__mainBodyBackup;
	info.__mainBodyBackup = nullptr;
}

Datatype getBestConvDatatype(const ExpressionRef left, const ExpressionRef right)
{
	if (left->datatype.ptrDepth > 0 || right->datatype.ptrDepth > 0)
		return Datatype();

	if (!isBuiltinType(left->datatype.name) || !isBuiltinType(right->datatype.name))
		return Datatype();
	
	static const std::vector<std::string> typeOrder = 
	{
		"bool", "u8", "u16", "u32", "u64",
	};
	
	auto leftIt = std::find(typeOrder.begin(), typeOrder.end(), left->datatype.name);
	auto rightIt = std::find(typeOrder.begin(), typeOrder.end(), right->datatype.name);
	if (leftIt == typeOrder.end() || rightIt == typeOrder.end())
		return Datatype();

	return (leftIt > rightIt) ? left->datatype : right->datatype;
}

ExpressionRef genConvertExpression(ExpressionRef expToConvert, const Datatype& newDatatype, bool isExplicit, bool doThrow)
{
	if (expToConvert->datatype == newDatatype)
		return expToConvert;

	if (!isExplicit)
	{
		if (expToConvert->datatype.ptrDepth > 0 && newDatatype.ptrDepth > 0)
		{
			if (doThrow)
				throw ProgGenError(expToConvert->pos, "Cannot implicitly convert between pointer types!");
			else
				return nullptr;
		}
		if (
			(expToConvert->datatype.ptrDepth > 0 && newDatatype.ptrDepth == 0) ||
			(expToConvert->datatype.ptrDepth == 0 && newDatatype.ptrDepth > 0)
			)
		{
			if (doThrow)
				throw ProgGenError(expToConvert->pos, "Cannot implicitly convert between pointer and non-pointer types!");
			else
				return nullptr;
		}

		if (expToConvert->datatype.ptrDepth == 0 && newDatatype.ptrDepth == 0)
			return genConvertExpression(expToConvert, newDatatype, true);
	}

	auto exp = std::make_shared<Expression>(expToConvert->pos);
	exp->eType = Expression::ExprType::Conversion;
	exp->datatype = newDatatype;
	exp->left = expToConvert;

	return exp;
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

void autoFixDatatypeMismatch(ExpressionRef exp)
{
	if (exp->left->datatype == exp->right->datatype)
		return;

	Datatype newDatatype;
	if (leftConversionIsProhibited(exp->eType))
		newDatatype = exp->left->datatype;
	else
		newDatatype = getBestConvDatatype(exp->left, exp->right);

	if (!newDatatype)
		throw ProgGenError(
			exp->pos, "Cannot convert between " +
			exp->left->datatype.name +
			"*" + std::to_string(exp->left->datatype.ptrDepth) + " and " +
			exp->right->datatype.name +
			"*" + std::to_string(exp->right->datatype.ptrDepth) + "!");
	
	exp->left = genConvertExpression(exp->left, newDatatype);
	exp->right = genConvertExpression(exp->right, newDatatype);
}

ExpressionRef getParseExpression(ProgGenInfo& info, int precLvl = 0);

ExpressionRef getParseLiteral(ProgGenInfo& info)
{
	auto& litToken = peekToken(info);
	if (!isLiteral(litToken))
		return nullptr;

	nextToken(info);
	ExpressionRef exp = std::make_shared<Expression>(litToken.pos);

	exp->eType = Expression::ExprType::Literal;

	switch (litToken.type)
	{
	case Token::Type::LiteralInteger:
		exp->valStr = litToken.value;
		exp->datatype.name = "u64";
		break;
	case Token::Type::LiteralChar:
		exp->valStr = std::to_string(litToken.value[0]);
		exp->datatype.name = "u8";
		break;
	case Token::Type::LiteralBoolean:
		exp->valStr = std::to_string(litToken.value == "true" ? 1 : 0);
		exp->datatype.name = "bool";
		break;
	case Token::Type::String:
		exp->valStr = getMangledName(info.program->strings.size());
		info.program->strings.insert({ info.program->strings.size(), litToken.value });
		exp->datatype.ptrDepth = 1;
		exp->datatype.name = "u8";
		break;
	default:
		throw ProgGenError(litToken.pos, "Invalid literal type!");
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
		throw ProgGenError(litToken.pos, "Variable " + litToken.value + " not found!");

	exp->eType = pVar->isLocal ? Expression::ExprType::LocalVariable : Expression::ExprType::GlobalVariable;
	exp->isLValue = true;
	exp->localOffset = pVar->offset;
	exp->datatype = pVar->datatype;
	exp->globName = litToken.value;

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

	exp->datatype = { 1, "...#" + litToken.value};

	nextToken(info);

	return exp;
}

ExpressionRef getParseValue(ProgGenInfo& info)
{
	auto exp = getParseLiteral(info);
	if (exp) return exp;

	exp = getParseFunctionName(info);
	if (exp) return exp;

	exp = getParseVariable(info);
	if (exp) return exp;

	throw ProgGenError(peekToken(info).pos, "Expected variable name or literal value!");
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
		throw ProgGenError(parenClose.pos, "Expected ')'!");

	return exp;
}

ExpressionRef getParseBinaryExpression(ProgGenInfo& info, int precLvl)
{
	auto& opsLvl = opPrecLvls[precLvl];

	auto exp = getParseExpression(info, precLvl + 1);

	const Token* pOpToken = nullptr;
	std::map<std::string, Expression::ExprType>::iterator it;
	while ((it = opsLvl.ops.find((pOpToken = &peekToken(info))->value)) != opsLvl.ops.end())
	{
		{
			auto temp = std::make_shared<Expression>(pOpToken->pos);
			temp->left = exp;
			exp = temp;
		}
		exp->eType = it->second;
		nextToken(info);
		exp->right = getParseExpression(info, precLvl + 1);

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
			autoFixDatatypeMismatch(exp);
			if (!exp->left->isLValue)
				throw ProgGenError(exp->left->pos, "Cannot assign to non-lvalue!");
			exp->datatype = exp->left->datatype;
			exp->isLValue = true;
			break;
		case Expression::ExprType::Logical_OR:
		case Expression::ExprType::Logical_AND:
			exp->left = genConvertExpression(exp->left, { 0, "bool" });
			exp->right = genConvertExpression(exp->right, { 0, "bool" });
			exp->datatype = { 0, "bool" };
			exp->isLValue = false;
			break;
		case Expression::ExprType::Bitwise_OR:
		case Expression::ExprType::Bitwise_XOR:
		case Expression::ExprType::Bitwise_AND:
			autoFixDatatypeMismatch(exp);
			exp->datatype = exp->left->datatype;
			exp->isLValue = false;
			break;
		case Expression::ExprType::Comparison_Equal:
		case Expression::ExprType::Comparison_NotEqual:
		case Expression::ExprType::Comparison_Less:
		case Expression::ExprType::Comparison_LessEqual:
		case Expression::ExprType::Comparison_Greater:
		case Expression::ExprType::Comparison_GreaterEqual:
			exp->left = genConvertExpression(exp->left, { 0, "bool" });
			exp->right = genConvertExpression(exp->right, { 0, "bool" });
			exp->datatype = { 0, "bool" };
			exp->isLValue = false;
			break;
		case Expression::ExprType::Shift_Left:
		case Expression::ExprType::Shift_Right:
		case Expression::ExprType::Sum:
		case Expression::ExprType::Difference:
		case Expression::ExprType::Product:
		case Expression::ExprType::Quotient:
		case Expression::ExprType::Remainder:
			autoFixDatatypeMismatch(exp);
			exp->datatype = exp->left->datatype;
			exp->isLValue = false;
			break;
		default:
			throw ProgGenError(exp->pos, "Invalid binary operator!");
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
	while ((it = opsLvl.ops.find((pOpToken = &peekToken(info))->value)) != opsLvl.ops.end())
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
			exp->isLValue = true;
			if (exp->datatype.ptrDepth == 0)
				throw ProgGenError(exp->pos, "Cannot subscript non-pointer type!");
			if (exp->datatype == Datatype{ 1, "void" })
				throw ProgGenError(exp->pos, "Cannot subscript pointer to void type!");
			--exp->datatype.ptrDepth;
			exp->right = genConvertExpression(getParseExpression(info), { 0, "u64" });
			parseExpected(info, Token::Type::Separator, "]");
			break;
		case Expression::ExprType::FunctionCall:
		{
			exp->isLValue = false;
			if (exp->left->datatype.ptrDepth == 0)
				throw ProgGenError(exp->pos, "Cannot call non-function!");
			exp->paramSizeSum = 0;
			while (!isSeparator(peekToken(info), ")"))
			{
				exp->paramExpr.push_back(getParseExpression(info));
				exp->paramSizeSum += getDatatypePushSize(exp->paramExpr.back()->datatype);
				if (!isSeparator(peekToken(info), ")"))
					parseExpected(info, Token::Type::Separator, ",");
			}
			parseExpected(info, Token::Type::Separator, ")");

			if (exp->left->eType == Expression::ExprType::FunctionName)
			{
				auto overloads = getFunctions(info, exp->left->funcName);
				if (!overloads)
					throw ProgGenError(exp->pos, "Function '" + exp->left->funcName + "' not found!");
				auto func = getMatchingOverload(info, *overloads, exp->paramExpr);
				if (!func)
					throw ProgGenError(exp->pos, "No matching overload found for function '" + exp->left->funcName + "'!");
				exp->datatype = func->retType;
				exp->left->datatype = { 1, getSignature(func) };
				exp->left->funcName = getMangledName(exp->left->funcName, exp.get());
			}
			else
			{
				throw ProgGenError(exp->pos, "Not implemented yet!");
				// TODO: Check if signature matches (possibly through implicit conversions)
				exp->datatype = {}; // TODO: Get return type from exp->left->datatype
			}
		}
			break;
		case Expression::ExprType::Suffix_Increment:
		case Expression::ExprType::Suffix_Decrement:
			exp->datatype = exp->left->datatype;
			exp->isLValue = false;
			break;
		}
	}

	return exp;
}

Datatype getParseDatatype(ProgGenInfo& info);

ExpressionRef getParseUnaryPrefixExpression(ProgGenInfo& info, int precLvl)
{
	auto& opsLvl = opPrecLvls[precLvl];

	auto opToken = peekToken(info);
	auto it = opsLvl.ops.find(opToken.value);
	if (it == opsLvl.ops.end())
		return getParseExpression(info, precLvl + 1);

	auto exp = std::make_shared<Expression>(opToken.pos);
	exp->eType = it->second;
	nextToken(info);

	switch (exp->eType)
	{
	case Expression::ExprType::AddressOf:
		exp->left = getParseExpression(info, precLvl);
		exp->datatype = exp->left->datatype;
		++exp->datatype.ptrDepth;
		exp->isLValue = false;
		break;
	case Expression::ExprType::Dereference:
		exp->left = getParseExpression(info, precLvl);
		exp->datatype = exp->left->datatype;
		if (exp->datatype.ptrDepth == 0)
			throw ProgGenError(opToken.pos, "Cannot dereference a non-pointer!");
		if (exp->datatype == Datatype{ 1, "void" })
			throw ProgGenError(opToken.pos, "Cannot dereference pointer to void!");
		--exp->datatype.ptrDepth;
		exp->isLValue = true;
		break;
	case Expression::ExprType::Logical_NOT:
		exp->left = getParseExpression(info, precLvl);
		exp->datatype = { 0, "bool" };
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
	default:
		throw ProgGenError(opToken.pos, "Unknown unary prefix expression!");
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
	default: throw ProgGenError(peekToken(info).pos, "Unknown operator precedence level type!");
	}
}

bool parseExpression(ProgGenInfo& info)
{
	auto exp = getParseExpression(info);
	if (exp)
		info.program->body->push_back(exp);
	return !!exp;
}

bool parseExpression(ProgGenInfo& info, const Datatype& targetType)
{
	auto expr = getParseExpression(info);
	if (!expr)
		return false;

	if (expr->datatype != targetType)
		expr = genConvertExpression(expr, targetType);
	info.program->body->push_back(expr);

	return true;
}

Datatype getParseDatatype(ProgGenInfo& info)
{
	auto& typeToken = peekToken(info);
	if (!isBuiltinType(typeToken))
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

ExpressionRef getParseDeclDefVariable(ProgGenInfo& info, const Datatype& datatype, const std::string& name)
{
	Variable var;
	var.pos = peekToken(info).pos;
	var.name = name;
	var.datatype = datatype;

	addVariable(info, var);

	ExpressionRef initExpr = nullptr;

	if (isOperator(peekToken(info), "="))
	{
		nextToken(info, -1);
		initExpr = getParseExpression(info);
	}

	parseExpectedNewline(info);

	return initExpr;
}

void parseExpectedDeclDefVariable(ProgGenInfo& info, const Datatype& datatype, const std::string& name)
{
	if (datatype == Datatype{0, "void" })
		throw ProgGenError(peekToken(info).pos, "Cannot declare variable of type void!");

	auto initExpr = getParseDeclDefVariable(info, datatype, name);
	if (initExpr)
		info.program->body->push_back(initExpr);
}

void parseExpectedDeclDefFunction(ProgGenInfo& info, const Datatype& datatype, const std::string& name)
{
	FunctionRef func = std::make_shared<Function>();
	func->pos = peekToken(info).pos;
	func->body = std::make_shared<Body>();
	func->name = name;
	func->retType = datatype;

	parseExpected(info, Token::Type::Separator, "(");

	while (!isSeparator(peekToken(info), ")"))
	{
		Variable param;
		param.isLocal = true;

		param.datatype = getParseDatatype(info);
		if (!param.datatype)
			throw ProgGenError(peekToken(info).pos, "Expected datatype!");
		
		func->retOffset += getDatatypePushSize(param.datatype);
		param.offset = func->retOffset;

		if (!isIdentifier(peekToken(info)))
			throw ProgGenError(peekToken(info).pos, "Expected identifier!");

		param.name = nextToken(info).value;

		func->params.push_back(param);

		if (isSeparator(peekToken(info), ")"))
			continue;

		parseExpected(info, Token::Type::Separator, ",");
	}

	func->retOffset += getDatatypePushSize(func->retType);

	parseExpected(info, Token::Type::Separator, ")");

	if (isSeparator(peekToken(info), "..."))
	{
		func->isDefined = false;

		nextToken(info);
		parseExpectedNewline(info);
	}
	else
	{
		func->isDefined = true;

		parseExpectedColon(info);
		parseExpectedNewline(info);

		increaseIndent(info.indent);
		setTempBody(info, func->body);

		info.funcRetOffset = func->retOffset;
		info.funcRetType = func->retType;
		info.funcFrameSize = 0;

		info.localStack.push_back({});
		for (auto& param : func->params)
			info.localStack.back().locals.insert({ param.name, param });

		parseFunctionBody(info);
		func->frameSize = info.funcFrameSize;
		info.localStack.pop_back();
		unsetTempBody(info);
		decreaseIndent(info.indent);
	}

	addFunction(info, func);
}

bool parseDeclDef(ProgGenInfo& info, bool allowFunctions)
{
	auto& typeToken = peekToken(info);

	auto datatype = getParseDatatype(info);
	if (!datatype)
		return false;

	auto& nameToken = nextToken(info);
	if (!isIdentifier(nameToken))
		throw ProgGenError(nameToken.pos, "Expected identifier!");

	if (getDatatypeSize(datatype) < 0)
		throw ProgGenError(typeToken.pos, "Unknown datatype!");

	if (isSeparator(peekToken(info), "("))
	{
		if (!allowFunctions)
			throw ProgGenError(peekToken(info).pos, "Functions are not allowed here!");
		parseExpectedDeclDefFunction(info, datatype, nameToken.value);
	}
	else
	{
		parseExpectedDeclDefVariable(info, datatype, nameToken.value);
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

	info.program->body->push_back(std::make_shared<Statement>(retToken.pos, Statement::Type::Return));
	info.program->body->back()->funcRetOffset = info.funcRetOffset;

	if (info.funcRetType != Datatype{ 0, "void" })
		info.program->body->back()->subExpr = genConvertExpression(getParseExpression(info), info.funcRetType);

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
		throw ProgGenError(strToken.pos, "Expected assembly string!");

	parseExpectedNewline(info);

	info.program->body->push_back(std::make_shared<Statement>(asmToken.pos, Statement::Type::Assembly));
	info.program->body->back()->asmLines.push_back(preprocessAsmCode(info, strToken));


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
			throw ProgGenError(strToken.pos, "Expected assembly string!");

		parseExpectedNewline(info);

		info.program->body->push_back(std::make_shared<Statement>(asmToken.pos, Statement::Type::Assembly));
		info.program->body->back()->asmLines.push_back(preprocessAsmCode(info, strToken));
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

void parseFunctionBody(ProgGenInfo& info)
{
	int numStatements = 0;

	auto& bodyBeginToken = peekToken(info, -1);

	while (parseIndent(info))
	{
		++numStatements;
		auto token = (*info.tokens)[info.currToken];
		if (parseEmptyLine(info)) continue;
		if (parseStatementPass(info)) continue;
		if (parseDeclDef(info, false)) continue;
		if (parseStatementReturn(info)) continue;
		if (parseSinglelineAssembly(info)) continue;
		if (parseMultilineAssembly(info)) continue;
		if (parseExpression(info)) continue;
		throw ProgGenError(token.pos, "Unexpected token: " + token.value + "!");
	}

	if (numStatements == 0)
		throw ProgGenError(bodyBeginToken.pos, "Expected function body!");

	if (info.program->body->empty() || info.program->body->back()->type != Statement::Type::Return)
	{
		if (info.funcRetType == Datatype{ 0, "void" })
			info.program->body->push_back(std::make_shared<Statement>(Token::Position(), Statement::Type::Return));
		else
			throw ProgGenError(info.program->body->empty() ? bodyBeginToken.pos : info.program->body->back()->pos, "Missing return statement!");
	}
}

bool parseStatementImport(ProgGenInfo& info);

void parseGlobalCode(ProgGenInfo& info)
{
	while (info.currToken < info.tokens->size())
	{
		if (!parseIndent(info))
			throw ProgGenError(peekToken(info).pos, "Expected indent!");
		auto token = (*info.tokens)[info.currToken];
		if (parseEmptyLine(info)) continue;
		if (parseStatementPass(info)) continue;
		if (parseStatementImport(info)) continue;
		if (parseDeclDef(info, true)) continue;
		if (parseSinglelineAssembly(info)) continue;
		if (parseMultilineAssembly(info)) continue;
		if (parseExpression(info)) continue;
		throw ProgGenError(token.pos, "Unexpected token: " + token.value + "!");
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
		throw ProgGenError(fileToken.pos, "Expected file path!");

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
		throw ProgGenError(fileToken.pos, "Import file not found: '" + fileToken.value + "'!");

	std::string code = readTextFile(path);

	auto backup = makeProgGenInfoBackup(info);

	info.tokens = tokenize(code, path);
	info.currToken = 0;
	parseGlobalCode(info);

	loadProgGenInfoBackup(info, backup);

	return true;
}

ProgramRef generateProgram(const TokenListRef tokens, const std::set<std::string>& importDirs)
{
	ProgGenInfo info = { tokens, ProgramRef(new Program()), importDirs };
	info.program->body = std::make_shared<Body>();

	parseGlobalCode(info);

	return info.program;
}