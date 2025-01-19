#pragma once

#include <map>
#include <memory>
#include <string>

#include "libQrawlr.h"
#include "utility/Datatype.h"
#include "utility/SymbolPath.h"

#define SYMBOL_NAME_BLUEPRINTS "$_BPS_$"

enum class DuplicateHandling
{
    Throw,  // Throws an exception when a duplicate symbol is detected
    Keep,   // Keeps the already existing symbol and discards the new symbol
    Replace, // Replaces the existing symbol with the new symbol
};

template <class SymType = class _Symbol>
class Symbol : public std::shared_ptr<SymType>
{
public:
    using std::shared_ptr<SymType>::shared_ptr;
public:
    Symbol<> add_child(Symbol<> child_sym, std::function<std::string(int)> tree_id_to_name, DuplicateHandling dupHandling = DuplicateHandling::Throw);
    Symbol<SymType>& if_null(std::function<void(void)> func);
    const Symbol<SymType>& if_null(std::function<void(void)> func) const;
public:
    template <class NewT>
    bool is_of_type() const;
    template <class NewT>
    Symbol<NewT> as_type();
public:
    template <typename... Args>
    static Symbol<SymType> make(Args&&... args);
};

class _Symbol
{
public:
    _Symbol();
    _Symbol(const std::string& name, const qrawlr::Position& position);
public:
    virtual ~_Symbol() = default;
public:
    Symbol<> get_child_by_name(const std::string& name) const;
public:
    const std::string& get_name() const;
    Symbol<> get_parent() const;
    const qrawlr::Position& get_position() const;
    SymbolPath get_symbol_path() const;
public:
    virtual bool is_object() const = 0;
    virtual Datatype<> get_datatype() const = 0;
private:
    Symbol<> add_child(Symbol<> this_sym, Symbol<> child_sym, std::function<std::string(int)> tree_id_to_name, DuplicateHandling dupHandling);
public:
    std::string to_digraph_str(bool verbose) const;
protected:
    void to_digraph_internal(std::string& graph, bool verbose) const;
    virtual std::string get_digraph_impl_text(bool verbose) const;
    virtual std::string get_symbol_type_str() const;
private:
    size_t m_id;
    std::string m_name;
    qrawlr::Position m_position;
    std::weak_ptr<_Symbol> m_parent;
    std::map<std::string, Symbol<>> m_children;
private:
    static size_t s_next_id;
private:
    template <class SymType>
    friend class Symbol;
};

template <class SymType>
Symbol<> Symbol<SymType>::add_child(Symbol<> child_sym, std::function<std::string(int)> tree_id_to_name, DuplicateHandling dupHandling)
{
    return (*this)->add_child(*this, child_sym, tree_id_to_name, dupHandling);
}

template <class SymType>
Symbol<SymType>& Symbol<SymType>::if_null(std::function<void(void)> func)
{
    if (this->get() == nullptr)
        func();
    
    return *this;
}

template <class SymType>
const Symbol<SymType>& Symbol<SymType>::if_null(std::function<void(void)> func) const
{
    if (this->get() == nullptr)
        func();
    
    return *this;
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