#include "ProgramGenerator.h"

#include <map>
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
		char ch = 0;
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

int getBuiltinTypeSize(const std::string& name)
{
	static const std::map<std::string, int> sizes =
	{
		//{ "i8", 1 },
		//{ "i16", 2 },
		//{ "i32", 4 },
		//{ "i64", 8 },
		{ "u8", 1 },
		{ "u16", 2 },
		{ "u32", 4 },
		{ "u64", 8 },
		//{ "f32", 4 },
		//{ "f64", 8 },
	};

	auto it = sizes.find(name);
	return (it != sizes.end()) ? it->second : -1;
}

int getDatatypeSize(ProgGenInfo& info, const Datatype& datatype)
{
	if (datatype.ptrDepth > 0)
		return sizeof(void*);
	int size = getBuiltinTypeSize(datatype.name);
	if (size > 0)
		return size;

	throw ProgGenError(peekToken(info).pos, "Unknown datatype: " + datatype.name);
}

const Variable& getVariable(ProgGenInfo& info, const std::string& name)
{
	auto& program = info.program;
	auto& variables = program->globals;
	auto it = variables.find(name);
	if (it == variables.end())
		throw ProgGenError(peekToken(info).pos, "Unknown variable: " + name);
	return it->second;
}

void parseExpectedNewline(ProgGenInfo& info)
{
	auto& token = nextToken(info);
	if (!isNewline(token) && !isEndOfCode(token))
		throw ProgGenError(token.pos, "Expected newline!");
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

	auto& var = getVariable(info, litToken.value);
	exp->eType = Expression::ExprType::GlobalVariable; // exp->eType = var.isLocal ? Expression::ExprType::LocalVariable : Expression::ExprType::GlobalVariable;
	exp->isLValue = true;
	exp->offset = var.offset;
	exp->datatype = var.datatype;

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

void autoFixDatatypeMismatch(ExpressionRef exp)
{
	// TODO: Disable automatic (left) conversion for some ExprTypes (e.g. assign)
	if (exp->left->datatype == exp->right->datatype)
		return;

	Datatype newDatatype = getBestConvDatatype(exp->left, exp->right);
	if (newDatatype.name.empty())
		throw ProgGenError(
			exp->pos, "Cannot convert between " +
			exp->left->datatype.name +
			"*" + std::to_string(exp->left->datatype.ptrDepth) + " and " +
			exp->right->datatype.name +
			"*" + std::to_string(exp->right->datatype.ptrDepth));
	
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
	info.globOffset += getDatatypeSize(info, var.datatype);

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

	parseExpression(info);

	parseExpectedNewline(info);

	info.program->body.push_back(std::make_shared<Statement>(exitToken.pos, Statement::Type::Exit));

	return true;
}

ProgramRef generateProgram(const TokenList& tokens)
{
	ProgGenInfo info = { tokens, ProgramRef(new Program()) };

	while (info.currToken < tokens.size())
	{
		auto token = info.tokens[info.currToken];
		if (parseDeclDef(info)) continue;
		if (parseStatementExit(info)) continue;
		if (parseExpression(info)) continue;
		throw ProgGenError(token.pos, "Unexpected token: " + token.value);
	}

	return info.program;
}