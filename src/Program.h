#pragma once

#include <set>
#include <vector>
#include <stack>
#include <string>
#include <filesystem>

#include "libQrawlr.h"
#include "TranslationUnit.h"

class Program
{
public:
    Program() = delete;
    Program(bool verbose);
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
    void handle_tree_node_import_specifiers(qrawlr::ParseTreeNodeRef node, void* pFlags);
    void handle_tree_node_literal_string(qrawlr::ParseTreeNodeRef node, void* pString);
    void handle_tree_node_comment(qrawlr::ParseTreeNodeRef node, void* pUnused);
private:
    qrawlr::GrammarException make_grammar_exception(const std::string& message, qrawlr::ParseTreeRef elem);
    qrawlr::GrammarException make_grammar_exception(const std::string& message, qrawlr::ParseTreeRef elem, const std::string& path);
private:
    bool m_verbose;
    SymbolRef m_root_sym;
    qrawlr::Grammar m_grammar;
    std::vector<std::filesystem::path> m_import_dirs;
    std::set<std::string> m_imported_files;
private:
    std::stack<TranslationUnit> m_translation_units;
    TranslationUnit& push_tu(const std::string& path);
    TranslationUnit& curr_tu();
    void pop_tu();
private:
    typedef void (Program::*Handler)(qrawlr::ParseTreeNodeRef, void*);
};