#pragma once

#include <map>
#include <memory>
#include <string>

#include "utility/Location.h"

template <class SymType = class _Symbol>
class Symbol : public std::shared_ptr<SymType>
{
public:
    using std::shared_ptr<SymType>::shared_ptr;
public:
    void add_child(Symbol<> sym);
public:
    template <typename... Args>
    static Symbol<SymType> make(Args&&... args);
public:
    template <class NewT>
    bool is_of_type() const;
    template <class NewT>
    Symbol<NewT> as_type();
};

class _Symbol
{
public:
    _Symbol() = default;
    _Symbol(const std::string& name, const Location& location);
public:
    virtual ~_Symbol() = default;
public:
    Symbol<> get_child_by_name(const std::string& name) const;
public:
    const std::string& get_name() const;
    Symbol<> get_parent() const;
    const Location& get_location() const;
private:
    void add_child(Symbol<> sym, Symbol<> this_sym);
private:
    std::string m_name;
    Location m_location;
    std::weak_ptr<_Symbol> m_parent;
    std::map<std::string, Symbol<>> m_children;
private:
    friend class Symbol<>;
};

template <class SymType>
void Symbol<SymType>::add_child(Symbol<> sym)
{
    (*this)->add_child(sym, *this);
}

template <class SymType>
template <class NewT>
bool Symbol<SymType>::is_of_type() const
{
    return std::dynamic_pointer_cast<NewT>(*this) != nullptr;
}

template <class SymType>
template <class NewT>
Symbol<NewT> Symbol<SymType>::as_type()
{
    auto sym = std::dynamic_pointer_cast<NewT>(*this);
    return *(Symbol<NewT>*)(&sym);
}

template <class SymType>
template <typename... Args>
Symbol<SymType> Symbol<SymType>::make(Args&&... args)
{
    return Symbol<SymType>(new SymType(std::forward<Args>(args)...));
}