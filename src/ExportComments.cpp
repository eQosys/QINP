#include "ExportComments.h"

#include <iostream>
#include <cassert>

std::string replace(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

void exportComments(TokenListRef tokens, std::ostream& out)
{
	out << "{ \"comments\": [";
	for (const Token& token : *tokens)
	{
		assert(token.type == Token::Type::Comment && "Tokenlist should only contain comments!");

		out << "{ \"pos\": \"" << token.pos << "\"";

		out << ", \"text\": \"" << replace(replace(token.value, "\\", "\\\\"), "\"", "\\\"") << "\"";

		out << "},";
	}

	out << "] }";
}