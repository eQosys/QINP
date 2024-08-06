#pragma once

#include <vector>

#include "expressions/Expression.h"

struct CodeBlock
{
    std::vector<Expression<>> statements;
};