#include "ProgramGenerator.h"

#include <map>

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
};

struct OpPrecLvl // OperatorPrecedenceLevel
{
	std::map<std::string, Expression::ExprType> ops;
	enum class Assoc // Associativity
	{
		LeftToRight,
		RightToLeft,
	} assoc;
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
		OpPrecLvl::Assoc::RightToLeft
	},
	{
		{
			{ "||", Expression::ExprType::Logical_OR },
		},
		OpPrecLvl::Assoc::LeftToRight
	},
	{
		{
			{ "&&", Expression::ExprType::Logical_AND },
		},
		OpPrecLvl::Assoc::LeftToRight
	},
	{
		{
			{ "|", Expression::ExprType::Bitwise_OR },
		},
		OpPrecLvl::Assoc::LeftToRight
	},
	{
		{
			{ "^", Expression::ExprType::Bitwise_XOR },
		},
		OpPrecLvl::Assoc::LeftToRight
	},
	{
		{
			{ "&", Expression::ExprType::Bitwise_AND },
		},
		OpPrecLvl::Assoc::LeftToRight
	},
	{
		{
			{ "==", Expression::ExprType::Equality_Equal },
			{ "!=", Expression::ExprType::Equality_NotEqual },
		},
		OpPrecLvl::Assoc::LeftToRight
	},
	{
		{
			{ "<", Expression::ExprType::Relational_Less },
			{ "<=", Expression::ExprType::Relational_LessEqual },
			{ ">", Expression::ExprType::Relational_Greater },
			{ ">=", Expression::ExprType::Relational_GreaterEqual },
		},
		OpPrecLvl::Assoc::LeftToRight
	},
	{
		{
			{ "<<", Expression::ExprType::Shift_Left },
			{ ">>", Expression::ExprType::Shift_Right },
		},
		OpPrecLvl::Assoc::LeftToRight
	},
	{
		{
			{ "+", Expression::ExprType::Sum },
			{ "-", Expression::ExprType::Difference },
		},
		OpPrecLvl::Assoc::LeftToRight
	},
	{
		{
			{ "*", Expression::ExprType::Product },
			{ "/", Expression::ExprType::Quotient },
			{ "%", Expression::ExprType::Remainder },
		},
		OpPrecLvl::Assoc::LeftToRight
	},
};

const Token& peekToken(ProgGenInfo& info, int offset = 0)
{
	static const Token emptyToken = { { "", 0, 0 }, Token::Type::EndOfCode, "" };
	int index = info.currToken + offset;
	return (index < info.tokens.size()) ? info.tokens[index] : emptyToken;
}

const Token& nextToken(ProgGenInfo& info)
{
	auto& temp = peekToken(info, 0);
	++info.currToken;
	return temp;
}

void parseExpectedNewline(ProgGenInfo& info)
{
	auto& token = nextToken(info);
	if (token.type != Token::Type::Newline && token.type != Token::Type::EndOfCode)
		throw ProgGenError(token.pos, "Expected newline!");
}

ExpressionRef parseExpression(ProgGenInfo& info, int precLvl = 0);

ExpressionRef parseLiteral(ProgGenInfo& info)
{
	auto& litToken = peekToken(info);
	if (litToken.type != Token::Type::Literal)
		throw ProgGenError(litToken.pos, "Expected literal!");

	nextToken(info);
	ExpressionRef exp = std::make_shared<Expression>(litToken.pos);
	exp->eType = Expression::ExprType::Literal;
	exp->valIntSigned = std::stoi(litToken.value);

	return exp;
}

ExpressionRef parseAfterDefaultBinaryPrec(ProgGenInfo& info)
{
	ExpressionRef exp = nullptr;
	auto& parenOpen = peekToken(info);
	if (parenOpen.type == Token::Type::Separator && parenOpen.value == "(")
	{
		nextToken(info);
		exp = parseExpression(info);

		auto& parenClose = nextToken(info);
		if (parenClose.type != Token::Type::Separator || parenClose.value != ")")
			throw ProgGenError(parenClose.pos, "Expected ')'!");
	}
	else
	{
		exp = parseLiteral(info);
	}
	return exp;
}

ExpressionRef parseExpression(ProgGenInfo& info, int precLvl)
{
	static const int maxPrecLvl = opPrecLvls.size() - 1;

	if (precLvl > maxPrecLvl)
		return parseAfterDefaultBinaryPrec(info);

	auto exp = parseExpression(info, precLvl + 1);

	auto& opsLvl = opPrecLvls[precLvl];

	const Token* pOpToken = nullptr;
	std::map<std::string, Expression::ExprType>::iterator it;
	while ((it = opsLvl.ops.find((pOpToken = &peekToken(info))->value)) != opsLvl.ops.end())
	{
		auto temp = std::make_shared<Expression>(pOpToken->pos);
		temp->left = exp;
		temp->eType = it->second;
		nextToken(info);
		temp->right = parseExpression(info, precLvl + 1);
		exp = temp;
	}
	return exp;
}

bool parseStatementExit(ProgGenInfo& info)
{
	auto& exitToken = peekToken(info);
	if (exitToken.type != Token::Type::Keyword || exitToken.value != "exit")
		return false;

	nextToken(info);
	auto& exprBegin = peekToken(info);

	auto exp = parseExpression(info);
	info.program->body.push_back(exp);

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
		if (parseStatementExit(info)) continue;
		throw ProgGenError(token.pos, "Unexpected token: " + token.value);
	}

	return info.program;
}