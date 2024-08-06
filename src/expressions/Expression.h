#pragma once

template <class ExprType = class _Expression>
class Expression : public std::shared_ptr<ExprType>
{
public:
    using std::shared_ptr<ExprType>::shared_ptr;
};

class _Expression
{
    ;
};