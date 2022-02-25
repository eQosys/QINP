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

	int globOffset = 0;
};

typedef std::map<std::string, Expression::ExprType> OpPrecLvl;

std::vector<OpPrecLvl> opPrecLvls = 
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
	{
		{ "||", Expression::ExprType::Logical_OR },
	},
	{
		{ "&&", Expression::ExprType::Logical_AND },
	},
	{
		{ "|", Expression::ExprType::Bitwise_OR },
	},
	{
		{ "^", Expression::ExprType::Bitwise_XOR },
	},
	{
		{ "&", Expression::ExprType::Bitwise_AND },
	},
	{
		{ "==", Expression::ExprType::Equality_Equal },
		{ "!=", Expression::ExprType::Equality_NotEqual },
	},
	{
		{ "<", Expression::ExprType::Relational_Less },
		{ "<=", Expression::ExprType::Relational_LessEqual },
		{ ">", Expression::ExprType::Relational_Greater },
		{ ">=", Expression::ExprType::Relational_GreaterEqual },
	},
	{
		{ "<<", Expression::ExprType::Shift_Left },
		{ ">>", Expression::ExprType::Shift_Right },
	},
	{
		{ "+", Expression::ExprType::Sum },
		{ "-", Expression::ExprType::Difference },
	},
	{
		{ "*", Expression::ExprType::Product },
		{ "/", Expression::ExprType::Quotient },
		{ "%", Expression::ExprType::Remainder },
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

void parseExpectedNewline(ProgGenInfo& info)
{
	auto& token = nextToken(info);
	if (!isNewline(token) && !isEndOfCode(token))
		throw ProgGenError(token.pos, "Expected newline!");
}

void parseExpectedColon(ProgGenInfo& info)
{
	auto& token = nextToken(info);
	if (!isSeparator(token, ":"))
		throw ProgGenError(token.pos, "Expected colon!");
}

Datatype getBestConvDatatype(const ExpressionRef left, const ExpressionRef right)
{
	if (left->datatype.ptrDepth > 0 || right->datatype.ptrDepth > 0)
		return Datatype();

	if (!isBuiltinType(left->datatype.name) || !isBuiltinType(right->datatype.name))
		return Datatype();
	
	static const std::vector<std::string> typeOrder = 
	{
		"u8", "u16", "u32", "u64",
	};
	
	auto leftIt = std::find(typeOrder.begin(), typeOrder.end(), left->datatype.name);
	auto rightIt = std::find(typeOrder.begin(), typeOrder.end(), right->datatype.name);
	if (leftIt == typeOrder.end() || rightIt == typeOrder.end())
		return Datatype();

	return (leftIt > rightIt) ? left->datatype : right->datatype;
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

ExpressionRef getParsePostfixExpression(ProgGenInfo& info)
{
	// TODO: Implement correct PostfixExpression parsing
	auto exp = getParseLiteral(info);
	if (exp) return exp;

	exp = getParseVariable(info);
	if (exp) return exp;

	throw ProgGenError(peekToken(info).pos, "Expected variable name or literal value!");
}

ExpressionRef getParsePrefixExpression(ProgGenInfo& info)
{
	// TODO: Implement correct PrefixExpression parsing

	auto& parenOpen = peekToken(info);
	if (!isSeparator(parenOpen, "("))
		return getParsePostfixExpression(info);
	
	nextToken(info);
	auto exp = getParseExpression(info);

	auto& parenClose = nextToken(info);
	if (!isSeparator(parenClose, ")"))
		throw ProgGenError(parenClose.pos, "Expected ')'!");

	return exp;
}

ExpressionRef genConvertExpression(ExpressionRef expToConvert, const Datatype& newDatatype)
{
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
	
	if (newDatatype != exp->left->datatype)
		exp->left = genConvertExpression(exp->left, newDatatype);
	if (newDatatype != exp->right->datatype)
		exp->right = genConvertExpression(exp->right, newDatatype);
}

ExpressionRef getParseExpression(ProgGenInfo& info, int precLvl)
{
	static const int maxPrecLvl = opPrecLvls.size() - 1;

	if (precLvl > maxPrecLvl)
		return getParsePrefixExpression(info);

	auto exp = getParseExpression(info, precLvl + 1);

	auto& opsLvl = opPrecLvls[precLvl];

	const Token* pOpToken = nullptr;
	std::map<std::string, Expression::ExprType>::iterator it;
	while ((it = opsLvl.find((pOpToken = &peekToken(info))->value)) != opsLvl.end())
	{
		auto temp = std::make_shared<Expression>(pOpToken->pos);
		temp->left = exp;
		temp->eType = it->second;
		nextToken(info);
		temp->right = getParseExpression(info, precLvl + 1);
		autoFixDatatypeMismatch(temp);
		temp->datatype = temp->left->datatype;
		exp = temp;
	}

	return exp;
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

bool parseDeclDef(ProgGenInfo& info)
{
	auto& typeToken = peekToken(info);
	if (!isBuiltinType(typeToken))
		return false;
	nextToken(info);

	Variable var;
	var.datatype.name = typeToken.value;

	while (isOperator(peekToken(info), "*"))
	{
		nextToken(info);
		++var.datatype.ptrDepth;
	}

	auto& nameToken = nextToken(info);
	if (!isIdentifier(nameToken))
		throw ProgGenError(nameToken.pos, "Expected identifier!");

	var.offset = info.globOffset;
	int varSize = getDatatypeSize(var.datatype);
	if (varSize < 0)
		throw ProgGenError(typeToken.pos, "Invalid datatype!");
	info.globOffset += varSize;

	info.program->globals.insert({ nameToken.value, var });

	if (isOperator(peekToken(info), "="))
	{
		nextToken(info, -1);
		parseExpression(info);
	}

	parseExpectedNewline(info);

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