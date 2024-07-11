#pragma once

#include <map>
#include <memory>
#include <string>

#include "utility/Location.h"

typedef std::shared_ptr<class Symbol> SymbolRef;

class Symbol
{
public:
    Symbol() = default;
    Symbol(const std::string& name, const Location& location, SymbolRef parent);
public:
    virtual ~Symbol() = default;
public:
    void add_child(SymbolRef sym);
    SymbolRef get_child(const std::string& name) const;
public:
    const std::string& name() const;
    const Location& location() const;
public:
    template <typename T>
    bool is_of_type() const;
    template <typename T>
    T& as_type();
public:
    template <typename T, typename... Args>
    static std::shared_ptr<T> make(Args&&... args);
private:
    std::string m_name;
    Location m_location;
    std::weak_ptr<Symbol> m_parent;
    std::map<std::string, SymbolRef> m_children;
};

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

template <typename T, typename... Args>
std::shared_ptr<T> Symbol::make(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}