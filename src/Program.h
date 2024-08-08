#pragma once

#include <set>
#include <vector>
#include <stack>
#include <queue>
#include <string>
#include <filesystem>

#include "libQrawlr.h"
#include "TranslationUnit.h"
#include "errors/QinpError.h"
#include "utility/Architecture.h"
#include "utility/Platform.h"
#include "expressions/Expression.h"
#include "expressions/ExpressionUnaryOp.h"
#include "expressions/ExpressionBinaryOp.h"
#include "expressions/ExpressionTernaryOp.h"

typedef std::shared_ptr<class Program> ProgramRef;

typedef std::function<Expression<>(qrawlr::ParseTreeRef, Expression<>, Expression<>)> ExprGeneratorBinOp;

enum class EvaluationOrder
{
    Left_to_Right,
    Right_to_Left,
};

class Program
{
protected:
    Program() = delete;
    Program(Architecture arch, Platform platform, bool verbose);
public:
    int get_ptr_size() const;
    int get_builtin_type_size(const std::string& type_name) const;
public:
    std::function<std::string(int)> get_fn_tree_id_to_name() const;
public:
    void add_import_directory(const std::string& path_str);
    void import_source_file(std::string path_str, bool skip_duplicate, bool ignore_import_dirs = false);
    void import_source_code(const std::string& code_str, const std::string& path_str = "<inline>");
private:
    void handle_tree_node(qrawlr::ParseTreeRef tree, const std::string& name, void* pData);
    void handle_tree_node_one_of(qrawlr::ParseTreeRef tree, const std::set<std::string>& names, void* pData);
private:
    void handle_tree_node_code_block(qrawlr::ParseTreeNodeRef node, void* pUnused);
    void handle_tree_node_stmt_import(qrawlr::ParseTreeNodeRef node, void* pUnused);
    void handle_tree_node_stmt_space(qrawlr::ParseTreeNodeRef node, void* pUnused);
    void handle_tree_node_stmt_func_decl_def(qrawlr::ParseTreeNodeRef node, void* pUnused);
    void handle_tree_node_func_header(qrawlr::ParseTreeNodeRef node, void* pFuncSymOut);
    void handle_tree_node_optional_datatype(qrawlr::ParseTreeNodeRef node, void* pDatatypeOut);
    void handle_tree_node_func_params(qrawlr::ParseTreeNodeRef node, void* pParamsOut);
    void handle_tree_node_import_specifiers(qrawlr::ParseTreeNodeRef node, void* pFlagsOut);
    void handle_tree_node_literal_string(qrawlr::ParseTreeNodeRef node, void* pStringOut);
    void handle_tree_node_comment(qrawlr::ParseTreeNodeRef node, void* pUnused);
    void handle_tree_node_datatype(qrawlr::ParseTreeNodeRef node, void* pDatatypeOut);
    void handle_tree_node_identifier(qrawlr::ParseTreeNodeRef node, void* pStringOut);
    void handle_tree_node_symbol_reference(qrawlr::ParseTreeNodeRef node, void* pPathOut);
    void handle_tree_node_stmt_defer(qrawlr::ParseTreeNodeRef node, void* pUnused);
    void handle_tree_node_stmt_enum_decl_def(qrawlr::ParseTreeNodeRef node, void* pUnused);
    void handle_tree_node_enum_header(qrawlr::ParseTreeNodeRef node, void* pEnumSymOut);
    void handle_tree_node_enum_def(qrawlr::ParseTreeNodeRef node, void* pEnumSym);
    void handle_tree_node_enum_member_def(qrawlr::ParseTreeNodeRef node, void* pEnumSym);
    void handle_tree_node_stmt_var_decl_def(qrawlr::ParseTreeNodeRef node, void* pUnused);
    void handle_tree_node_var_declarators(qrawlr::ParseTreeNodeRef node, void* pDeclaratorsOut);
    void handle_tree_node_var_initializer(qrawlr::ParseTreeNodeRef node, void* pInitializerOut);
    void handle_tree_node_expression(qrawlr::ParseTreeNodeRef node, void* pExpressionOut);
    void handle_appr_expr_prec(qrawlr::ParseTreeNodeRef node, void* pExpressionOut);
    void handle_tree_node_expr_prec_1(qrawlr::ParseTreeNodeRef node, void* pExpressionOut);
    void handle_tree_node_expr_prec_2(qrawlr::ParseTreeNodeRef node, void* pExpressionOut);
    void handle_tree_node_expr_prec_3(qrawlr::ParseTreeNodeRef node, void* pExpressionOut);
    void handle_tree_node_expr_prec_4(qrawlr::ParseTreeNodeRef node, void* pExpressionOut);
    void handle_tree_node_expr_prec_5(qrawlr::ParseTreeNodeRef node, void* pExpressionOut);
    void handle_tree_node_expr_prec_6(qrawlr::ParseTreeNodeRef node, void* pExpressionOut);
    void handle_tree_node_expr_prec_7(qrawlr::ParseTreeNodeRef node, void* pExpressionOut);
    void handle_tree_node_expr_prec_8(qrawlr::ParseTreeNodeRef node, void* pExpressionOut);
    void handle_tree_node_expr_prec_9(qrawlr::ParseTreeNodeRef node, void* pExpressionOut);
    void handle_tree_node_expr_prec_10(qrawlr::ParseTreeNodeRef node, void* pExpressionOut);
    void handle_tree_node_expr_prec_11(qrawlr::ParseTreeNodeRef node, void* pExpressionOut);
    void handle_tree_node_expr_prec_12(qrawlr::ParseTreeNodeRef node, void* pExpressionOut);
    void handle_tree_node_expr_prec_13(qrawlr::ParseTreeNodeRef node, void* pExpressionOut);
    void handle_tree_node_expr_prec_14(qrawlr::ParseTreeNodeRef node, void* pExpressionOut);
    void handle_tree_node_expr_prec_15(qrawlr::ParseTreeNodeRef node, void* pExpressionOut);
    Expression<> expr_parse_helper_binary_op(qrawlr::ParseTreeNodeRef superNode, EvaluationOrder evalOrder, ExprGeneratorBinOp generate_expression);
private:
    QinpError make_node_error(const std::string& message, qrawlr::ParseTreeRef elem);
    QinpError make_expr_error(const std::string& message, Expression<> expr);
private:
    Architecture m_architecture;
    Platform m_platform;
    bool m_verbose;
    Symbol<> m_root_sym;
    qrawlr::Grammar m_grammar;
    std::vector<std::filesystem::path> m_import_dirs;
    std::set<std::string> m_imported_files;
    std::map<int, std::string> m_file_tree_ids;
    std::function<std::string(int)> m_f_tree_id_to_name;
private:
    std::stack<TranslationUnitRef> m_translation_units;
    std::queue<TranslationUnitRef> m_deferred_translation_units;
    TranslationUnitRef push_tu(const std::string& path);
    TranslationUnitRef push_tu(TranslationUnitRef tu);
    TranslationUnitRef curr_tu();
    TranslationUnitRef pop_tu();
private:
    typedef void (Program::*Handler)(qrawlr::ParseTreeNodeRef, void*);
public:
    static ProgramRef get();
    static void init(Architecture arch, Platform platform, bool verbose);
private:
    static ProgramRef s_singleton;
};