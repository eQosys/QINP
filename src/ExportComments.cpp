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

void exportComments(CommentTokenMapRef comments, std::ostream& out)
{
	out << "{ \"comments\": {";
	int i = 0;
	for (const auto& [file, tokens] : *comments)
	{
		out << "\"" << file << "\": {";

		int j = 0;
		for (const auto& [line, text] : tokens)
		{
		
			out << "\"" <<  line << "\": \"" << replace(replace(text, "\\", "\\\\"), "\"", "\\\"") << "\"";
			
			if (++j < tokens.size())
				out << ",";
		}

		if (++i < comments->size())
			out << "},";
		else
			out << "}";
	}

	out << "} }";
}