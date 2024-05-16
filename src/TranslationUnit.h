#pragma once

#include "libQrawlr.h"

class TranslationUnit
{
public:
    TranslationUnit(const std::string& path, qrawlr::ParseTreeRef tree)
        : path(path), tree(tree)
    {}
public:
    const std::string& get_path() const { return path; }
    qrawlr::ParseTreeRef get_tree() const { return tree; }
private:
    std::string path;
    qrawlr::ParseTreeRef tree;
};