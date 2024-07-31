#include "Program.h"

#include "errors/QinpError.h"
#include "utility/FileReader.h"
#include "utility/StringHelpers.h"
#include "utility/ImportSpecifiers.h"
#include "grammar/QinpGrammarCStr.h"
#include "symbols/Symbols.h"

ProgramRef Program::s_singleton = nullptr;

Program::Program(Architecture arch, bool verbose)
    : m_architecture(arch), m_verbose(verbose),
    m_root_sym(Symbol<SymbolSpace>::make("", qrawlr::Position()).as_type<_Symbol>()),
    m_grammar(
        qrawlr::Grammar::load_from_text(
            QINP_GRAMMAR_C_STR,
            QINP_GRAMMAR_ORIGIN_C_STR
        )
    )
{
    m_f_tree_id_to_name = [&](int tree_id){ return m_file_tree_ids.find(tree_id)->second; };
}

int Program::get_ptr_size() const
{
    switch (m_architecture)
    {
    case Architecture::Unknown: throw QinpError("[*Program::get_ptr_size*]: Unknown architecture!");
    case Architecture::x86_64: return 8;
    case Architecture::QIPU: return 4;
    default: throw QinpError("[*Program::get_ptr_size*]: Unhandled Architecture!");
    }
}

int Program::get_builtin_type_size(const std::string& type_name) const
{
    static const std::map<std::string, int> type_map = {
        { "void", 1 },
        { "i8",   1 },
        { "i16",  2 },
        { "i32",  4 },
        { "i64",  8 },
        { "u8",   1 },
        { "u16",  2 },
        { "u32",  4 },
        { "u64",  8 }
    };

    auto it = type_map.find(type_name);

    if (it == type_map.end())
        return -1;

    return it->second;
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
        import_from_dir = std::filesystem::path(curr_tu()->get_file_path()).parent_path();
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
    if ((std::size_t)result.pos_end.index < code_str.size())
        throw qrawlr::GrammarException("Could not parse remaining source", result.pos_end.to_string(std::function([&](int){ return path_str; })));

    // TODO: remove debug tree graphviz + render
    //std::string out_file_pdf = "out/" + replace_all(path_str, "/", ".") + ".pdf";
    //system("mkdir -p out/");
    //qrawlr::write_file("out/tree.gv", result.tree->to_digraph_str(m_verbose));
    //std::string dot_command = "dot -Tpdf -o \"" + out_file_pdf + "\" out/tree.gv";
    //system(dot_command.c_str());

    m_file_tree_ids.insert({ result.tree->get_pos_begin().tree_id, path_str });

    push_tu(path_str);
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
        throw make_node_exception("[*handle_tree_node_one_of*]: Node with name '" + node->get_name() + "' does not match any of the given names: [ " + join(names.begin(), names.end(), ", ") + "]", node);

    static const std::map<std::string, Handler> handlers = {
        { "GlobalCode",               &Program::handle_tree_node_code_block         },
        { "CodeBlock",                &Program::handle_tree_node_code_block         },
        { "StatementImport",          &Program::handle_tree_node_stmt_import        },
        { "StatementSpace",           &Program::handle_tree_node_stmt_space         },
        { "StatementFunctionDeclDef", &Program::handle_tree_node_stmt_func_decl_def },
        { "FunctionHeader",           &Program::handle_tree_node_func_header        },
        { "FunctionReturnType",       &Program::handle_tree_node_func_ret_type      },
        { "FunctionParameters",       &Program::handle_tree_node_func_params        },
        { "ImportSpecifiers",         &Program::handle_tree_node_import_specifiers  },
        { "LiteralString",            &Program::handle_tree_node_literal_string     },
        { "Comment",                  &Program::handle_tree_node_comment            },
        { "Datatype",                 &Program::handle_tree_node_datatype           },
        { "Identifier",               &Program::handle_tree_node_identifier         },
        { "SymbolReference",          &Program::handle_tree_node_symbol_reference   },
        { "StatementDefer",           &Program::handle_tree_node_stmt_defer         },
    };

    // Get the corresponding handler
    auto it = handlers.find(node->get_name());
    if (it == handlers.end())
        throw make_node_exception("[*handle_tree_node_one_of*]: Unhandled node name '" + node->get_name() + "'", node);
    Handler handler = it->second;

    // Call the handler
    (this->*handler)(node, pData);
}

void Program::handle_tree_node_code_block(qrawlr::ParseTreeNodeRef node, void* pUnused)
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

    if (m_verbose)
        printf("Importing '%s'... (requested by '%s')\n", path_str.c_str(), curr_tu()->get_file_path().c_str());

    import_source_file(path_str, true);
}

void Program::handle_tree_node_stmt_space(qrawlr::ParseTreeNodeRef node, void* pUnused)
{
    (void)pUnused;

    std::string space_name = qrawlr::expect_child_leaf(node, "SpaceHeader.SpaceName.Identifier.0")->get_value();

    auto space = curr_tu()->curr_symbol().add_child(
        Symbol<SymbolSpace>::make(space_name, node->get_pos_begin()),
        DuplicateHandling::Keep
    );

    curr_tu()->enter_symbol(space);
    handle_tree_node(qrawlr::expect_child_node(node, "CodeBlock"), "CodeBlock", nullptr);
    curr_tu()->leave_symbol();
}

void Program::handle_tree_node_stmt_func_decl_def(qrawlr::ParseTreeNodeRef node, void* pUnused)
{
    (void)pUnused;

    Symbol sym;
    handle_tree_node(qrawlr::expect_child_node(node, "FunctionHeader"), "FunctionHeader", &sym);

    if (qrawlr::has_child_node(node, "FunctionDeclaration"))
        ; // Nothing to do
    else if (qrawlr::has_child_node(node, "FunctionDefinition"))
    {
        auto func = sym.as_type<SymbolFunction>();
        if (func->is_defined())
            throw make_node_exception("Function '' has already been defined here: ", node);

        // TODO: Parse function body
        throw make_node_exception("[*Program::handle_tree_node_stmt_func_decl_def*]: Function definition not implemented yet!", node);
    }
    else
        throw make_node_exception("[*Program::handle_tree_node_stmt_func_decl_def*]: Missing 'FunctionDeclaration' or 'FunctionDefinition' node!", node);

    printf("Reached function with name '%s'\n", sym->get_symbol_path().to_string().c_str());
}

void Program::handle_tree_node_func_header(qrawlr::ParseTreeNodeRef node, void* pSym)
{
    auto& sym = *(Symbol<>*)pSym;

    Datatype return_type;
    SymbolPath func_name_path;
    Parameter_Decl parameters;
    bool is_nodiscard = false;
    handle_tree_node(qrawlr::expect_child_node(node, "FunctionReturnType"), "FunctionReturnType", &return_type);
    handle_tree_node(qrawlr::expect_child_node(node, "SymbolReference"),    "SymbolReference",    &func_name_path);
    handle_tree_node(qrawlr::expect_child_node(node, "FunctionParameters"), "FunctionParameters", &parameters);
    // TODO: Handle Function Specifiers

    // Get SymbolFunctionName
    sym = curr_tu()->get_symbol_from_path(func_name_path.get_parent_path()).add_child(
        Symbol<SymbolFunctionName>::make(
            func_name_path.get_name(),
            node->get_pos_begin()
        ).as_type<_Symbol>(),
        DuplicateHandling::Keep
    );

    if (!sym.is_of_type<SymbolFunctionName>())
        throw make_node_exception("Cannot declare/define function with name '" +
            func_name_path.get_name() +
            "', non-function symbol with same name already defined here: " +
            sym->get_position().to_string(m_f_tree_id_to_name), node);

    auto spec_path = SymbolPath(func_name_path).enter(parameters.get_symbol_path_str());

    // Create and enter blueprint symbol if necessary
    if (parameters.is_blueprint)
    {
        sym = sym.add_child(
            Symbol<SymbolSpace>::make(
                SYMBOL_NAME_BLUEPRINT,
                node->get_pos_begin()
            ),
            DuplicateHandling::Keep
        );
    }

    // Create specialization if not already declared
    sym = sym.add_child(
        Symbol<SymbolFunction>::make(
            return_type,
            spec_path.get_name(),
            parameters,
            is_nodiscard,
            node->get_pos_begin()
        ),
        DuplicateHandling::Keep
    );
}

void Program::handle_tree_node_func_ret_type(qrawlr::ParseTreeNodeRef node, void* pReturn_type)
{
    if (!qrawlr::has_child_node(node, "Datatype")) // No return type specified
    {
        *(Datatype<>*)pReturn_type = DT_NAMED("void", false).as_type<_Datatype>();
        return;
    }

    handle_tree_node(qrawlr::expect_child_node(node, "Datatype"), "Datatype", pReturn_type);
}

void Program::handle_tree_node_func_params(qrawlr::ParseTreeNodeRef node, void* pParameters)
{
    auto& parameters = *(Parameter_Decl*)pParameters;

    for (auto tree_ref : node->get_children())
    {
        if (!parameters.named_parameters.empty() && parameters.named_parameters.back().datatype->get_type() == Datatype<>::Type::Variadic)
            throw make_node_exception("Variadic parameter must be at the end of the parameter list", tree_ref);

        auto param_node = qrawlr::expect_node(tree_ref);
        if (param_node->get_name() == "Normal")
        {
            Parameter param;
            handle_tree_node(qrawlr::expect_child_node(param_node, "Datatype"), "Datatype", &param.datatype);
            handle_tree_node(qrawlr::expect_child_node(param_node, "Identifier"), "Identifier", &param.name);
            parameters.named_parameters.push_back(param);

            // Check if param is macro
            auto dt = param.datatype;
            while (dt)
            {
                if (dt->get_type() == Datatype<>::Type::Macro)
                {
                    parameters.is_blueprint = true;
                    break;
                }
                dt = dt.as_type<_Datatype_Parent>();
            }
        }
        else if (param_node->get_name() == "Variadic")
        {
            parameters.is_blueprint = true;

            Parameter param;
            param.datatype = DT_VARIADIC();
            param.name = "__VA_ARGS__";
            parameters.named_parameters.push_back(param);
        }
        else
        {
            throw make_node_exception("[*Program::handle_tree_node_func_params*]: Unhandled parameter type!", node);
        }
    }
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
            throw make_node_exception("Unknown import specifier '" + value + "'", child);
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
                throw make_node_exception("Unknown escape sequence", child);
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


void Program::handle_tree_node_datatype(qrawlr::ParseTreeNodeRef node, void* pDatatype)
{
    auto& dt = *(Datatype<>*)pDatatype;

    if (qrawlr::has_child_node(node, "DatatypeBlueprint"))
    {
        // TODO: More elaborate blueprint handling
        std::string identifier;
        handle_tree_node(qrawlr::expect_child_node(node, "DatatypeBlueprint.Identifier"), "Identifier", &identifier);
        dt = DT_MACRO(identifier, false).as_type<_Datatype>();
        return;
    }
    else if (qrawlr::has_child_node(node, "DatatypeFunction"))
    {
        // TODO: Handle constness
        Datatype return_type;
        Parameter_Types parameters;
        handle_tree_node(qrawlr::expect_child_node(node, "DatatypeFunction.FunctionReturnType"), "FunctionReturnType", &return_type);
        handle_tree_node(qrawlr::expect_child_node(node, "DatatypeFunction.DatatypeFunctionParameters"), "DatatypeFunctionParameters", &parameters);
        dt = DT_FUNCTION(return_type, parameters, false).as_type<_Datatype>();
        return;
    }
    
    node = qrawlr::expect_child_node(node, "DatatypeNamed");
    auto& children = node->get_children();
    for (auto it = children.begin(); it != children.end(); ++it)
    {
        auto elem = qrawlr::expect_node(*it);
        
        bool is_const = false;
        if (qrawlr::is_node(*++it, "Const"))
            is_const = true;
        else
            --it;

        if (elem->get_name() == "SymbolReference")
        {
            SymbolPath name_path;
            handle_tree_node(elem, "SymbolReference", &name_path);
            // TODO: Could use SymbolPath instead of string?
            dt = DT_NAMED(name_path.to_string(), is_const).as_type<_Datatype>();
        }
        else if (elem->get_name() == "Pointer")
        {
            dt = DT_POINTER(dt, is_const).as_type<_Datatype>();
        }
        else if (elem->get_name() == "Expression")
        {
            // TODO: Implementation
            throw make_node_exception("[]: Handling of datatype array not implemented yet!", node);
            int num_elements = 1;
            dt = DT_ARRAY(num_elements, dt, is_const).as_type<_Datatype>();
        }
        else if (elem->get_name() == "Reference")
        {
            // TODO: Implementation
            throw make_node_exception("[]: Handling of datatype reference not implemented yet!", node);
            dt = DT_REFERENCE(dt, is_const).as_type<_Datatype>();
        }
        else
        {
            throw make_node_exception("[*Program::handle_tree_node_datatype*]: Unhandled element type!", node);
        }
    }
}

void Program::handle_tree_node_identifier(qrawlr::ParseTreeNodeRef node, void* pString)
{
    *(std::string*)pString = qrawlr::expect_child_leaf(node, "0")->get_value();
}

void Program::handle_tree_node_symbol_reference(qrawlr::ParseTreeNodeRef node, void* pPath)
{
    std::vector<std::string> elements;
    bool is_from_root = qrawlr::has_child_leaf(node, "0");

    auto& children = node->get_children();
    for (std::size_t i = is_from_root ? 1 : 0; i < children.size(); ++i)
    {
        std::string identifier;
        handle_tree_node(children[i], "Identifier", &identifier);
        elements.push_back(identifier);
    }

    *(SymbolPath*)pPath = SymbolPath(elements, is_from_root);
}

void Program::handle_tree_node_stmt_defer(qrawlr::ParseTreeNodeRef node, void* pUnused)
{
    (void)pUnused;
    (void)node;

    m_deferred_translation_units.push(curr_tu());
    pop_tu();
    push_tu(m_deferred_translation_units.front());
    m_deferred_translation_units.pop();
}

QinpError Program::make_node_exception(const std::string& message, qrawlr::ParseTreeRef elem)
{
    return QinpError(qrawlr::GrammarException(message, elem->get_pos_begin().to_string(m_f_tree_id_to_name)).what());
}

TranslationUnitRef Program::push_tu(const std::string& path)
{
    return push_tu(std::make_shared<TranslationUnit>(path, m_root_sym));
}

TranslationUnitRef Program::push_tu(TranslationUnitRef tu)
{
    m_translation_units.push(tu);
    return curr_tu();
}

TranslationUnitRef Program::curr_tu()
{
    return m_translation_units.top();
}

void Program::pop_tu()
{
    m_translation_units.pop();
}

ProgramRef Program::get()
{
    if (!s_singleton)
        throw QinpError("[*Program::get*]: Program singleton has not been initialized yet!");
    return s_singleton;
}

void Program::init(Architecture arch, bool verbose)
{
    s_singleton = std::shared_ptr<Program>(new Program(arch, verbose));
}