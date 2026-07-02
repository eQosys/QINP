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
    m_sym_root(Symbol<SymbolSpace>::make("", qrawlr::Position())),
    m_grammar(
        qrawlr::Grammar::load_from_text(
            QINP_GRAMMAR_C_STR,
            QINP_GRAMMAR_ORIGIN_C_STR
        )
    )
{
    m_sym_strings = m_sym_root.add_child(
        Symbol<SymbolSpace>::make("~strings~", qrawlr::Position()),
        DuplicateHandling::Throw
    ).as_type<SymbolSpace>();
}

Program::~Program()
{
    if (m_flags.is_set(CmdFlag::Render_Symbol_Tree))
    {
        system("mkdir -p out/");

        if (m_flags.is_set(CmdFlag::Verbose))
            printf("Generating symbol tree graphviz...\n");
        qrawlr::write_file("out/tree.gv", m_sym_root->to_digraph_str(m_flags.is_set(CmdFlag::Verbose)));

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
    case Architecture::Unknown: throw QinpError("Unknown architecture!");
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
        throw qrawlr::GrammarException("Could not parse remaining source", result.pos_end);

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

    push_tu(path_str);
    parse_code_block(qrawlr::expect_node(result.tree));
    pop_tu();
}

void Program::parse_code_block(qrawlr::ParseTreeNodeRef node)
{
    for (auto child : node->get_children())
        parse_code_block_item(qrawlr::expect_node(child));
}
void Program::parse_code_block_item(qrawlr::ParseTreeNodeRef node)
{
    const std::string& name = node->get_name();
    if      (name == "Comment")                     parse_comment(node);
    else if (name == "StatementImport")             parse_stmt_import(node);
    else if (name == "StatementSpace")              parse_stmt_space(node);
    else if (name == "StatementFunctionDeclDef")    parse_stmt_func_decl_def(node);
    else if (name == "StatementDefer")              parse_stmt_defer(node);
    else if (name == "StatementEnumDeclDef")        parse_stmt_enum_decl_def(node);
    else if (name == "StatementVariableDeclDef")    parse_stmt_var_decl_def(node);
    else if (name == "StatementReturn")             parse_stmt_return(node);
    else if (name == "StatementIfElifElse")         parse_stmt_if_elif_else(node);
    else if (name == "Expression")                  parse_expression(node);
    else if (name == "StatementPass")               throw QinpError::from_node("[*Program::parse_code_block_item*]: " + name + " not implemented yet", node);
    else if (name == "StatementDefine")             throw QinpError::from_node("[*Program::parse_code_block_item*]: " + name + " not implemented yet", node);
    else if (name == "StatementWhile")              throw QinpError::from_node("[*Program::parse_code_block_item*]: " + name + " not implemented yet", node);
    else if (name == "StatementDoWhile")            throw QinpError::from_node("[*Program::parse_code_block_item*]: " + name + " not implemented yet", node);
    else if (name == "StatementPackDeclDef")        throw QinpError::from_node("[*Program::parse_code_block_item*]: " + name + " not implemented yet", node);
    else if (name == "StatementExternFunctionDecl") throw QinpError::from_node("[*Program::parse_code_block_item*]: " + name + " not implemented yet", node);
    else if (name == "StatementInlineAssembly")     throw QinpError::from_node("[*Program::parse_code_block_item*]: " + name + " not implemented yet", node);
    else throw QinpError::from_node("[*Program::parse_code_block_item*]: Unhandled code block item: " + name, node);
}

void Program::parse_stmt_import(qrawlr::ParseTreeNodeRef node)
{
    qrawlr::Flags<ImportSpecifier> flags = parse_import_specifiers(qrawlr::expect_child_node(node, "ImportSpecifiers"));
    // TODO: handle flags

    std::string path_str = parse_literal_string(qrawlr::expect_child_node(node, "LiteralString"));
    replace_all(path_str, "{architecture}", architecture_to_string(m_architecture));
    replace_all(path_str, "{platform}", platform_to_string(m_platform));

    if (m_flags.is_set(CmdFlag::Verbose))
        printf("Importing '%s'... (requested by '%s')\n", path_str.c_str(), curr_tu()->get_file_path().c_str());

    import_source_file(path_str, true);
}

void Program::parse_stmt_space(qrawlr::ParseTreeNodeRef node)
{
    std::string space_name = qrawlr::expect_child_leaf(node, "SpaceHeader.SpaceName.Identifier.0")->get_value();

    auto space = curr_tu()->curr_symbol().add_child(
        Symbol<SymbolSpace>::make(space_name, node->get_pos_begin()),
        DuplicateHandling::Keep
    ).as_type<SymbolSpace>();

    if (!space->is_defined())
        space->set_definition(node->get_pos_begin());

    curr_tu()->enter_symbol(space);
    parse_code_block(qrawlr::expect_child_node(node, "CodeBlock"));
    curr_tu()->leave_symbol();
}

void Program::parse_stmt_func_decl_def(qrawlr::ParseTreeNodeRef node)
{
    Symbol sym = parse_func_header(qrawlr::expect_child_node(node, "FunctionHeader"));

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
            func.as_type<SymbolFunctionSpecialization>()->set_definition(node->get_pos_begin());

            curr_tu()->enter_symbol(func);
            parse_code_block(qrawlr::expect_child_node(node, "FunctionDefinition.CodeBlock"));
            curr_tu()->leave_symbol();
        }
        else if (func.is_of_type<SymbolFunctionBlueprint>())
        {
            func.as_type<SymbolFunctionBlueprint>()->set_definition(qrawlr::expect_child_node(node, "FunctionDefinition.CodeBlock"));
        }
        else
        {
            throw QinpError::from_node("[*Program::parse_stmt_func_decl_def*]: Unknown SymbolFunction* type!", node);
        }
    }
    else
    {
        throw QinpError::from_node("[*Program::parse_stmt_func_decl_def*]: Missing 'FunctionDeclaration' or 'FunctionDefinition' node!", node);
    }

    // TODO: Remove debug print
    printf("Reached function with name '%s'\n", sym->get_symbol_path().to_string().c_str());
}

Symbol<> Program::parse_func_header(qrawlr::ParseTreeNodeRef node)
{
    Symbol<> sym;
    Datatype return_type;
    SymbolPath func_name_path;
    Parameter_Decl parameters;
    bool is_nodiscard = false;

    return_type = parse_optional_datatype(qrawlr::expect_child_node(node, "OptionalDatatype"));
    func_name_path = parse_symbol_reference(qrawlr::expect_child_node(node, "SymbolReference"));
    parameters = parse_func_params(qrawlr::expect_child_node(node, "FunctionParameters"));
    // TODO: Handle Function Specifiers

    // Get SymbolFunctionName
    sym = curr_tu()->get_symbol_from_path(func_name_path.get_parent_path()).add_child(
        Symbol<SymbolFunctionName>::make(
            func_name_path.get_name(),
            node->get_pos_begin()
        ),
        DuplicateHandling::Keep
    );

    if (!sym.is_of_type<SymbolFunctionName>())
        throw QinpError::from_node("Cannot declare/define function with name '" +
            func_name_path.get_name() +
            "', non-function symbol with same name already defined here: " +
            sym->get_position().to_string(), node);

    // Create blueprint symbol if necessary
    if (parameters.is_blueprint)
    {
        sym = sym.add_child(
            Symbol<SymbolFunctionBlueprint>::make(
                return_type,
                parameters,
                is_nodiscard,
                node->get_pos_begin()
            ),
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
            DuplicateHandling::Keep
        );
    }
    return sym;
}

Datatype<> Program::parse_optional_datatype(qrawlr::ParseTreeNodeRef node)
{
    if (!qrawlr::has_child_node(node, "Datatype")) // No return type specified
        return DT_NAMED("void", false);

    return parse_datatype(qrawlr::expect_child_node(node, "Datatype"));
}

Parameter_Decl Program::parse_func_params(qrawlr::ParseTreeNodeRef node)
{
    Parameter_Decl parameters;

    for (auto tree_ref : node->get_children())
    {
        if (!parameters.named_parameters.empty() && parameters.named_parameters.back().datatype->is_variadic())
            throw QinpError::from_node("Variadic parameter must be at the end of the parameter list", tree_ref);

        auto param_node = qrawlr::expect_node(tree_ref);
        if (param_node->get_name() == "Normal")
        {
            Parameter param;
            param.datatype = parse_datatype(qrawlr::expect_child_node(param_node, "Datatype"));
            param.name = parse_identifier(qrawlr::expect_child_node(param_node, "Identifier"));
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
            throw QinpError::from_node("[*Program::parse_func_params*]: Unhandled parameter type!", node);
        }
    }

    return parameters;
}

qrawlr::Flags<ImportSpecifier> Program::parse_import_specifiers(qrawlr::ParseTreeNodeRef node)
{
    qrawlr::Flags<ImportSpecifier> flags;

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
            throw QinpError::from_node("[*Program::parse_import_specifiers*]: Unknown import specifier '" + value + "'", child);
    }

    return flags;
}

Expression<> Program::parse_literal_integer(qrawlr::ParseTreeNodeRef node)
{
	throw QinpError::from_node("[*Program::parse_literal_integer*]: Not implemented yet!", node);
}

std::string Program::parse_literal_string(qrawlr::ParseTreeNodeRef node)
{
    std::string string_out;

    for (auto child : node->get_children())
    {
        if (qrawlr::is_leaf(child))
        {
            string_out.append(qrawlr::get_leaf(child)->get_value());
            continue;
        }

		char chr = parse_escape_sequence(qrawlr::expect_node(child));
		string_out.push_back(chr);
    }

    return string_out;
}

char Program::parse_literal_char(qrawlr::ParseTreeNodeRef node)
{
	throw QinpError::from_node("[*Program::parse_literal_char*]: Not implemented yet!", node);
}

char Program::parse_escape_sequence(qrawlr::ParseTreeNodeRef node)
{
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

    std::string seq = qrawlr::expect_child_leaf(node, "0")->get_value();

    if (seq[0] == 'x')
    {
        return (char)std::stoi(seq.substr(1), nullptr, 16);
    }
    else
    {
        auto it = sequences.find(seq[0]);

        if (it == sequences.end())
            throw QinpError::from_node("Unknown escape sequence", node);

		return it->second;
    }
}

void Program::parse_comment(qrawlr::ParseTreeNodeRef node)
{
    (void)node;

    // TODO: comment extraction
}


Datatype<> Program::parse_datatype(qrawlr::ParseTreeNodeRef node)
{
    Datatype<> dt;

    if (qrawlr::has_child_node(node, "DatatypeBlueprint"))
    {
        // TODO: More elaborate blueprint handling
        std::string identifier = parse_identifier(qrawlr::expect_child_node(node, "DatatypeBlueprint.Identifier"));
        return DT_MACRO(identifier, false);
    }
    else if (qrawlr::has_child_node(node, "DatatypeFunction"))
    {
        // TODO: Handle constness
        Datatype return_type = parse_optional_datatype(qrawlr::expect_child_node(node, "DatatypeFunction.OptionalDatatype"));
        Parameter_Decl decl_params = parse_func_params(qrawlr::expect_child_node(node, "DatatypeFunction.DatatypeFunctionParameters"));
        Parameter_Types parameters;

        for (const auto& p : decl_params.named_parameters)
            parameters.push_back(p.datatype);

        return DT_FUNCTION(return_type, parameters, false);
    }
    
    node = qrawlr::expect_child_node(node, "DatatypeNamed");
    auto& children = node->get_children();

    for (auto it = children.begin(); it != children.end(); ++it)
    {
        auto elem = qrawlr::expect_node(*it);
        
        bool is_const = false;
        if (++it != children.end() && qrawlr::is_node(*it, "Const"))
            is_const = true;
        else
            --it;

        if (elem->get_name() == "SymbolReference")
        {
            SymbolPath name_path;
            name_path = parse_symbol_reference(elem);
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
            Expression<> expr;
            expr = parse_expression(elem);
            if (!expr->is_const_expr())
                throw QinpError::from_node("Expected array size constexpr!", elem);
            auto result = expr->eval_const_expr();

            throw QinpError::from_node("[*Program::parse_datatype*]: Cannot evaluate constexpr at the moment!", elem);
            int num_elements = 1;
            dt = DT_ARRAY(num_elements, dt, is_const);
        }
        else if (elem->get_name() == "Reference")
        {
            // TODO: Implementation
            throw QinpError::from_node("[*Program::parse_datatype*]: Handling of datatype reference not implemented yet!", node);
            dt = DT_REFERENCE(dt, is_const);
        }
        else
        {
            throw QinpError::from_node("[*Program::parse_datatype*]: Unhandled element type!", node);
        }
    }

    return dt;
}

std::string Program::parse_identifier(qrawlr::ParseTreeNodeRef node)
{
    return qrawlr::expect_child_leaf(node, "0")->get_value();
}

SymbolPath Program::parse_symbol_reference(qrawlr::ParseTreeNodeRef node)
{
    std::vector<std::string> elements;
    bool is_from_root = qrawlr::has_child_leaf(node, "0");

    auto& children = node->get_children();
    for (std::size_t i = is_from_root ? 1 : 0; i < children.size(); ++i)
    {
        std::string identifier;
        identifier = parse_identifier(qrawlr::expect_node(children[i]));
        elements.push_back(identifier);
    }

    return SymbolPath(elements, is_from_root);
}

void Program::parse_stmt_defer(qrawlr::ParseTreeNodeRef node)
{
    (void)node;

    // TODO: Proper deferring
    m_deferred_translation_units.push(pop_tu());
    push_tu(m_deferred_translation_units.front());
    m_deferred_translation_units.pop();
}

void Program::parse_stmt_enum_decl_def(qrawlr::ParseTreeNodeRef node)
{

    Symbol symEnum;
    symEnum = parse_enum_header(qrawlr::expect_child_node(node, "EnumHeader"));

    if (qrawlr::has_child_node(node, "EnumDeclaration"))
    {
        ; // Nothing to do
    }
    else if (qrawlr::has_child_node(node, "EnumDefinition"))
    {
        parse_enum_def(qrawlr::expect_child_node(node, "EnumDefinition"), symEnum);
    }
    else
    {
        throw QinpError::from_node("[*Program::parse_stmt_enum_decl_def*]: Missing 'EnumDeclaration' or 'EnumDefinition' node!", node);
    }
}

Symbol<> Program::parse_enum_header(qrawlr::ParseTreeNodeRef node)
{
    Symbol<> enumSym;

    SymbolPath enum_path;
    enum_path = parse_symbol_reference(qrawlr::expect_child_node(node, "SymbolReference"));
    
    enumSym = curr_tu()->get_symbol_from_path(enum_path.get_parent_path()).add_child(
        Symbol<SymbolEnum>::make(
            enum_path.get_name(),
            node->get_pos_begin()
        ),
        DuplicateHandling::Keep
    );

    return enumSym;
}

void Program::parse_enum_def(qrawlr::ParseTreeNodeRef node, Symbol<>& enumSym)
{
    for (auto& child : qrawlr::expect_child_node(node, "EnumMemberList")->get_children())
        parse_enum_member_def(qrawlr::expect_node(child), enumSym);
}

void Program::parse_enum_member_def(qrawlr::ParseTreeNodeRef node, Symbol<>& enumSym)
{
    std::string name;
    name = parse_identifier(qrawlr::expect_child_node(node, "Identifier"));

    std::size_t value = enumSym.as_type<SymbolEnum>()->next_value();
    if (qrawlr::has_child_node(node, "Expression"))
    {
        // TODO: Implementation
        throw QinpError::from_node("[*Program::parse_enum_member_def*]: Member with initializer not implemented yet!", node);
    }

    enumSym.add_child(
        Symbol<SymbolEnumMember>::make(
            name,
            value,
            node->get_pos_begin()
        )
    );
}

void Program::parse_stmt_var_decl_def(qrawlr::ParseTreeNodeRef node)
{
    VariableDeclarators declarators;
    Datatype<> datatype;
    std::string name;
    Expression<> initializer;

    declarators = parse_var_declarators(qrawlr::expect_child_node(node, "VariableDeclarators"));
    datatype = parse_optional_datatype(qrawlr::expect_child_node(node, "OptionalDatatype"));
    name = parse_identifier(qrawlr::expect_child_node(node, "VariableName.Identifier"));
    initializer = parse_var_initializer(qrawlr::expect_child_node(node, "VariableInitializer"));

    curr_tu()->curr_symbol().add_child(
        Symbol<SymbolVariable>::make(
            name,
            declarators,
            datatype,
            node->get_pos_begin()
        ),
        DuplicateHandling::Throw
    );

    // TODO: Append initializer to current space body
}

VariableDeclarators Program::parse_var_declarators(qrawlr::ParseTreeNodeRef node)
{
    VariableDeclarators declarators;

    for (auto& child : node->get_children())
    {
        auto declarator = qrawlr::expect_leaf(child);
        auto& value = declarator->get_value();
        if (value == "static")
            declarators.is_static = true;
        else if (value == "var")
            ; // Nothing to do
        else if (value == "const")
            declarators.is_const = true;
        else
            throw QinpError::from_node("[*Program::parse_var_declarators*]: Unexpected declarator '" + value + "'!", child);
    }

    return declarators;
}

Expression<> Program::parse_var_initializer(qrawlr::ParseTreeNodeRef node)
{
    Expression<> initializer;
    if (qrawlr::has_child_node(node, "Expression"))
        return parse_expression(qrawlr::expect_child_node(node, "Expression"));

    return nullptr;
}

void Program::parse_stmt_return(qrawlr::ParseTreeNodeRef node)
{
    // TODO: Check if current scope allows return statement

    Expression<> expr;
    expr = parse_expression(qrawlr::expect_child_node(node, "Expression"));

    curr_tu()->curr_symbol().as_type<SymbolSpace>()->append_expr(
        Expression<ExpressionReturn>::make(
            expr,
            node->get_pos_begin()
        )
    );
}

void Program::parse_stmt_if_elif_else(qrawlr::ParseTreeNodeRef node)
{
	auto gen_space = [](Symbol<> baseSym, const std::string& typeName, const qrawlr::Position& position)
	{
		return baseSym.add_child(
			Symbol<SymbolSpace>::make(Symbol<>::gen_unique_name(typeName), position),
			DuplicateHandling::Throw
		).as_type<SymbolSpace>();
	};

    auto if_elif_helper = [this, gen_space](const std::string& typeName, qrawlr::ParseTreeNodeRef stmtNode)
    {
        Expression<> condition;
        condition = parse_expression(qrawlr::expect_child_node(stmtNode, "Expression"));
    
		auto space = gen_space(curr_tu()->curr_symbol(), typeName, stmtNode->get_pos_begin());

		auto expr = curr_tu()->curr_symbol().as_type<SymbolSpace>()->append_expr(
			Expression<ExpressionIf>::make(
            	condition, space,
				stmtNode->get_pos_begin()
        	)
		).as_type<ExpressionIf>();

        curr_tu()->enter_symbol(space);
        parse_code_block(qrawlr::expect_child_node(stmtNode, "CodeBlock"));
        curr_tu()->leave_symbol();

        return expr;
    };

    auto exprTemp = if_elif_helper("if", qrawlr::expect_child_node(node, "StatementIf"));

	int i = 0;
	while (qrawlr::has_child_node(node, "StatementElif#" + std::to_string(i)))
	{
		auto nodeElif = qrawlr::expect_child_node(node, "StatementElif#" + std::to_string(i));
		auto spaceElif = gen_space(curr_tu()->curr_symbol(), "elif", nodeElif->get_pos_begin());

		exprTemp->set_body_false(spaceElif);

		curr_tu()->enter_symbol(spaceElif);
		auto exprElif = if_elif_helper("elif", nodeElif);
		curr_tu()->leave_symbol();

		exprTemp = exprElif;

		++i;
	}

	if (qrawlr::has_child_node(node, "StatementElse"))
	{
		auto nodeElse = qrawlr::expect_child_node(node, "StatementElse");
		auto spaceElse = gen_space(curr_tu()->curr_symbol(), "else", nodeElse->get_pos_begin());

		exprTemp->set_body_false(spaceElse);

		curr_tu()->enter_symbol(spaceElse);
        parse_code_block(qrawlr::expect_child_node(nodeElse, "CodeBlock"));
		curr_tu()->leave_symbol();
	}
}

Expression<> Program::parse_expression(qrawlr::ParseTreeNodeRef node)
{
    Expression<> expr_to_add = parse_appr_expr_prec(qrawlr::expect_child_node(node, "0"));

    if (expr_to_add)
        curr_tu()->curr_symbol().as_type<SymbolSpace>()->append_expr(expr_to_add);

    return expr_to_add;
}

Expression<> Program::parse_appr_expr_prec(qrawlr::ParseTreeNodeRef node)
{
    const std::string& name = node->get_name();
    if      (name == "ExprPrec1")  return parse_expr_prec_1(node);
    else if (name == "ExprPrec2")  return parse_expr_prec_2(node);
    else if (name == "ExprPrec3")  return parse_expr_prec_3(node);
    else if (name == "ExprPrec4")  return parse_expr_prec_4(node);
    else if (name == "ExprPrec5")  return parse_expr_prec_5(node);
    else if (name == "ExprPrec6")  return parse_expr_prec_6(node);
    else if (name == "ExprPrec7")  return parse_expr_prec_7(node);
    else if (name == "ExprPrec8")  return parse_expr_prec_8(node);
    else if (name == "ExprPrec9")  return parse_expr_prec_9(node);
    else if (name == "ExprPrec10") return parse_expr_prec_10(node);
    else if (name == "ExprPrec11") return parse_expr_prec_11(node);
    else if (name == "ExprPrec12") return parse_expr_prec_12(node);
    else if (name == "ExprPrec13") return parse_expr_prec_13(node);
    else if (name == "ExprPrec14") return parse_expr_prec_14(node);
    else if (name == "ExprPrec15") return parse_expr_prec_15(node);
    else throw QinpError::from_node("[*Program::parse_appr_expr_prec*]: Unhandled expr prec: " + name, node);
}

Expression<> Program::parse_expr_prec_1(qrawlr::ParseTreeNodeRef node)
{
    return expr_parse_helper_binary_op(
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
            else throw QinpError::from_pos("Unknown operator: '" + opInfo.value + "'!", opInfo.position);

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

Expression<> Program::parse_expr_prec_2(qrawlr::ParseTreeNodeRef node)
{
    return expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            if (opInfo.value != "||")
                throw QinpError::from_pos("[*Program::parse_expr_prec_2*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);

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

Expression<> Program::parse_expr_prec_3(qrawlr::ParseTreeNodeRef node)
{
    return expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            if (opInfo.value != "&&")
                throw QinpError::from_pos("[*Program::parse_expr_prec_3*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);

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

Expression<> Program::parse_expr_prec_4(qrawlr::ParseTreeNodeRef node)
{
    return expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            if (opInfo.value != "|")
                throw QinpError::from_pos("[*Program::parse_expr_prec_4*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);

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

Expression<> Program::parse_expr_prec_5(qrawlr::ParseTreeNodeRef node)
{
    return expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            if (opInfo.value != "^")
                throw QinpError::from_pos("[*Program::parse_expr_prec_5*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);

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

Expression<> Program::parse_expr_prec_6(qrawlr::ParseTreeNodeRef node)
{
    return expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            if (opInfo.value != "&")
                throw QinpError::from_pos("[*Program::parse_expr_prec_6*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);

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

Expression<> Program::parse_expr_prec_7(qrawlr::ParseTreeNodeRef node)
{
    return expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            BinaryOperatorType opType;

            if (opInfo.value == "==") opType = BinaryOperatorType::Equal;
            else if (opInfo.value == "!=") opType = BinaryOperatorType::NotEqual;
            else throw QinpError::from_pos("[*Program::parse_expr_prec_7*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);

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

Expression<> Program::parse_expr_prec_8(qrawlr::ParseTreeNodeRef node)
{
    return expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            BinaryOperatorType opType;

            if (opInfo.value == "<=") opType = BinaryOperatorType::SmallerEquals;
            else if (opInfo.value == "<") opType = BinaryOperatorType::Smaller;
            else if (opInfo.value == ">=") opType = BinaryOperatorType::GreaterEquals;
            else if (opInfo.value == ">") opType = BinaryOperatorType::Greater;
            else throw QinpError::from_pos("[*Program::parse_expr_prec_8*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);

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

Expression<> Program::parse_expr_prec_9(qrawlr::ParseTreeNodeRef node)
{
    return expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            BinaryOperatorType opType;

            if (opInfo.value == "<<") opType = BinaryOperatorType::ShiftLeft;
            else if (opInfo.value == ">>") opType = BinaryOperatorType::ShiftRight;
            else throw QinpError::from_pos("[*Program::parse_expr_prec_9*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);

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

Expression<> Program::parse_expr_prec_10(qrawlr::ParseTreeNodeRef node)
{
    return expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            BinaryOperatorType opType;

            if (opInfo.value == "+") opType = BinaryOperatorType::Sum;
            else if (opInfo.value == "-") opType = BinaryOperatorType::Difference;
            else throw QinpError::from_pos("[*Program::parse_expr_prec_10*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);

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

Expression<> Program::parse_expr_prec_11(qrawlr::ParseTreeNodeRef node)
{
    return expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        true,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            BinaryOperatorType opType;

            if (opInfo.value == "*") opType = BinaryOperatorType::Equal;
            else if (opInfo.value == "/") opType = BinaryOperatorType::NotEqual;
            else if (opInfo.value == "%") opType = BinaryOperatorType::Modulo;
            else throw QinpError::from_pos("[*Program::parse_expr_prec_11*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);

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

Expression<> Program::parse_expr_prec_12(qrawlr::ParseTreeNodeRef node)
{
    return expr_parse_helper_unary_op(
        node, EvaluationOrder::Right_to_Left,
        [&](OperatorInfo opInfo, Expression<> expr) -> Expression<>
        {
            UnaryOperatorType opType;
            Datatype<> datatype;

            if (opInfo.value == ".")
            {
                if (!expr.is_of_type<ExpressionIdentifier>())
                    throw QinpError::from_expr("Expected identifier after global-scope operator!", expr);
                
                auto& name = expr.as_type<ExpressionIdentifier>()->get_name();
                
                return Expression<ExpressionSymbol>::make(
                    m_sym_root->get_child_by_name(name),
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
                datatype = datatype->get_dereferenced();
            }
            else if (opInfo.value == "&")
            {
                opType = UnaryOperatorType::AddressOf;
                datatype = expr->get_datatype();
                datatype = DT_POINTER(datatype, false);
            }
            else
                throw QinpError::from_pos("[*Program::parse_expr_prec_12*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);
        
            return Expression<ExpressionUnaryOperator>::make(
                opType,
                expr,
                datatype,
                opInfo.position
            );
        },
        [&](qrawlr::ParseTreeNodeRef opTree, Expression<> expr) -> Expression<>
        {
            if (opTree->get_name() == "ExprOpTypeCast")
            {
				Datatype<> target_type = parse_datatype(qrawlr::expect_child_node(opTree, "Datatype"));

				// TODO: Check for user defined conversion (implicit function call)

				return Expression<ExpressionTypeCast>::make(
					target_type,
					expr
				);
            }
            else
                throw QinpError::from_node("[*Program::parse_expr_prec_12*]: Unhandled operator!", opTree);

            return nullptr;
        }
    );
}

Expression<> Program::parse_expr_prec_13(qrawlr::ParseTreeNodeRef node)
{
    return expr_parse_helper_unary_op(
        node, EvaluationOrder::Left_to_Right,
        [&](OperatorInfo opInfo, Expression<> expr) -> Expression<>
        {
            (void)expr;

            if (opInfo.value == "++")
            {
                QinpError::from_pos("[*Program::parse_expr_prec_13*]: Postfix increment not implemented yet!", opInfo.position);
            }
            else if (opInfo.value == "--")
            {
                QinpError::from_pos("[*Program::parse_expr_prec_13*]: Postfix decrement not implemented yet!", opInfo.position);
            }
            else
                throw QinpError::from_pos("[*Program::parse_expr_prec_13*]: Unhandled operator '" + opInfo.value + "'", opInfo.position);

            return nullptr;
        },
        [&](qrawlr::ParseTreeNodeRef opTree, Expression<> expr) -> Expression<>
        {
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
                    auto child = qrawlr::expect_child(opTree, std::to_string(argID) + ".FunctionCallArgument.0");
                    if (qrawlr::is_node(child, "Expression"))
                    {
                        Expression<> expr;
                        expr = parse_expression(qrawlr::expect_node(child));
                        arguments.push_back(expr);
                    }
                    else if (qrawlr::is_leaf(child))
                        throw QinpError::from_node("[*Program::parse_expr_prec_13*]: Variadic unpacking not implemented yet!", child);
                    else
                        throw QinpError::from_node("[*Program::parse_expr_prec_13*]: Unhandled child in 'FunctionCallArgument'!", child);
                }

                return make_ExprFunctionCall(expr, arguments);
            }
            else if (opTree->get_name() == "ExprOpSubscript")
            {
                Expression<> subExpr;
                subExpr = parse_expression(qrawlr::expect_child_node(opTree, "Expression"));

                return Expression<ExpressionSubscript>::make(
                    expr, subExpr
                );
            }
            else
                throw QinpError::from_node("[*Program::parse_expr_prec_13*]: Unhandled operator!", opTree);

            return nullptr;
        }
    );
}

Expression<> Program::parse_expr_prec_14(qrawlr::ParseTreeNodeRef node)
{
    return expr_parse_helper_binary_op(
        node, EvaluationOrder::Left_to_Right,
        false,
        [&](OperatorInfo opInfo, Expression<> exprLeft, Expression<> exprRight) -> Expression<>
        {
            if (opInfo.value == ".")
            {
                if (!exprRight.is_of_type<ExpressionIdentifier>())
                    throw QinpError::from_expr("Expected identifier on right-hand-side of member access operator!", exprRight);
                auto& nameRight = exprRight.as_type<ExpressionIdentifier>()->get_name();

                // Convert identifier to symbol expression
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
                
                throw QinpError::from_pos("[*Program::parse_expr_prec_14*]: Member access not implemented yet!", opInfo.position);
            }
            else if (opInfo.value == "->")
            {
                throw QinpError::from_pos("[*Program::parse_expr_prec_14*]: Pointer member access not implemented yet!", opInfo.position);
            }
            
            throw QinpError::from_pos("[*Program::parse_expr_prec_14*]: Unhandled operator '" + opInfo.value + "'!", opInfo.position);
        
            return exprLeft;
        },
        nullptr
    );
}

Expression<> Program::parse_expr_prec_15(qrawlr::ParseTreeNodeRef node)
{
    auto sub_node = qrawlr::expect_child_node(node, "0");
    if (sub_node->get_name() == "ExprPrec1")
        return parse_appr_expr_prec(sub_node);
    else if (sub_node->get_name() == "LambdaDefinition")
        throw QinpError::from_node("[*Program::parse_expr_prec_15*]: LambdaDefinition not implemented yet!", sub_node);
    else if (sub_node->get_name() == "Identifier")
    {
        std::string name = parse_identifier(sub_node);

        return Expression<ExpressionIdentifier>::make(
            name,
            sub_node->get_pos_begin()
        );
    }
    else if (sub_node->get_name() == "LiteralInteger")
	{
		return parse_literal_integer(sub_node);
	}
	else if (sub_node->get_name() == "LiteralString")
	{
		std::string value = parse_literal_string(sub_node);

        return make_ExprString(value, sub_node->get_pos_begin());
	}
    else if (sub_node->get_name() == "LiteralChar")
	{
		parse_literal_char(sub_node);
        throw QinpError::from_node("[*Program::parse_expr_prec_15*]: LiteralChar parsing incomplete", sub_node);
	}
    else
        throw QinpError::from_node("[*Program::parse_expr_prec_15*]: Unhandled sub_node '" + sub_node->get_name() + "'!", node);
        
    return nullptr;
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
    expr = parse_appr_expr_prec(qrawlr::expect_node(children[move_id(id_curr)]));

    while (id_curr != id_end)
    {
        auto opTree = qrawlr::expect_child(children[move_id(id_curr)], "0");
        if (qrawlr::is_leaf(opTree))
        {
            if (!gen_expr_op_str)
                throw QinpError::from_node("[*Program::expr_parse_helper_unary_op*]: Handler for leaf-operators not provided!", opTree);

            expr = gen_expr_op_str(
                {
                    qrawlr::expect_leaf(opTree)->get_value(),
                    opTree->get_pos_begin()
                },
                expr
            );
        }
        else
        {
            if (!gen_expr_op_tree)
                throw QinpError::from_node("[*Program::expr_parse_helper_unary_op*]: Handler for node-operators not provided!", opTree);

            expr = gen_expr_op_tree(qrawlr::expect_node(opTree), expr);
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
    exprPrimary = parse_appr_expr_prec(qrawlr::expect_node(children[move_id(id_curr)]));

    while (id_curr != id_end)
    {
        Expression<> exprSecondary;
        auto opTree = qrawlr::expect_child(children[move_id(id_curr)], "0");
        exprSecondary = parse_appr_expr_prec(qrawlr::expect_node(children[move_id(id_curr)]));

        // TODO: Future: Check if overload exists (in EVERY parse method*)

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
                    qrawlr::expect_leaf(opTree)->get_value(),
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
                qrawlr::expect_node(opTree),
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

Expression<ExpressionSymbol> Program::make_ExprString(const std::string& value, const qrawlr::Position& position)
{
    auto name = Symbol<>::gen_unique_name("s");

    auto symStr = m_sym_strings.add_child(
        Symbol<SymbolString>::make(
            name, value, position
        )
    );

    return Expression<ExpressionSymbol>::make(symStr, position);
}

TranslationUnitRef Program::push_tu(const std::string& path)
{
    return push_tu(std::make_shared<TranslationUnit>(path, m_sym_root));
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