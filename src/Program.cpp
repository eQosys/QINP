#include "Program.h"

#include "errors/QinpError.h"
#include "utility/FileReader.h"
#include "utility/StringHelpers.h"
#include "grammar/QinpGrammarCStr.h"

Program::Program(bool verbose)
    : m_verbose(verbose),
    m_grammar(
        qrawlr::Grammar::load_from_text(
            QINP_GRAMMAR_C_STR,
            QINP_GRAMMAR_ORIGIN_C_STR
        )
    )
{
    ;
}

void Program::add_import_directory(const std::string& path_str)
{
    auto path = std::filesystem::path(path_str);
    if (!std::filesystem::is_directory(path))
        throw QinpError("Import directory '" + path_str + "' does not exist");
    m_import_dirs.push_back(path);
}

void Program::import_source_file(std::string path_str, const std::string& import_from_dir_str, bool skip_duplicate, bool ignore_import_dirs)
{
    std::filesystem::path path(path_str);
    if (!path.is_absolute())
    {
        bool found_base = false;

        // try importing relative to "importer"
        try {
            path_str = std::filesystem::canonical(import_from_dir_str / path).generic_string();
            found_base = true;
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            ;
        }

        // when not found, import from specified import directories (stdlib first element, check last)
        if (!found_base && !ignore_import_dirs)
        {
            for (auto it = m_import_dirs.rbegin(); it != m_import_dirs.rend(); ++it)
            {
                auto base = *it;
                try {
                    path_str = std::filesystem::canonical(base / path).generic_string();
                    found_base = true;
                }
                catch (const std::filesystem::filesystem_error& e)
                {
                    continue;
                }
            }
        }

        if (!found_base)
            throw QinpError("Could not locate source file '" + path_str + "'");
    }

    if (skip_duplicate && m_imported_files.find(path_str) != m_imported_files.end())
        return; // skip duplicate import
    
    std::string code_str = read_file(path_str);

    m_imported_files.insert(path_str);
    import_source_code(code_str, path_str);
}

void Program::import_source_code(const std::string& code_str, const std::string& path_str)
{
    // Parse source code with Qrawlr
    // qrawlr::GrammarException handled by main function
    qrawlr::MatchResult result = m_grammar.apply_to(code_str, "GlobalCode", path_str);
    if ((size_t)result.pos_end.index < code_str.size())
        throw qrawlr::GrammarException("Could not parse remaining source", result.pos_end.to_string(path_str));

    // TODO: remove debug tree graphviz + render
    system("mkdir -p out/");
    qrawlr::write_file("out/tree.gv", result.tree->to_digraph_str(m_verbose));
    system("dot -Tpdf -o out/tree.pdf out/tree.gv");

    // TODO: Proper backup
    auto backup_translation_unit_path = m_current_translation_unit_path;
    handle_tree_node(result.tree, "GlobalCode", nullptr);
    m_current_translation_unit_path = backup_translation_unit_path;
}

void Program::handle_tree_node(qrawlr::ParseTreeRef tree, const std::string& name, void* pData)
{
    handle_tree_node_one_of(tree, { name }, pData);
}

void Program::handle_tree_node_one_of(qrawlr::ParseTreeRef tree, const std::set<std::string>& names, void* pData)
{
    auto node = qrawlr::expect_node(tree);

    // Check if nodes' name matches any of the given names
    if (names.find(node->get_name()) == names.end())
        throw qrawlr::GrammarException("[*handle_tree_node_one_of*]: Node with name '" + node->get_name() + "' does not match any of the given names: [ " + join(names, ", ") + "]", node->get_pos_begin().to_string(m_current_translation_unit_path));

    static const std::map<std::string, Handler> handlers = {
        { "GlobalCode", handle_tree_node_global_code },
        { "String",     handle_tree_node_string },
    };

    // Get the corresponding handler
    auto it = handlers.find(node->get_name());
    if (it == handlers.end())
        throw QinpError("[*handle_tree_node_one_of*]: Unhandled node name '" + node->get_name() + "'");
    Handler handler = it->second;

    // Call the handler
    (this->*handler)(node, pData);
}

void Program::handle_tree_node_global_code(qrawlr::ParseTreeNodeRef node, void* pData)
{
    static const std::set<std::string> valid_children = {
        "Comment",
        "StatementPass", "StatementDefer",
        "StatementImport", "StatementSpace",
        "StatementReturn", "StatementDefine",
        "StatementWhile", "StatementDoWhile",
        "StatementIfElifElse",
        "StatementPackDeclDef", "StatementEnumDeclDef",
        "StatementFunctionDeclDef", "StatementExternFunctionDecl",
        "StatementVariableDeclDef",
        "StatementInlineAssembly",
        "Expression"
    };

    for (auto child : node->get_children())
        handle_tree_node_one_of(child, valid_children, nullptr);
}

void Program::handle_tree_node_string(qrawlr::ParseTreeNodeRef node, void* pString)
{
    std::string& string_out = *(std::string*)pString;
    string_out.clear();

    static const std::map<char, char> sequences = {
        { '0',  '\0' },
        { 'a',  '\a' },
        { 'b',  '\b' },
        { 'e',  '\e' },
        { 'f',  '\f' },
        { 'n',  '\n' },
        { 'r',  '\r' },
        { 't',  '\t' },
        { 'v',  '\v' },
        { '\\', '\\' },
        { '\'', '\'' },
        { '"',  '"'  },
    };

    for (auto child : node->get_children())
    {
        if (qrawlr::is_leaf(child))
        {
            string_out.append(qrawlr::get_leaf(child)->get_value());
            continue;
        }

        std::string seq = qrawlr::expect_leaf(qrawlr::expect_node(child, "EscapeSequence"))->get_value();
        if (seq[0] == 'x')
        {
            string_out.push_back((char)std::stoi(seq.substr(1), nullptr, 16));
        }
        else
        {
            auto it = sequences.find(seq[0]);
            if (it == sequences.end())
                throw qrawlr::GrammarException("Unknown escape sequence", child->get_pos_begin().to_string(m_current_translation_unit_path));
            string_out.push_back(it->second);
        }
    }
}