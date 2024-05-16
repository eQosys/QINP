#pragma once

#include <memory>

class Symbol
{
public:
    Symbol() = default;
public:
    virtual ~Symbol() = default;
public:
    template <typename T> bool is_of_type() const;
    template <typename T> T& as_type();
private:
    ;
};

typedef std::shared_ptr<Symbol> SymbolRef;

template <typename T>
bool Symbol::is_of_type() const
{
    return dynamic_cast<const T*>(this) != nullptr;
}

template <typename T>
T& Symbol::as_type()
{
    return dynamic_cast<T&>(*this);
}