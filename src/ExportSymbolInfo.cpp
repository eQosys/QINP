#include "ExportSymbolInfo.h"

void exportPosition(Token::Position pos, std::ostream& out)
{
	out << "\"file\": \"" << pos.file << "\",";
	out << "\"line\": " << pos.line << ",";
	out << "\"col\": " << pos.column;
}

// Export the symbol info to a text file with the json format.
void exportSymbolInfo(SymbolRef root, std::ostream& out)
{
	out << "{";

	out << "\"name\": \"" << root->name << "\",";

	out << "\"pos\": {";
	exportPosition(root->pos, out);
	out << "},";

	out << "\"type\": \"" << SymTypeToString(root->type) << "\",";

	out << "\"state\": \"" << SymStateToString(root->state) << "\",";

	out << "\"subSymbols\": [";
	{
		auto it = root->subSymbols.begin();
		while (it != root->subSymbols.end())
		{
			exportSymbolInfo(it->second, out);
			if (++it != root->subSymbols.end())
				out << ",";
		}
	}
	out << "]";

	// TODO: Export Type specific info.

	out << "}";
}