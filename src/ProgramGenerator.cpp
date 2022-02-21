#include "ProgramGenerator.h"

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

bool parseExpectedNewline(ProgGenInfo& info)
{
	auto& token = nextToken(info);
	if (token.type != Token::Type::Newline && token.type != Token::Type::EndOfCode)
		throw ProgGenError(token.pos, "Expected newline!");
	return true;
}

ExpressionRef parseExpression(ProgGenInfo& info);

ExpressionRef parseLiteral(ProgGenInfo& info)
{
	auto& litToken = peekToken(info);
	if (litToken.type != Token::Type::Literal)
		return nullptr;

	nextToken(info);
	ExpressionRef exp = std::make_shared<Expression>(litToken.pos);
	exp->eType = Expression::ExprType::Literal;
	exp->value = std::stoi(litToken.value);

	return exp;
}

ExpressionRef parseFactor(ProgGenInfo& info)
{
	ExpressionRef exp = nullptr;
	auto& parenOpen = peekToken(info);
	if (parenOpen.type == Token::Type::Separator && parenOpen.value == "(")
	{
		nextToken(info);
		exp = parseExpression(info);
		if (!exp) return nullptr;

		auto& parenClose = peekToken(info);
		if (parenClose.type != Token::Type::Separator || parenClose.value != ")")
			throw ProgGenError(parenClose.pos, "Expected ')'!");
		nextToken(info);
	}
	else
	{
		exp = parseLiteral(info);
	}
	return exp;
}

ExpressionRef parseSummand(ProgGenInfo& info)
{
	ExpressionRef exp = std::make_shared<Expression>(Token::Position());
	exp->left = parseFactor(info);
	if (!exp->left) return nullptr;

	auto& opToken = peekToken(info);
	exp->pos = opToken.pos;
	if (opToken.type == Token::Type::Operator && (opToken.value == "*" || opToken.value == "/"))
	{
		exp->eType = (opToken.value == "*") ? Expression::ExprType::Product : Expression::ExprType::Division;
		nextToken(info);
		exp->right = parseFactor(info);
		if (!exp->right) return nullptr;
		return exp;
	}
	return exp->left;
}

ExpressionRef parseExpression(ProgGenInfo& info)
{
	auto exp = std::make_shared<Expression>(Token::Position());
	exp->left = parseSummand(info);
	if (!exp->left) return nullptr;
	
	auto& opToken = peekToken(info);
	exp->pos = opToken.pos;
	if (opToken.type == Token::Type::Operator && (opToken.value == "+" || opToken.value == "-"))
	{
		exp->eType = opToken.value == "+" ? Expression::ExprType::Sum : Expression::ExprType::Difference;
		nextToken(info);
		exp->right = parseSummand(info);
		if (!exp->right) return nullptr;
		return exp;
	}
	return exp->left;
}

bool parseStatementExit(ProgGenInfo& info)
{
	auto& exitToken = peekToken(info, 0);
	if (exitToken.type != Token::Type::Keyword || exitToken.value != "exit")
		return false;

	nextToken(info);

	auto exp = parseExpression(info);
	if (!exp)
		throw ProgGenError(exitToken.pos, "Expected expression after 'exit'!");
	info.program->body.push_back(exp);

	info.program->body.push_back(std::make_shared<ReturnStatement>(exitToken.pos));

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