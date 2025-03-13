#pragma once

#include "libQrawlr.h"
#include "utility/Datatype.h"

template <class ExprType = class _Expression>
class Expression : public std::shared_ptr<ExprType>
{
public:
    using std::shared_ptr<ExprType>::shared_ptr;
public:
    template <typename... Args>
    static Expression<ExprType> make (Args&&... args);
public:
    template <class NewT>
    bool is_of_type() const;
    template <class NewT>
    Expression<NewT> as_type();
};

class _Expression
{
public:
    _Expression() = default;
    _Expression(Datatype<> datatype, const qrawlr::Position& position);
public:
    virtual ~_Expression() = default;
public:
    Datatype<> get_datatype() const;
    const qrawlr::Position& get_position() const;
public:
    virtual bool is_const_expr() const = 0;
    virtual bool results_in_object() const = 0;
private:
    qrawlr::Position m_position;
protected:
    Datatype<> m_datatype;
};

template <class ExprType>
template <class NewT>
bool Expression<ExprType>::is_of_type() const
{
    return std::dynamic_pointer_cast<NewT>(*this) != nullptr;
}

template <class ExprType>
template <class NewT>
Expression<NewT> Expression<ExprType>::as_type()
{
    auto sym = std::dynamic_pointer_cast<NewT>(*this);
    return *(Expression<NewT>*)(&sym);
}

template <class ExprType>
template <typename... Args>
Expression<ExprType> Expression<ExprType>::make(Args&&... args)
{
    return Expression<ExprType>(new ExprType(std::forward<Args>(args)...));
}