#include "Program.h"

#include "errors/QinpError.h"
#include "utility/FileReader.h"
#include "utility/StringHelpers.h"
#include "utility/ImportSpecifiers.h"
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

void Program::import_source_file(std::string path_str, bool skip_duplicate, bool ignore_import_dirs)
{
    std::filesystem::path import_from_dir;
    if (!m_translation_units.empty())
        import_from_dir = std::filesystem::path(curr_tu().get_path()).parent_path();
    else
        import_from_dir = "";

    std::filesystem::path path(path_str);
    if (!path.is_absolute())
    {
        bool found_base = false;

        // try importing relative to "importer"
        try {
            path_str = std::filesystem::canonical(import_from_dir / path).generic_string();
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
    //std::string out_file_pdf = "out/" + replace_all(path_str, "/", ".") + ".pdf";
    //system("mkdir -p out/");
    //qrawlr::write_file("out/tree.gv", result.tree->to_digraph_str(m_verbose));
    //std::string dot_command = "dot -Tpdf -o \"" + out_file_pdf + "\" out/tree.gv";
    //system(dot_command.c_str());

    push_tu(path_str, result.tree);
    handle_tree_node(result.tree, "GlobalCode", nullptr);
    pop_tu();
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
        throw make_grammar_exception("[*handle_tree_node_one_of*]: Node with name '" + node->get_name() + "' does not match any of the given names: [ " + join(names, ", ") + "]", node);

    static const std::map<std::string, Handler> handlers = {
        { "GlobalCode",           &Program::handle_tree_node_global_code        },
        { "StatementImport",      &Program::handle_tree_node_stmt_import        },
        { "ImportSpecifiers",     &Program::handle_tree_node_import_specifiers  },
        { "LiteralString",        &Program::handle_tree_node_literal_string     },
        { "Comment",              &Program::handle_tree_node_comment            },
    };

    // Get the corresponding handler
    auto it = handlers.find(node->get_name());
    if (it == handlers.end())
        throw make_grammar_exception("[*handle_tree_node_one_of*]: Unhandled node name '" + node->get_name() + "'", node);
    Handler handler = it->second;

    // Call the handler
    (this->*handler)(node, pData);
}

void Program::handle_tree_node_global_code(qrawlr::ParseTreeNodeRef node, void* pUnused)
{
    (void)pUnused;

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

void Program::handle_tree_node_stmt_import(qrawlr::ParseTreeNodeRef node, void* pUnused)
{
    (void)pUnused;

    qrawlr::Flags<ImportSpecifier> flags;
    handle_tree_node(qrawlr::expect_child_node(node, "ImportSpecifiers"), "ImportSpecifiers", &flags);
    // TODO: handle flags

    std::string path_str;
    handle_tree_node(qrawlr::expect_child_node(node, "LiteralString"), "LiteralString", &path_str);

    // TODO: remove debug print
    printf("Importing '%s'... (requested by '%s')\n", path_str.c_str(), curr_tu().get_path().c_str());

    import_source_file(path_str, true);
}

void Program::handle_tree_node_import_specifiers(qrawlr::ParseTreeNodeRef node, void* pFlags)
{
    auto& flags = *(qrawlr::Flags<ImportSpecifier>*)pFlags;

    for (auto child : node->get_children())
    {
        auto leaf = qrawlr::expect_leaf(child);
        auto value = leaf->get_value();

        if (value == "linux")
            flags.set(ImportSpecifier::Platform_Linux);
        else if (value == "windows")
            flags.set(ImportSpecifier::Platform_Windows);
        else if (value == "macos")
            flags.set(ImportSpecifier::Platform_MacOS);
        else if (value == "defer")
            flags.set(ImportSpecifier::Defer);
        else
            throw make_grammar_exception("Unknown import specifier '" + value + "'", child);
    }
}

void Program::handle_tree_node_literal_string(qrawlr::ParseTreeNodeRef node, void* pString)
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
                throw make_grammar_exception("Unknown escape sequence", child);
            string_out.push_back(it->second);
        }
    }
}

void Program::handle_tree_node_comment(qrawlr::ParseTreeNodeRef node, void* pUnused)
{
    (void)pUnused;
    (void)node;

    // TODO: comment extraction
}

qrawlr::GrammarException Program::make_grammar_exception(const std::string& message, qrawlr::ParseTreeRef elem)
{
    return make_grammar_exception(message, elem, curr_tu().get_path());
}

qrawlr::GrammarException Program::make_grammar_exception(const std::string& message, qrawlr::ParseTreeRef elem, const std::string& path)
{
    return qrawlr::GrammarException(message, elem->get_pos_begin().to_string(path));
}

TranslationUnit& Program::push_tu(const std::string& path, qrawlr::ParseTreeRef tree)
{
    m_translation_units.emplace(path, tree);
    return curr_tu();
}

TranslationUnit& Program::curr_tu()
{
    return m_translation_units.top();
}

void Program::pop_tu()
{
    m_translation_units.pop();
}