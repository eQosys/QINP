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

typedef std::shared_ptr<class Program> ProgramRef;

class Program
{
protected:
    Program() = delete;
    Program(Architecture arch, Platform platform, bool verbose);
public:
    int get_ptr_size() const;
    int get_builtin_type_size(const std::string& type_name) const;
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
    void handle_tree_node_func_ret_type(qrawlr::ParseTreeNodeRef node, void* pReturnTypeOut);
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
private:
    QinpError make_node_exception(const std::string& message, qrawlr::ParseTreeRef elem);
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