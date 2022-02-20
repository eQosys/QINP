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

bool parseExpression(ProgGenInfo& info)
{
	auto& token = peekToken(info);
	switch (token.type)
	{
		case Token::Type::Literal:
			info.program->body.push_back(std::make_shared<LiteralExpression>(token.value));
			nextToken(info);
			parseExpectedNewline(info);
			break;
		default:
			return false;
	}
	return true;
}

bool parseStatementExit(ProgGenInfo& info)
{
	auto& exitToken = peekToken(info, 0);
	if (exitToken.type != Token::Type::Keyword || exitToken.value != "exit")
		return false;

	nextToken(info);

	if (!parseExpression(info))
		throw ProgGenError(peekToken(info).pos, "Expected expression after exit keyword!");

	info.program->body.push_back(std::make_shared<ReturnStatement>());

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