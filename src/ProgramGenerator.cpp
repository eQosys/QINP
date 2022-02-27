#include "ProgramGenerator.h"

#include <map>
#include <set>
#include <algorithm>

#include "Errors/ProgGenError.h"

struct ProgGenInfo
{
	const TokenList& tokens;
	ProgramRef program;
	int currToken = 0;
	struct Indent
	{
		int lvl = 0;
		int nPerLvl = 0;
		std::string chStr = "";
	} indent;
};

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
		},
		OpPrecLvl::Type::Unary_Prefix,
	},
	{
		{
			{ "[", Expression::ExprType::Subscript },
			//{ "(", Expression::ExprType::FunctionCall },
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
	return token.type == Token::Type::Literal;
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
	return (index < info.tokens.size()) ? info.tokens[index] : emptyToken;
}

const Token& nextToken(ProgGenInfo& info, int offset = 1)
{
	auto& temp = peekToken(info, offset - 1);
	info.currToken += offset;
	return temp;
}

const Variable& getVariable(ProgGenInfo& info, const std::string& name, const Token::Position& tokenPos)
{
	auto& program = info.program;
	auto& variables = program->globals;
	auto it = variables.find(name);
	if (it == variables.end())
		throw ProgGenError(tokenPos, "Unknown variable: " + name + "!");
	return it->second;
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

			auto& var = getVariable(info, varName, asmToken.pos);
			if (var.isLocal)
				throw ProgGenError(asmToken.pos, "Local variable cannot be used in asm code: " + varName + "!");
				
			result += varName;
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
	if (info.indent.chStr.empty() && info.indent.lvl > 0)
	{
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

	for (int i = 0; i < info.indent.nPerLvl * info.indent.lvl; ++i)
	{
		auto& token = peekToken(info);
		if (!isWhitespace(token) || token.value != info.indent.chStr)
			return false;
		nextToken(info);
	}

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

ExpressionRef genConvertExpression(ExpressionRef expToConvert, const Datatype& newDatatype)
{
	if (expToConvert->datatype == newDatatype)
		return expToConvert;

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

	if (newDatatype.name.empty())
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
	exp->valIntUnsigned = std::stoull(litToken.value);
	exp->datatype.name = "u64";

	return exp;
}

ExpressionRef getParseVariable(ProgGenInfo& info)
{
	auto& litToken = peekToken(info);
	if (!isIdentifier(litToken))
		return nullptr;
	
	nextToken(info);
	ExpressionRef exp = std::make_shared<Expression>(litToken.pos);

	auto& var = getVariable(info, litToken.value, litToken.pos);
	exp->eType = Expression::ExprType::GlobalVariable; // exp->eType = var.isLocal ? Expression::ExprType::LocalVariable : Expression::ExprType::GlobalVariable;
	exp->isLValue = true;
	exp->offset = var.offset;
	exp->datatype = var.datatype;
	exp->globName = litToken.value;

	return exp;
}

ExpressionRef getParseValue(ProgGenInfo& info)
{
	auto exp = getParseLiteral(info);
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
	auto exp = getParseParenthesized(info);
	
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
			--exp->datatype.ptrDepth;
			exp->right = genConvertExpression(getParseExpression(info), { 0, "u64" });
			parseExpected(info, Token::Type::Separator, "]");
			break;
		case Expression::ExprType::FunctionCall:
			throw ProgGenError(exp->pos, "Function call not supported!");
		case Expression::ExprType::Suffix_Increment:
		case Expression::ExprType::Suffix_Decrement:
			exp->datatype = exp->left->datatype;
			exp->isLValue = false;
		}
	}

	return exp;
}

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
	exp->left = getParseExpression(info, precLvl);

	switch (exp->eType)
	{
	case Expression::ExprType::AddressOf:
		exp->datatype = exp->left->datatype;
		++exp->datatype.ptrDepth;
		exp->isLValue = false;
		break;
	case Expression::ExprType::Dereference:
		exp->datatype = exp->left->datatype;
		if (exp->datatype.ptrDepth == 0)
			throw ProgGenError(opToken.pos, "Cannot dereference a non-pointer!");
		--exp->datatype.ptrDepth;
		exp->isLValue = true;
		break;
	case Expression::ExprType::Logical_NOT:
		exp->datatype = { 0, "bool" };
		exp->isLValue = false;
	case Expression::ExprType::Bitwise_NOT:
		exp->datatype = exp->left->datatype;
		exp->isLValue = false;
		break;
	case Expression::ExprType::Prefix_Plus:
	case Expression::ExprType::Prefix_Minus:
	case Expression::ExprType::Prefix_Increment:
	case Expression::ExprType::Prefix_Decrement:
		exp->datatype = exp->left->datatype;
		exp->isLValue = false;
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
		info.program->body.push_back(exp);
	return !!exp;
}

bool parseExpression(ProgGenInfo& info, const Datatype& targetType)
{
	auto expr = getParseExpression(info);
	if (!expr)
		return false;

	if (expr->datatype != targetType)
		expr = genConvertExpression(expr, targetType);
	info.program->body.push_back(expr);

	return true;
}

ExpressionRef getParseDeclDefVariable(ProgGenInfo& info, const Datatype& datatype, const std::string& name)
{
	Variable var;
	var.datatype = datatype;
	var.isLocal = false;

	info.program->globals.insert({ name, var });

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
	auto initExpr = getParseDeclDefVariable(info, datatype, name);
	if (initExpr)
		info.program->body.push_back(initExpr);
}

void parseExpectedDeclDefFunction(ProgGenInfo& info, const Datatype& datatype, const std::string& name)
{
	throw ProgGenError(peekToken(info).pos, "Function declarations/definitions not supported!");
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

bool parseDeclDef(ProgGenInfo& info)
{
	auto& typeToken = peekToken(info);

	auto datatype = getParseDatatype(info);
	if (!datatype)
		return false;

	auto& nameToken = nextToken(info);
	if (!isIdentifier(nameToken))
		throw ProgGenError(nameToken.pos, "Expected identifier!");

	if (getDatatypeSize(datatype) < 0)
		throw ProgGenError(typeToken.pos, "Invalid datatype!");
\
	if (isSeparator(peekToken(info), "("))
		parseExpectedDeclDefFunction(info, datatype, nameToken.value);
	else
		parseExpectedDeclDefVariable(info, datatype, nameToken.value);

	return true;
}

bool parseStatementExit(ProgGenInfo& info)
{
	auto& exitToken = peekToken(info);
	if (!isKeyword(exitToken, "exit"))
		return false;

	nextToken(info);
	auto& exprBegin = peekToken(info);

	parseExpression(info, { 0, "u64" });

	parseExpectedNewline(info);

	info.program->body.push_back(std::make_shared<Statement>(exitToken.pos, Statement::Type::Exit));

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

	info.program->body.push_back(std::make_shared<Statement>(asmToken.pos, Statement::Type::Assembly));
	info.program->body.back()->asmLines.push_back(preprocessAsmCode(info, strToken));


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

		info.program->body.push_back(std::make_shared<Statement>(asmToken.pos, Statement::Type::Assembly));
		info.program->body.back()->asmLines.push_back(preprocessAsmCode(info, strToken));
	}

	decreaseIndent(info.indent);

	return true;
}

ProgramRef generateProgram(const TokenList& tokens)
{
	ProgGenInfo info = { tokens, ProgramRef(new Program()) };

	while (info.currToken < tokens.size())
	{
		if (!parseIndent(info))
			throw ProgGenError(peekToken(info).pos, "Inconsistent indentation!");
		auto token = info.tokens[info.currToken];
		if (parseEmptyLine(info)) continue;
		if (parseDeclDef(info)) continue;
		if (parseStatementExit(info)) continue;
		if (parseSinglelineAssembly(info)) continue;
		if (parseMultilineAssembly(info)) continue;
		if (parseExpression(info)) continue;
		throw ProgGenError(token.pos, "Unexpected token: " + token.value + "!");
	}

	return info.program;
}