#include "Program.h"

#include "errors/QinpError.h"
#include "utility/FileReader.h"
#include "utility/StringHelpers.h"
#include "utility/ImportSpecifiers.h"
#include "grammar/QinpGrammarCStr.h"
#include "symbols/Symbols.h"
#include "expressions/Expressions.h"

ProgramRef Program::s_singleton = nullptr;

Program::Program(Architecture arch, Platform platform, CmdFlags flags)
    : m_architecture(arch), m_platform(platform), m_flags(flags),
    m_root_sym(Symbol<SymbolSpace>::make("", qrawlr::Position())),
    m_grammar(
        qrawlr::Grammar::load_from_text(
            QINP_GRAMMAR_C_STR,
            QINP_GRAMMAR_ORIGIN_C_STR
        )
    )
{
    m_f_tree_id_to_name = [&](int tree_id){ return m_file_tree_ids.find(tree_id)->second; };
}

Program::~Program()
{
    if (m_flags.is_set(CmdFlag::Render_Symbol_Tree))
    {
        system("mkdir -p out/");

        if (m_flags.is_set(CmdFlag::Verbose))
            printf("Generating symbol tree graphviz...\n");
        qrawlr::write_file("out/tree.gv", m_root_sym->to_digraph_str(m_flags.is_set(CmdFlag::Verbose)));

        if (m_flags.is_set(CmdFlag::Verbose))
            printf("Rendering symbol tree graphviz...\n");
        std::string out_file_pdf = "out/symbol_tree.pdf";
        std::string dot_command = "dot -Tpdf -o \"" + out_file_pdf + "\" out/tree.gv";
        system(dot_command.c_str());
    }
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

std::function<std::string(int)> Program::get_fn_tree_id_to_name() const
{
    return m_f_tree_id_to_name;
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

    if (m_flags.is_set(CmdFlag::Render_Qrawlr_Tree))
    {
        system("mkdir -p out/");

        if (m_flags.is_set(CmdFlag::Verbose))
            printf("Generating graphviz for qrawlr tree '%s'...\n", path_str.c_str());
        qrawlr::write_file("out/tree.gv", result.tree->to_digraph_str(m_flags.is_set(CmdFlag::Verbose)));

        if (m_flags.is_set(CmdFlag::Verbose))
            printf("Rendering graphviz for qrawlr tree '%s'...\n", path_str.c_str());
        std::string out_file_pdf = "out/" + replace_all(path_str, "/", ".") + ".pdf";
        std::string dot_command = "dot -Tpdf -o \"" + out_file_pdf + "\" out/tree.gv";
        system(dot_command.c_str());
    }

    if (!result.tree)
        return;

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
    auto node = qrawlr::expect_node(tree, m_f_tree_id_to_name);

    // Check if nodes' name matches any of the given names
    if (names.find(node->get_name()) == names.end())
        throw QinpError::from_node("[*handle_tree_node_one_of*]: Node with name '" + node->get_name() + "' does not match any of the given names: [ " + join(names.begin(), names.end(), ", ") + "]", node);

    static const std::map<std::string, Handler> handlers = {
        { "GlobalCode",               &Program::handle_tree_node_code_block         },
        { "CodeBlock",                &Program::handle_tree_node_code_block         },
        { "StatementImport",          &Program::handle_tree_node_stmt_import        },
        { "StatementSpace",           &Program::handle_tree_node_stmt_space         },
        { "StatementFunctionDeclDef", &Program::handle_tree_node_stmt_func_decl_def },
        { "FunctionHeader",           &Program::handle_tree_node_func_header        },
        { "OptionalDatatype",         &Program::handle_tree_node_optional_datatype  },
        { "FunctionParameters",       &Program::handle_tree_node_func_params        },
        { "ImportSpecifiers",         &Program::handle_tree_node_import_specifiers  },
        { "LiteralString",            &Program::handle_tree_node_literal_string     },
        { "Comment",                  &Program::handle_tree_node_comment            },
        { "Datatype",                 &Program::handle_tree_node_datatype           },
        { "Identifier",               &Program::handle_tree_node_identifier         },
        { "SymbolReference",          &Program::handle_tree_node_symbol_reference   },
        { "StatementDefer",           &Program::handle_tree_node_stmt_defer         },
        { "StatementEnumDeclDef",     &Program::handle_tree_node_stmt_enum_decl_def },
        { "EnumHeader",               &Program::handle_tree_node_enum_header        },
        { "EnumDefinition",           &Program::handle_tree_node_enum_def           },
        { "EnumMemberDefinition",     &Program::handle_tree_node_enum_member_def    },
        { "StatementVariableDeclDef", &Program::handle_tree_node_stmt_var_decl_def  },
        { "VariableDeclarators",      &Program::handle_tree_node_var_declarators    },
        { "VariableInitializer",      &Program::handle_tree_node_var_initializer    },
        { "StatementReturn",          &Program::handle_tree_node_stmt_return        },
        { "StatementIfElifElse",      &Program::handle_tree_node_stmt_if_elif_else  },
        { "Expression",               &Program::handle_tree_node_expression         },
        { "ExprPrec1",                &Program::handle_tree_node_expr_prec_1        },
        { "ExprPrec2",                &Program::handle_tree_node_expr_prec_2        },
        { "ExprPrec3",                &Program::handle_tree_node_expr_prec_3        },
        { "ExprPrec4",                &Program::handle_tree_node_expr_prec_4        },
        { "ExprPrec5",                &Program::handle_tree_node_expr_prec_5        },
        { "ExprPrec6",                &Program::handle_tree_node_expr_prec_6        },
        { "ExprPrec7",                &Program::handle_tree_node_expr_prec_7        },
        { "ExprPrec8",                &Program::handle_tree_node_expr_prec_8        },
        { "ExprPrec9",                &Program::handle_tree_node_expr_prec_9        },
        { "ExprPrec10",               &Program::handle_tree_node_expr_prec_10       },
        { "ExprPrec11",               &Program::handle_tree_node_expr_prec_11       },
        { "ExprPrec12",               &Program::handle_tree_node_expr_prec_12       },
        { "ExprPrec13",               &Program::handle_tree_node_expr_prec_13       },
        { "ExprPrec14",               &Program::handle_tree_node_expr_prec_14       },
        { "ExprPrec15",               &Program::handle_tree_node_expr_prec_15       },
    };

    // Get the corresponding handler
    auto it = handlers.find(node->get_name());
    if (it == handlers.end())
        throw QinpError::from_node("[*handle_tree_node_one_of*]: Unhandled node name '" + node->get_name() + "'", node);
    Handler handler = it->second;

    // Call the handler
    try {
        (this->*handler)(node, pData);
    }
    catch (std::runtime_error& err)
    {
        if (m_flags.is_set(CmdFlag::Verbose))
            throw QinpError::from_node("[" + node->get_name() + "]\n" + err.what(), node);
        else
            throw err;
    }
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
    handle_tree_node(qrawlr::expect_child_node(node, "ImportSpecifiers", m_f_tree_id_to_name), "ImportSpecifiers", &flags);
    // TODO: handle flags

    std::string path_str;
    handle_tree_node(qrawlr::expect_child_node(node, "LiteralString", m_f_tree_id_to_name), "LiteralString", &path_str);
    replace_all(path_str, "{architecture}", architecture_to_string(m_architecture));
    replace_all(path_str, "{platform}", platform_to_string(m_platform));

    if (m_flags.is_set(CmdFlag::Verbose))
        printf("Importing '%s'... (requested by '%s')\n", path_str.c_str(), curr_tu()->get_file_path().c_str());

    import_source_file(path_str, true);
}

void Program::handle_tree_node_stmt_space(qrawlr::ParseTreeNodeRef node, void* pUnused)
{
    (void)pUnused;

    std::string space_name = qrawlr::expect_child_leaf(node, "SpaceHeader.SpaceName.Identifier.0", m_f_tree_id_to_name)->get_value();

    auto space = curr_tu()->curr_symbol().add_child(
        Symbol<SymbolSpace>::make(space_name, node->get_pos_begin()),
        m_f_tree_id_to_name,
        DuplicateHandling::Keep
    ).as_type<SymbolSpace>();

    if (!space->is_defined())
        space->set_definition(node->get_pos_begin());

    curr_tu()->enter_symbol(space);
    handle_tree_node(qrawlr::expect_child_node(node, "CodeBlock", m_f_tree_id_to_name), "CodeBlock", nullptr);
    curr_tu()->leave_symbol();
}

void Program::handle_tree_node_stmt_func_decl_def(qrawlr::ParseTreeNodeRef node, void* pUnused)
{
    (void)pUnused;

    Symbol sym;
    handle_tree_node(qrawlr::expect_child_node(node, "FunctionHeader", m_f_tree_id_to_name), "FunctionHeader", &sym);

    if (qrawlr::has_child_node(node, "FunctionDeclaration"))
    {
        ; // Nothing to do
    }
    else if (qrawlr::has_child_node(node, "FunctionDefinition"))
    {
        auto func = sym.as_type<SymbolFunction>();
        if (func->is_defined())
            throw QinpError::from_node("Function '" + func->get_symbol_path().to_string() + "' has already been defined.", node);

        if (func.is_of_type<SymbolFunctionSpecialization>())
        {
            // TODO: Parse function body
            auto& block = func.as_type<SymbolFunctionSpecialization>()->set_definition(node->get_pos_begin());

            curr_tu()->enter_symbol(func);
            handle_tree_node(qrawlr::expect_child(node, "FunctionDefinition.CodeBlock", m_f_tree_id_to_name), "CodeBlock", &block);
            curr_tu()->leave_symbol();
        }
        else if (func.is_of_type<SymbolFunctionBlueprint>())
        {
            func.as_type<SymbolFunctionBlueprint>()->set_definition(qrawlr::expect_child(node, "FunctionDefinition.CodeBlock", m_f_tree_id_to_name));
        }
        else
        {
            throw QinpError::from_node("[*Program::handle_tree_node_stmt_func_decl_def*]: Unknown SymbolFunction* type!", node);
        }
    }
    else
    {
        throw QinpError::from_node("[*Program::handle_tree_node_stmt_func_decl_def*]: Missing 'FunctionDeclaration' or 'FunctionDefinition' node!", node);
    }

    // TODO: Remove debug print
    printf("Reached function with name '%s'\n", sym->get_symbol_path().to_string().c_str());
}

void Program::handle_tree_node_func_header(qrawlr::ParseTreeNodeRef node, void* pFuncSymOut)
{
    auto& sym = *(Symbol<>*)pFuncSymOut;

    Datatype return_type;
    SymbolPath func_name_path;
    Parameter_Decl parameters;
    bool is_nodiscard = false;
    handle_tree_node(qrawlr::expect_child_node(node, "OptionalDatatype", m_f_tree_id_to_name), "OptionalDatatype", &return_type);
    handle_tree_node(qrawlr::expect_child_node(node, "SymbolReference", m_f_tree_id_to_name),    "SymbolReference",    &func_name_path);
    handle_tree_node(qrawlr::expect_child_node(node, "FunctionParameters", m_f_tree_id_to_name), "FunctionParameters", &parameters);
    // TODO: Handle Function Specifiers

    // Get SymbolFunctionName
    sym = curr_tu()->get_symbol_from_path(func_name_path.get_parent_path()).add_child(
        Symbol<SymbolFunctionName>::make(
            func_name_path.get_name(),
            node->get_pos_begin()
        ),
        m_f_tree_id_to_name,
        DuplicateHandling::Keep
    );

    if (!sym.is_of_type<SymbolFunctionName>())
        throw QinpError::from_node("Cannot declare/define function with name '" +
            func_name_path.get_name() +
            "', non-function symbol with same name already defined here: " +
            sym->get_position().to_string(m_f_tree_id_to_name), node);

    // Create and enter blueprint symbol if necessary
    if (parameters.is_blueprint)
    {
        // sym = sym.add_child(
        //     Symbol<SymbolSpace>::make(
        //         SYMBOL_NAME_BLUEPRINTS,
        //         node->get_pos_begin()
        //     ),
        //     m_f_tree_id_to_name,
        //     DuplicateHandling::Keep
        // );

        sym = sym.add_child(
            Symbol<SymbolFunctionBlueprint>::make(
                return_type,
                parameters,
                is_nodiscard,
                node->get_pos_begin()
            ),
            m_f_tree_id_to_name,
            DuplicateHandling::Keep
        );
    }
    else
    {
        // Create specialization if not already declared
        sym = sym.add_child(
            Symbol<SymbolFunctionSpecialization>::make(
                return_type,
                parameters,
                is_nodiscard,
                node->get_pos_begin()
            ),
            m_f_tree_id_to_name,
            DuplicateHandling::Keep
        );
    }
}

void Program::handle_tree_node_optional_datatype(qrawlr::ParseTreeNodeRef node, void* pDatatypeOut)
{
    if (!qrawlr::has_child_node(node, "Datatype")) // No return type specified
    {
        *(Datatype<>*)pDatatypeOut = DT_NAMED("void", false);
        return;
    }

    handle_tree_node(qrawlr::expect_child_node(node, "Datatype", m_f_tree_id_to_name), "Datatype", pDatatypeOut);
}

void Program::handle_tree_node_func_params(qrawlr::ParseTreeNodeRef node, void* pParamsOut)
{
    auto& parameters = *(Parameter_Decl*)pParamsOut;

    for (auto tree_ref : node->get_children())
    {
        if (!parameters.named_parameters.empty() && parameters.named_parameters.back().datatype->is_variadic())
            throw QinpError::from_node("Variadic parameter must be at the end of the parameter list", tree_ref);

        auto param_node = qrawlr::expect_node(tree_ref, m_f_tree_id_to_name);
        if (param_node->get_name() == "Normal")
        {
            Parameter param;
            handle_tree_node(qrawlr::expect_child_node(param_node, "Datatype", m_f_tree_id_to_name), "Datatype", &param.datatype);
            handle_tree_node(qrawlr::expect_child_node(param_node, "Identifier", m_f_tree_id_to_name), "Identifier", &param.name);
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
                // TODO: Proper implementation
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
            throw QinpError::from_node("[*Program::handle_tree_node_func_params*]: Unhandled parameter type!", node);
        }
    }
}

void Program::handle_tree_node_import_specifiers(qrawlr::ParseTreeNodeRef node, void* pFlagsOut)
{
    auto& flags = *(qrawlr::Flags<ImportSpecifier>*)pFlagsOut;

    for (auto child : node->get_children())
    {
        auto leaf = qrawlr::expect_leaf(child, m_f_tree_id_to_name);
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
            throw QinpError::from_node("Unknown import specifier '" + value + "'", child);
    }
}

void Program::handle_tree_node_literal_string(qrawlr::ParseTreeNodeRef node, void* pStringOut)
{
    std::string& string_out = *(std::string*)pStringOut;
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

        qrawlr::expect_node(child, "EscapeSequence", m_f_tree_id_to_name);
        std::string seq = qrawlr::expect_child_leaf(child, "0", m_f_tree_id_to_name)->get_value();
        if (seq[0] == 'x')
        {
            string_out.push_back((char)std::stoi(seq.substr(1), nullptr, 16));
        }
        else
        {
            auto it = sequences.find(seq[0]);
            if (it == sequences.end())
                throw QinpError::from_node("Unknown escape sequence", child);
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


void Program::handle_tree_node_datatype(qrawlr::ParseTreeNodeRef node, void* pDatatypeOut)
{
    auto& dt = *(Datatype<>*)pDatatypeOut;

    if (qrawlr::has_child_node(node, "DatatypeBlueprint"))
    {
        // TODO: More elaborate blueprint handling
        std::string identifier;
        handle_tree_node(qrawlr::expect_child_node(node, "DatatypeBlueprint.Identifier", m_f_tree_id_to_name), "Identifier", &identifier);
        dt = DT_MACRO(identifier, false);
        return;
    }
    else if (qrawlr::has_child_node(node, "DatatypeFunction"))
    {
        // TODO: Handle constness
        Datatype return_type;
        Parameter_Types parameters;
        handle_tree_node(qrawlr::expect_child_node(node, "DatatypeFunction.OptionalDatatype", m_f_tree_id_to_name), "OptionalDatatype", &return_type);
        handle_tree_node(qrawlr::expect_child_node(node, "DatatypeFunction.DatatypeFunctionParameters", m_f_tree_id_to_name), "DatatypeFunctionParameters", &parameters);
        dt = DT_FUNCTION(return_type, parameters, false);
        return;
    }
    
    node = qrawlr::expect_child_node(node, "DatatypeNamed", m_f_tree_id_to_name);
    auto& children = node->get_children();
    for (auto it = children.begin(); it != children.end(); ++it)
    {
        auto elem = qrawlr::expect_node(*it, m_f_tree_id_to_name);
        
        bool is_const = false;
        if (++it != children.end() && qrawlr::is_node(*it, "Const"))
            is_const = true;
        else
            --it;

        if (elem->get_name() == "SymbolReference")
        {
            SymbolPath name_path;
            handle_tree_node(elem, "SymbolReference", &name_path);
            // TODO: Could use SymbolPath instead of string?
            dt = DT_NAMED(name_path.to_string(), is_const);
        }
        else if (elem->get_name() == "Pointer")
        {
            dt = DT_POINTER(dt, is_const);
        }
        else if (elem->get_name() == "Expression")
        {
            // TODO: Implementation
            throw QinpError::from_node("[*Program::handle_tree_node_datatype*]: Handling of datatype array not implemented yet!", node);
            int num_elements = 1;
            dt = DT_ARRAY(num_elements, dt, is_const);
        }
        else if (elem->get_name() == "Reference")
        {
            // TODO: Implementation
            throw QinpError::from_node("[*Program::handle_tree_node_datatype*]: Handling of datatype reference not implemented yet!", node);
            dt = DT_REFERENCE(dt, is_const);
        }
        else
        {
            throw QinpError::from_node("[*Program::handle_tree_node_datatype*]: Unhandled element type!", node);
        }
    }
}

void Program::handle_tree_node_identifier(qrawlr::ParseTreeNodeRef node, void* pStringOut)
{
    *(std::string*)pStringOut = qrawlr::expect_child_leaf(node, "0", m_f_tree_id_to_name)->get_value();
}

void Program::handle_tree_node_symbol_reference(qrawlr::ParseTreeNodeRef node, void* pPathOut)
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

    *(SymbolPath*)pPathOut = SymbolPath(elements, is_from_root);
}

void Program::handle_tree_node_stmt_defer(qrawlr::ParseTreeNodeRef node, void* pUnused)
{
    (void)pUnused;
    (void)node;

    // TODO: Proper deferring
    m_deferred_translation_units.push(pop_tu());
    push_tu(m_deferred_translation_units.front());
    m_deferred_translation_units.pop();
}

void Program::handle_tree_node_stmt_enum_decl_def(qrawlr::ParseTreeNodeRef node, void* pUnused)
{
    (void)pUnused;

    Symbol symEnum;
    handle_tree_node(qrawlr::expect_child_node(node, "EnumHeader", m_f_tree_id_to_name), "EnumHeader", &symEnum);

    if (qrawlr::has_child_node(node, "EnumDeclaration"))
    {
        ; // Nothing to do
    }
    else if (qrawlr::has_child_node(node, "EnumDefinition"))
    {
        handle_tree_node(qrawlr::expect_child_node(node, "EnumDefinition", m_f_tree_id_to_name), "EnumDefinition", &symEnum);
    }
    else
    {
        throw QinpError::from_node("[*Program::handle_tree_node_stmt_enum_decl_def*]: Missing 'EnumDeclaration' or 'EnumDefinition' node!", node);
    }
}

void Program::handle_tree_node_enum_header(qrawlr::ParseTreeNodeRef node, void* pEnumSymOut)
{
    auto& enumSym = *(Symbol<>*)pEnumSymOut;

    SymbolPath enum_path;
    handle_tree_node(qrawlr::expect_child_node(node, "SymbolReference", m_f_tree_id_to_name), "SymbolReference", &enum_path);
    
    enumSym = curr_tu()->get_symbol_from_path(enum_path.get_parent_path()).add_child(
        Symbol<SymbolEnum>::make(
            enum_path.get_name(),
            node->get_pos_begin()
        ),
        m_f_tree_id_to_name,
        DuplicateHandling::Keep
    );
}

void Program::handle_tree_node_enum_def(qrawlr::ParseTreeNodeRef node, void* pEnumSym)
{
    for (auto& child : qrawlr::expect_child_node(node, "EnumMemberList", m_f_tree_id_to_name)->get_children())
        handle_tree_node(child, "EnumMemberDefinition", pEnumSym);
}

void Program::handle_tree_node_enum_member_def(qrawlr::ParseTreeNodeRef node, void* pEnumSym)
{
    auto& enumSym = *(Symbol<SymbolEnum>*)pEnumSym;

    std::string name;
    handle_tree_node(qrawlr::expect_child_node(node, "Identifier", m_f_tree_id_to_name), "Identifier", &name);

    std::size_t value = enumSym->next_value();
    if (qrawlr::has_child_node(node, "Expression"))
    {
        // TODO: Implementation
        throw QinpError::from_node("[*Program::handle_tree_node_enum_member_def*]: Member with initializer not implemented yet!", node);
    }

    enumSym.add_child(
        Symbol<SymbolEnumMember>::make(
            name,
            value,
            node->get_pos_begin()
        ),
        m_f_tree_id_to_name
    );
}

void Program::handle_tree_node_stmt_var_decl_def(qrawlr::ParseTreeNodeRef node, void* pUnused)
{
    (void)pUnused;

    VariableDeclarators declarators;
    Datatype<> datatype;
    std::string name;
    Expression<> initializer;

    handle_tree_node(qrawlr::expect_child_node(node, "VariableDeclarators", m_f_tree_id_to_name), "VariableDeclarators", &declarators);
    handle_tree_node(qrawlr::expect_child_node(node, "OptionalDatatype", m_f_tree_id_to_name), "OptionalDatatype", &datatype);
    handle_tree_node(qrawlr::expect_child_node(node, "VariableName.Identifier", m_f_tree_id_to_name), "Identifier", &name);
    handle_tree_node(qrawlr::expect_child_node(node, "VariableInitializer", m_f_tree_id_to_name), "VariableInitializer", &initializer);

    curr_tu()->curr_symbol().add_child(
        Symbol<SymbolVariable>::make(
            name,
            declarators,
            datatype,
            node->get_pos_begin()
        ),
        m_f_tree_id_to_name,
        DuplicateHandling::Throw
    );

    // TODO: Append initializer to current space body
}

void Program::handle_tree_node_var_declarators(qrawlr::ParseTreeNodeRef node, void* pDeclaratorsOut)
{
    auto& declarators = *(VariableDeclarators*)pDeclaratorsOut;

    for (auto& child : node->get_children())
    {
        auto declarator = qrawlr::expect_leaf(child, m_f_tree_id_to_name);
        auto& value = declarator->get_value();
        if (value == "static")
            declarators.is_static = true;
        else if (value == "var")
            ; // Nothing to do
        else if (value == "const")
            declarators.is_const = true;
        else
            throw QinpError::from_node("[*Program::handle_tree_node_var_declarators*]: Unexpected declarator '" + value + "'!", child);
    }
}

void Program::handle_tree_node_var_initializer(qrawlr::ParseTreeNodeRef node, void* pInitializerOut)
{
    if (qrawlr::has_child_node(node, "Expression"))
        handle_tree_node(qrawlr::expect_child_node(node, "Expression", m_f_tree_id_to_name), "Expression", pInitializerOut);
    else
        *(Expression<>*)pInitializerOut = nullptr;
}

void Program::handle_tree_node_stmt_return(qrawlr::ParseTreeNodeRef node, void* pUnused)
{
    (void)pUnused;

    // TODO: Check if current scope allows return statement

    Expression<> expr;
    handle_tree_node(qrawlr::expect_child_node(node, "Expression", m_f_tree_id_to_name), "Expression", &expr);

    curr_tu()->curr_symbol().as_type<SymbolSpace>()->append_expr(
        Expression<ExpressionReturn>::make(
            expr,
            node->get_pos_begin()
        )
    );
}

void Program::handle_tree_node_stmt_if_elif_else(qrawlr::ParseTreeNodeRef node, void* pUnused)
{
    (void)node;
    (void)pUnused;
    throw QinpError::from_pos("[*Program::handle_tree_node_stmt_if_elif_else*]: if/elif/else not implemented yet!", node->get_pos_begin());

    auto if_elif_helper = [&](const std::string& typeName, qrawlr::ParseTreeNodeRef stmtNode)
    {
        Expression<> condition;
        handle_tree_node(qrawlr::expect_child_node(stmtNode, "Expression", m_f_tree_id_to_name), "Expression", &condition);
    
        auto space = curr_tu()->curr_symbol().add_child(
            Symbol<SymbolSpace>::make(SymbolSpace::gen_unique_space_name(typeName), node->get_pos_begin()),
            m_f_tree_id_to_name,
            DuplicateHandling::Keep
        ).as_type<SymbolSpace>();

        auto expr = Expression<ExpressionIf>::make(
            condition,
            space->get_symbol_path()
        );

        curr_tu()->enter_symbol(space);
        handle_tree_node(qrawlr::expect_child_node(node, "CodeBlock", m_f_tree_id_to_name), "CodeBlock", nullptr);
        curr_tu()->leave_symbol();

        return expr;
    };

    auto expr = if_elif_helper("if", qrawlr::expect_child_node(node, "StatementIf", m_f_tree_id_to_name));

    // curr_tu()->curr_symbol().as_type<SymbolSpace>()->append_expr(
        
    // )
}

void Program::handle_tree_node_expression(qrawlr::ParseTreeNodeRef node, void* pExpressionOut)
{
    Expression<> expr_to_add = nullptr;
    if (!pExpressionOut)
        pExpressionOut = &expr_to_add;
    
    handle_appr_expr_prec(qrawlr::expect_child_node(node, "0", m_f_tree_id_to_name), pExpressionOut);

    // When pExpressionOut was null on call, expr_to_add is now non-null
    if (expr_to_add)
        curr_tu()->curr_symbol().as_type<SymbolSpace>()->append_expr(expr_to_add);
}

void Program::handle_appr_expr_prec(qrawlr::ParseTreeNodeRef node, void* pExpressionOut)
{
    static const std::set<std::string> valid_names = {
        "ExprPrec1",  "ExprPrec2",  "ExprPrec3",
        "ExprPrec4",  "ExprPrec5",  "ExprPrec6",
        "ExprPrec7",  "ExprPrec8",  "ExprPrec9",
        "ExprPrec10", "ExprPrec11", "ExprPrec12",
        "ExprPrec13", "ExprPrec14", "ExprPrec15",
    };

    handle_tree_node_one_of(node, valid_names, pExpressionOut);
}

void Program::handle_tree_node_expr_prec_1(qrawlr::ParseTreeNodeRef node, void* pExpressionOut)
{
    *(Expression<>*)pExpressionOut = expr_parse_helper_binary_op(
        node, EvaluationOrder::Right_to_Left,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            auto datatype = exprLeft->get_datatype();

            BinaryOperatorType opType;
            if (opInfo.value == "=") opType = BinaryOperatorType::Assign;
            else if (opInfo.value == "+=") opType = BinaryOperatorType::AssignSum;
            else if (opInfo.value == "-=") opType = BinaryOperatorType::AssignDifference;
            else if (opInfo.value == "*=") opType = BinaryOperatorType::AssignProduct;
            else if (opInfo.value == "/=") opType = BinaryOperatorType::AssignQuotient;
            else if (opInfo.value == "%=") opType = BinaryOperatorType::AssignModulo;
            else if (opInfo.value == "&=") opType = BinaryOperatorType::AssignLogicalAnd;
            else if (opInfo.value == "|=") opType = BinaryOperatorType::AssignLogicalOr;
            else if (opInfo.value == "^=") opType = BinaryOperatorType::AssignLogicalXor;
            else if (opInfo.value == "<<=") opType = BinaryOperatorType::AssignShiftLeft;
            else if (opInfo.value == ">>=") opType = BinaryOperatorType::AssignShiftRight;
            else throw QinpError::from_pos("Unknown operator'" + opInfo.value + "'!", opInfo.position);

            return Expression<ExpressionBinaryOperator>::make(
                opType,
                exprLeft,
                exprRight,
                datatype,
                opInfo.position
            );
        },
        nullptr
    );
}

void Program::handle_tree_node_expr_prec_2(qrawlr::ParseTreeNodeRef node, void* pExpressionOut)
{
    *(Expression<>*)pExpressionOut = expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            if (opInfo.value != "||")
                throw QinpError::from_pos("[*Program::handle_tree_node_expr_prec_2*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);

            auto datatype = DT_NAMED("bool", false);

            return Expression<ExpressionBinaryOperator>::make(
                BinaryOperatorType::LogicalOr,
                exprLeft,
                exprRight,
                datatype,
                opInfo.position
            );
        },
        nullptr
    );
}

void Program::handle_tree_node_expr_prec_3(qrawlr::ParseTreeNodeRef node, void* pExpressionOut)
{
    *(Expression<>*)pExpressionOut = expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            if (opInfo.value != "&&")
                throw QinpError::from_pos("[*Program::handle_tree_node_expr_prec_3*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);

            auto datatype = DT_NAMED("bool", false);

            return Expression<ExpressionBinaryOperator>::make(
                BinaryOperatorType::LogicalAnd,
                exprLeft,
                exprRight,
                datatype,
                opInfo.position
            );
        },
        nullptr
    );
}

void Program::handle_tree_node_expr_prec_4(qrawlr::ParseTreeNodeRef node, void* pExpressionOut)
{
    *(Expression<>*)pExpressionOut = expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            if (opInfo.value != "|")
                throw QinpError::from_pos("[*Program::handle_tree_node_expr_prec_4*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);

            auto datatype = exprLeft->get_datatype();

            return Expression<ExpressionBinaryOperator>::make(
                BinaryOperatorType::BitwiseOr,
                exprLeft,
                exprRight,
                datatype,
                opInfo.position
            );
        },
        nullptr
    );
}

void Program::handle_tree_node_expr_prec_5(qrawlr::ParseTreeNodeRef node, void* pExpressionOut)
{
    *(Expression<>*)pExpressionOut = expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            if (opInfo.value != "^")
                throw QinpError::from_pos("[*Program::handle_tree_node_expr_prec_5*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);

            auto datatype = exprLeft->get_datatype();

            return Expression<ExpressionBinaryOperator>::make(
                BinaryOperatorType::BitwiseXor,
                exprLeft,
                exprRight,
                datatype,
                opInfo.position
            );
        },
        nullptr
    );
}

void Program::handle_tree_node_expr_prec_6(qrawlr::ParseTreeNodeRef node, void* pExpressionOut)
{
    *(Expression<>*)pExpressionOut = expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            if (opInfo.value != "&")
                throw QinpError::from_pos("[*Program::handle_tree_node_expr_prec_6*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);

            auto datatype = exprLeft->get_datatype();

            return Expression<ExpressionBinaryOperator>::make(
                BinaryOperatorType::BitwiseAnd,
                exprLeft,
                exprRight,
                datatype,
                opInfo.position
            );
        },
        nullptr
    );
}

void Program::handle_tree_node_expr_prec_7(qrawlr::ParseTreeNodeRef node, void* pExpressionOut)
{
    *(Expression<>*)pExpressionOut = expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            BinaryOperatorType opType;

            if (opInfo.value == "==") opType = BinaryOperatorType::Equal;
            else if (opInfo.value == "!=") opType = BinaryOperatorType::NotEqual;
            else throw QinpError::from_pos("[*Program::handle_tree_node_expr_prec_7*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);

            auto datatype = DT_NAMED("bool", false);

            return Expression<ExpressionBinaryOperator>::make(
                opType,
                exprLeft,
                exprRight,
                datatype,
                opInfo.position
            );
        },
        nullptr
    );
}

void Program::handle_tree_node_expr_prec_8(qrawlr::ParseTreeNodeRef node, void* pExpressionOut)
{
    *(Expression<>*)pExpressionOut = expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            BinaryOperatorType opType;

            if (opInfo.value == "<=") opType = BinaryOperatorType::SmallerEquals;
            else if (opInfo.value == "<") opType = BinaryOperatorType::Smaller;
            else if (opInfo.value == ">=") opType = BinaryOperatorType::GreaterEquals;
            else if (opInfo.value == ">") opType = BinaryOperatorType::Greater;
            else throw QinpError::from_pos("[*Program::handle_tree_node_expr_prec_8*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);

            auto datatype = DT_NAMED("bool", false);

            return Expression<ExpressionBinaryOperator>::make(
                opType,
                exprLeft,
                exprRight,
                datatype,
                opInfo.position
            );
        },
        nullptr
    );
}

void Program::handle_tree_node_expr_prec_9(qrawlr::ParseTreeNodeRef node, void* pExpressionOut)
{
    *(Expression<>*)pExpressionOut = expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            BinaryOperatorType opType;

            if (opInfo.value == "<<") opType = BinaryOperatorType::ShiftLeft;
            else if (opInfo.value == ">>") opType = BinaryOperatorType::ShiftRight;
            else throw QinpError::from_pos("[*Program::handle_tree_node_expr_prec_9*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);

            auto datatype = exprLeft->get_datatype();

            return Expression<ExpressionBinaryOperator>::make(
                opType,
                exprLeft,
                exprRight,
                datatype,
                opInfo.position
            );
        },
        nullptr
    );
}

void Program::handle_tree_node_expr_prec_10(qrawlr::ParseTreeNodeRef node, void* pExpressionOut)
{
    *(Expression<>*)pExpressionOut = expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            BinaryOperatorType opType;

            if (opInfo.value == "+") opType = BinaryOperatorType::Sum;
            else if (opInfo.value == "-") opType = BinaryOperatorType::Difference;
            else throw QinpError::from_pos("[*Program::handle_tree_node_expr_prec_10*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);

            auto datatype = exprLeft->get_datatype();

            return Expression<ExpressionBinaryOperator>::make(
                opType,
                exprLeft,
                exprRight,
                datatype,
                opInfo.position
            );
        },
        nullptr
    );
}

void Program::handle_tree_node_expr_prec_11(qrawlr::ParseTreeNodeRef node, void* pExpressionOut)
{
    *(Expression<>*)pExpressionOut = expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            BinaryOperatorType opType;

            if (opInfo.value == "*") opType = BinaryOperatorType::Equal;
            else if (opInfo.value == "/") opType = BinaryOperatorType::NotEqual;
            else if (opInfo.value == "%") opType = BinaryOperatorType::Modulo;
            else throw QinpError::from_pos("[*Program::handle_tree_node_expr_prec_11*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);

            auto datatype = exprLeft->get_datatype();

            return Expression<ExpressionBinaryOperator>::make(
                opType,
                exprLeft,
                exprRight,
                datatype,
                opInfo.position
            );
        },
        nullptr
    );
}

void Program::handle_tree_node_expr_prec_12(qrawlr::ParseTreeNodeRef node, void* pExpressionOut)
{
    *(Expression<>*)pExpressionOut = expr_parse_helper_unary_op(
        node, EvaluationOrder::Right_to_Left,
        [&](OperatorInfo opInfo, Expression<> expr) -> Expression<>
        {
            (void)expr;

            UnaryOperatorType opType;
            Datatype<> datatype;

            if (opInfo.value == ".")
            {
                if (!expr.is_of_type<ExpressionIdentifier>())
                    throw QinpError::from_expr("Expected identifier after global-scope operator!", expr);
                
                auto& name = expr.as_type<ExpressionIdentifier>()->get_name();
                
                return Expression<ExpressionSymbol>::make(
                    m_root_sym->get_child_by_name(name),
                    opInfo.position
                );
            }
            else if (opInfo.value == "++")
            {
                opType = UnaryOperatorType::PrefixIncrement;
                datatype = expr->get_datatype();
            }
            else if (opInfo.value == "--")
            {
                opType = UnaryOperatorType::PrefixDecrement;
                datatype = expr->get_datatype();
            }
            else if (opInfo.value == "+")
            {
                opType = UnaryOperatorType::PrefixPlus;
                datatype = expr->get_datatype();
            }
            else if (opInfo.value == "-")
            {
                opType = UnaryOperatorType::PrefixMinus;
                datatype = expr->get_datatype();
            }
            else if (opInfo.value == "!")
            {
                opType = UnaryOperatorType::LogicalNot;
                datatype = DT_NAMED("bool", false);
            }
            else if (opInfo.value == "~")
            {
                opType = UnaryOperatorType::BitwiseNot;
                datatype = expr->get_datatype();
            }
            else if (opInfo.value == "*")
            {
                opType = UnaryOperatorType::Dereference;
                datatype = expr->get_datatype();
                // TODO: Modify datatype
                QinpError::from_pos("[*Program::handle_tree_node_expr_prec_12*]: Dereference operator not implemented yet!", opInfo.position);
            }
            else if (opInfo.value == "&")
            {
                opType = UnaryOperatorType::AddressOf;
                datatype = expr->get_datatype();
                // TODO: Modify datatype
                QinpError::from_pos("[*Program::handle_tree_node_expr_prec_12*]: Address-of operator not implemented yet!", opInfo.position);
            }
            else
                throw QinpError::from_pos("[*Program::handle_tree_node_expr_prec_12*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);
        
            return Expression<ExpressionUnaryOperator>::make(
                opType,
                expr,
                datatype,
                opInfo.position
            );
        },
        nullptr
    );
}

void Program::handle_tree_node_expr_prec_13(qrawlr::ParseTreeNodeRef node, void* pExpressionOut)
{
    *(Expression<>*)pExpressionOut = expr_parse_helper_unary_op(
        node, EvaluationOrder::Left_to_Right,
        [&](OperatorInfo opInfo, Expression<> expr) -> Expression<>
        {
            (void)expr;

            if (opInfo.value == "++")
            {
                QinpError::from_pos("[*Program::handle_tree_node_expr_prec_13*]: Postfix increment not implemented yet!", opInfo.position);
            }
            else if (opInfo.value == "--")
            {
                QinpError::from_pos("[*Program::handle_tree_node_expr_prec_13*]: Postfix decrement not implemented yet!", opInfo.position);
            }
            else
                throw QinpError::from_pos("[*Program::handle_tree_node_expr_prec_13*]: Unhandled operator '" + opInfo.value + "'", opInfo.position);

            return nullptr;
        },
        [&](qrawlr::ParseTreeNodeRef opTree, Expression<> expr) -> Expression<>
        {
            (void)expr;

            if (opTree->get_name() == "ExprOpFunctionCall")
            {
                if (expr.is_of_type<ExpressionIdentifier>())
                    expr = make_ExprSymbol_from_ExprIdentifier(expr);
                
                if (expr->get_datatype()->get_type() != Datatype<>::Type::Function &&
                    expr->get_datatype()->get_type() != Datatype<>::Type::FunctionName)
                    throw QinpError::from_node("Cannot call non-function!", opTree);

                // TODO: Handle blueprint parameters
                std::vector<Expression<>> arguments;

                auto& children = opTree->get_children();
                for (size_t argID = 1; argID < children.size(); ++argID)
                {
                    auto child = qrawlr::expect_child(opTree, std::to_string(argID) + ".FunctionCallArgument.0", get_fn_tree_id_to_name());
                    if (qrawlr::is_node(child, "Expression"))
                    {
                        Expression<> expr;
                        handle_tree_node(child, "Expression", &expr);
                        arguments.push_back(expr);
                    }
                    else if (qrawlr::is_leaf(child))
                        throw QinpError::from_node("[*Program::handle_tree_node_expr_prec_13*]: Variadic unpacking not implemented yet!", child);
                    else
                        throw QinpError::from_node("[*Program::handle_tree_node_expr_prec_13*]: Unhandled child in 'FunctionCallArgument'!", child);
                }

                return make_ExprFunctionCall(expr, arguments);

                throw QinpError::from_node("[*Program::handle_tree_node_expr_prec_13*]: Function calls not implemented yet!", opTree);
            }
            else if (opTree->get_name() == "ExprOpSubscript")
                throw QinpError::from_node("[*Program::handle_tree_node_expr_prec_13*]: Subscripts not implemented yet!", opTree);
            else
                throw QinpError::from_node("[*Program::handle_tree_node_expr_prec_13*]: Unhandled operator!", opTree);

            return nullptr;
        }
    );
}

void Program::handle_tree_node_expr_prec_14(qrawlr::ParseTreeNodeRef node, void* pExpressionOut)
{
    *(Expression<>*)pExpressionOut = expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        false,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            if (opInfo.value == ".")
            {
                if (!exprRight.is_of_type<ExpressionIdentifier>())
                    throw QinpError::from_expr("Expected identifier on right-hand-side of member access operator!", exprRight);
                auto& nameRight = exprRight.as_type<ExpressionIdentifier>()->get_name();

                // Convert identifier to symbol
                if (exprLeft.is_of_type<ExpressionIdentifier>())
                    exprLeft = make_ExprSymbol_from_ExprIdentifier(exprLeft);

                if (exprLeft->results_in_object())
                {
                    ;
                }
                else
                {
                    if (!exprLeft.is_of_type<ExpressionSymbol>())
                        throw QinpError::from_expr("Cannot access non-object non-symbol!", exprLeft);

                    return Expression<ExpressionSymbol>::make(
                        curr_tu()->get_symbol_from_path(nameRight, exprLeft.as_type<ExpressionSymbol>()->get_symbol()),
                        exprRight->get_position()
                    );
                }

                if (exprLeft.is_of_type<ExpressionSymbol>())
                {
                    ;
                }
                
                throw QinpError::from_pos("[*Program::handle_tree_node_expr_prec_14*]: Member access not implemented yet!", opInfo.position);
            }
            else if (opInfo.value == "->")
            {
                throw QinpError::from_pos("[*Program::handle_tree_node_expr_prec_14*]: Pointer member access not implemented yet!", opInfo.position);
            }
            
            throw QinpError::from_pos("[*Program::handle_tree_node_expr_prec_14*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);
        
            return exprLeft;
        },
        nullptr
    );
}

void Program::handle_tree_node_expr_prec_15(qrawlr::ParseTreeNodeRef node, void* pExpressionOut)
{
    auto sub_node = qrawlr::expect_child_node(node, "0", m_f_tree_id_to_name);
    if (sub_node->get_name() == "ExprPrec1")
        handle_appr_expr_prec(sub_node, pExpressionOut);
    else if (sub_node->get_name() == "LambdaDefinition")
        throw QinpError::from_node("[*Program::handle_tree_node_expr_prec_15*]: LambdaDefinition not implemented yet!", node);
    else if (sub_node->get_name() == "Identifier")
    {
        std::string name;
        handle_tree_node(sub_node, "Identifier", &name);
        *(Expression<>*)pExpressionOut = Expression<ExpressionIdentifier>::make(
            name,
            sub_node->get_pos_begin()
        );
    }
    else if (sub_node->get_name() == "Literal")
        throw QinpError::from_node("[*Program::handle_tree_node_expr_prec_15*]: Literal not implemented yet!", node);
    else
        throw QinpError::from_node("[*Program::handle_tree_node_expr_prec_15*]: Unhandled sub_node '" + sub_node->get_name() + "'!", node);
}

Expression<> Program::expr_parse_helper_unary_op(qrawlr::ParseTreeNodeRef superNode, EvaluationOrder eval_order, ExprGeneratorUnaryOpStr gen_expr_op_str, ExprGeneratorUnaryOpTree gen_expr_op_tree)
{
    auto& children = superNode->get_children();

    std::size_t id_curr;
    std::size_t id_end;
    std::size_t (*move_id)(std::size_t&);

    auto move_id_ltr = [](std::size_t& id){ return id++; };
    auto move_id_rtl = [](std::size_t& id){ return id--; };

    switch (eval_order)
    {
    case EvaluationOrder::Left_to_Right: id_curr = 0; id_end = children.size(); move_id = move_id_ltr; break;
    case EvaluationOrder::Right_to_Left: id_curr = children.size() - 1; id_end = -1; move_id = move_id_rtl; break;
    default: throw QinpError::from_node("[*Program::expr_parse_helper_unary_op*]: Invalid EvaluationOrder provided!", superNode);
    }

    Expression<> expr;
    handle_appr_expr_prec(qrawlr::expect_node(children[move_id(id_curr)], m_f_tree_id_to_name), &expr);

    while (id_curr != id_end)
    {
        auto opTree = qrawlr::expect_child(children[move_id(id_curr)], "0", m_f_tree_id_to_name);
        if (qrawlr::is_leaf(opTree))
        {
            if (!gen_expr_op_str)
                throw QinpError::from_node("[*Program::expr_parse_helper_unary_op*]: Handler for leaf-operators not provided!", opTree);

            expr = gen_expr_op_str(
                {
                    qrawlr::expect_leaf(opTree, m_f_tree_id_to_name)->get_value(),
                    opTree->get_pos_begin()
                },
                expr
            );
        }
        else
        {
            if (!gen_expr_op_str)
                throw QinpError::from_node("[*Program::expr_parse_helper_unary_op*]: Handler for node-operators not provided!", opTree);

            expr = gen_expr_op_tree(qrawlr::expect_node(opTree, m_f_tree_id_to_name), expr);
        }
    }

    return expr;
}

Expression<> Program::expr_parse_helper_binary_op(qrawlr::ParseTreeNodeRef superNode, EvaluationOrder eval_order, bool add_implicit_conversion, ExprGeneratorBinaryOpStr gen_expr_op_str, ExprGeneratorBinaryOpTree gen_expr_op_tree)
{
    auto& children = superNode->get_children();

    std::size_t id_curr;
    std::size_t id_end;
    std::size_t (*move_id)(std::size_t&);

    auto move_id_ltr = [](std::size_t& id){ return id++; };
    auto move_id_rtl = [](std::size_t& id){ return id--; };

    switch (eval_order)
    {
    case EvaluationOrder::Left_to_Right: id_curr = 0; id_end = children.size(); move_id = move_id_ltr; break;
    case EvaluationOrder::Right_to_Left: id_curr = children.size() - 1; id_end = -1; move_id = move_id_rtl; break;
    default: throw QinpError::from_node("[*Program::expr_parse_helper_binary_op*]: Invalid EvaluationOrder provided!", superNode);
    }

    Expression<> exprPrimary;
    handle_appr_expr_prec(qrawlr::expect_node(children[move_id(id_curr)], m_f_tree_id_to_name), &exprPrimary);

    while (id_curr != id_end)
    {
        Expression<> exprSecondary;
        auto opTree = qrawlr::expect_child(children[move_id(id_curr)], "0", m_f_tree_id_to_name);
        handle_appr_expr_prec(qrawlr::expect_node(children[move_id(id_curr)], m_f_tree_id_to_name), &exprSecondary);

        // TODO: Future: Check if overload exists (in EVERY handle_tree_node_expr_prec_*)

        if (add_implicit_conversion)
            add_implicit_conversion_to_same_datatype(exprPrimary, exprSecondary);

        if (eval_order == EvaluationOrder::Right_to_Left)
            exprPrimary.swap(exprSecondary);

        if (qrawlr::is_leaf(opTree))
        {
            if (!gen_expr_op_str)
                throw QinpError::from_node("[*Program::expr_parse_helper_binary_op*]: Handler for leaf-operators not provided!", opTree);

            exprPrimary = gen_expr_op_str(
                {
                    qrawlr::expect_leaf(opTree, m_f_tree_id_to_name)->get_value(),
                    opTree->get_pos_begin()
                },
                exprPrimary,
                exprSecondary
            );
        }
        else
        {
            if (!gen_expr_op_str)
                throw QinpError::from_node("[*Program::expr_parse_helper_binary_op*]: Handler for node-operators not provided!", opTree);

            exprPrimary = gen_expr_op_tree(
                qrawlr::expect_node(opTree, m_f_tree_id_to_name),
                exprPrimary,
                exprSecondary
            );
        }
    }

    return exprPrimary;
}

void Program::add_implicit_conversion_to_same_datatype(Expression<>& expr1, Expression<>& expr2) const
{
    (void)expr1, (void)expr2;
    // TODO: Implementation
}

Expression<ExpressionSymbol> Program::make_ExprSymbol_from_ExprIdentifier(Expression<> expr) const
{
    auto exprIdentifier = expr.as_type<ExpressionIdentifier>();
    if (!exprIdentifier)
        throw QinpError::from_expr("[*Program::make_ExprSymbol_from_ExprIdentifier*]: Provided expression is not of type 'ExpressionIdentifier'", expr);

    auto& name = exprIdentifier->get_name();
    auto symbol = curr_tu()->get_symbol_from_path(name);
    if (!symbol)
        throw QinpError::from_expr("Unknown identifier '" + name + "'", expr);

    return Expression<ExpressionSymbol>::make(
        symbol,
        expr->get_position()
    );
}

Expression<ExpressionFunctionCall> Program::make_ExprFunctionCall(Expression<>& expr, const std::vector<Expression<>>& arguments)
{
    auto exprSymbol = expr.as_type<ExpressionSymbol>();
    if (!exprSymbol)
        throw QinpError::from_expr("[*Program::make_ExprFunctionCall*]: Provided expression is not of type 'ExpressionSymbol'", expr);

    auto symbol = exprSymbol->get_symbol();

    if (!symbol.is_of_type<SymbolFunctionName>())
        throw QinpError::from_expr("[*Program::make_ExprFunctionCall*]: Provided expression does not resolve to symbol of type 'SymbolFunctionName'", expr);
    
    Expression<ExpressionFunctionCall> bestCandidateExpr;

    for (auto child : symbol->get_children())
    {
        auto funcSym = child.second;
        if (!funcSym.is_of_type<SymbolFunction>())
            throw QinpError::from_expr("[*Program::make_ExprFunctionCall*]: Invalid child symbol found in symbol of type 'SymbolFunctionName'", expr);

        auto callExpr = Expression<ExpressionFunctionCall>::make(funcSym.as_type<SymbolFunction>(), arguments);

        if (!bestCandidateExpr || callExpr->get_match_level() < bestCandidateExpr->get_match_level())
            bestCandidateExpr = callExpr;
        else if (callExpr->get_match_level() == bestCandidateExpr->get_match_level())
            throw QinpError::from_expr("[*Program::make_ExprFunctionCall*]: Ambiguous function call", expr);
    }

    if (!bestCandidateExpr)
        throw QinpError::from_expr("[*Program::make_ExprFunctionCall*]: No candidate found for function cal!", expr);

    return bestCandidateExpr;
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

TranslationUnitRef Program::curr_tu() const
{
    return m_translation_units.top();
}

TranslationUnitRef Program::pop_tu()
{
    auto tu = curr_tu();
    m_translation_units.pop();
    return tu;
}

ProgramRef Program::get()
{
    if (!s_singleton)
        throw QinpError("[*Program::get*]: Program singleton has not been initialized yet!");
    return s_singleton;
}

void Program::init(Architecture arch, Platform platform, CmdFlags flags)
{
    s_singleton = std::shared_ptr<Program>(new Program(arch, platform, flags));
}