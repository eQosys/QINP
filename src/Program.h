#pragma once

#include <set>
#include <vector>
#include <stack>
#include <string>
#include <filesystem>

#include "libQrawlr.h"
#include "TranslationUnit.h"
#include "utility/Architecture.h"

typedef std::shared_ptr<class Program> ProgramRef;

class Program
{
protected:
    Program() = delete;
    Program(Architecture arch, bool verbose);
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
    void handle_tree_node_func_header(qrawlr::ParseTreeNodeRef node, void* pSym);
    void handle_tree_node_func_ret_type(qrawlr::ParseTreeNodeRef node, void* pReturn_type);
    void handle_tree_node_import_specifiers(qrawlr::ParseTreeNodeRef node, void* pFlags);
    void handle_tree_node_literal_string(qrawlr::ParseTreeNodeRef node, void* pString);
    void handle_tree_node_comment(qrawlr::ParseTreeNodeRef node, void* pUnused);
private:
    qrawlr::GrammarException make_grammar_exception(const std::string& message, qrawlr::ParseTreeRef elem);
private:
    Architecture m_architecture;
    bool m_verbose;
    SymbolRef m_root_sym;
    qrawlr::Grammar m_grammar;
    std::vector<std::filesystem::path> m_import_dirs;
    std::set<std::string> m_imported_files;
    std::map<int, std::string> m_file_tree_ids;
private:
    std::stack<TranslationUnit> m_translation_units;
    TranslationUnit& push_tu(const std::string& path);
    TranslationUnit& curr_tu();
    void pop_tu();
private:
    typedef void (Program::*Handler)(qrawlr::ParseTreeNodeRef, void*);
public:
    static ProgramRef get();
    static void init(Architecture arch, bool verbose);
private:
    static ProgramRef s_singleton;
};