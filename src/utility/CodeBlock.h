#pragma once

#include <vector>

#include "Statement.h"

struct CodeBlock
{
    std::vector<StatementRef> statements;
};