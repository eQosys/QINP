#pragma once

#include <set>
#include <vector>
#include <string>
#include <filesystem>

#include "libQrawlr.h"

class Program
{
public:
    Program() = delete;
    Program(bool verbose);
public:
    void add_import_directory(const std::string& path_str);
    void import_source_file(std::string path_str, const std::string& import_from_dir_str, bool skip_duplicate, bool ignore_import_dirs = false);
    void import_source_code(const std::string& code_str, const std::string& path_str = "<inline>");
private:
    void handle_tree_node(qrawlr::ParseTreeRef tree, const std::string& name, void* pData);
    void handle_tree_node_one_of(qrawlr::ParseTreeRef tree, const std::set<std::string>& names, void* pData);
private:
    void handle_tree_node_global_code(qrawlr::ParseTreeNodeRef node, void* pData);
    void handle_tree_node_string(qrawlr::ParseTreeNodeRef node, void* pString);
private:
    bool m_verbose;
    std::vector<std::filesystem::path> m_import_dirs;
    std::set<std::string> m_imported_files;
    qrawlr::Grammar m_grammar;
private:
    std::string m_current_translation_unit_path;
private:
    typedef void (Program::*Handler)(qrawlr::ParseTreeNodeRef, void*);
};