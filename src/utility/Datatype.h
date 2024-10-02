#pragma once

#include <string>
#include <memory>
#include <vector>
#include <utility>

#define MAKE_DT(dt, ...) Datatype<dt>::make(__VA_ARGS__)

#define DT_NAMED(...) MAKE_DT(_Datatype_Named, __VA_ARGS__)
#define DT_MACRO(...) MAKE_DT(_Datatype_Macro, __VA_ARGS__)
#define DT_FUNCTION(...) MAKE_DT(_Datatype_Function, __VA_ARGS__)
#define DT_FUNCTION_NAME() MAKE_DT(_Datatype_FunctionName)
#define DT_ARRAY(...) MAKE_DT(_Datatype_Array, __VA_ARGS__)
#define DT_POINTER(...) MAKE_DT(_Datatype_Pointer, __VA_ARGS__)
#define DT_REFERENCE(...) MAKE_DT(_Datatype_Reference, __VA_ARGS__)
#define DT_VARIADIC(...) MAKE_DT(_Datatype_Variadic, __VA_ARGS__)

template <class Dt = class _Datatype>
class Datatype : public std::shared_ptr<Dt>
{
public:
    enum class Type
    {
        Name, Macro,
        Function, FunctionName,
        Array, Pointer,
        Reference,
        Variadic
    };
public:
    using std::shared_ptr<Dt>::shared_ptr;
public:
    template <class NewDt>
    Datatype<NewDt> as_type();
public:
    template <typename ...Args>
    static Datatype<Dt> make(Args&&... args);
};

template <class Dt>
template <typename ...Args>
Datatype<Dt> Datatype<Dt>::make(Args&&... args)
{
    return Datatype<Dt>(new Dt(std::forward<Args>(args)...));
}

class _Datatype
{
public:
    _Datatype() = delete;
    _Datatype(bool _is_const);
    virtual ~_Datatype() = default;
public:
    void make_const();
public:
    bool is_const() const;
    std::string get_symbol_name() const;
    virtual Datatype<>::Type get_type() const = 0;
    virtual int get_size() const = 0;
    virtual Datatype<> get_dereferenced() const;
protected:
    virtual std::string __get_symbol_name() const = 0;
private:
    bool m_is_const;
};

template <class Dt>
template <class NewDt>
Datatype<NewDt> Datatype<Dt>::as_type()
{
    auto dt = std::dynamic_pointer_cast<NewDt>(*this);
    return *(Datatype<NewDt>*)(&dt);
}

class _Datatype_Parent : public _Datatype
{
public:
    _Datatype_Parent() = delete;
    _Datatype_Parent(Datatype<> child, bool _is_const);
public:
    virtual Datatype<> get_dereferenced() const override;
protected:
    virtual std::string __get_symbol_name() const override final;
    virtual std::string __get_parent_symbol_name() const = 0;
private:
    Datatype<> m_child;
};

class _Datatype_Named : public _Datatype
{
public:
    _Datatype_Named() = delete;
    _Datatype_Named(const std::string& name, bool _is_const);
public:
    const std::string& get_name() const;
public:
    virtual Datatype<>::Type get_type() const override;
    virtual int get_size() const override;
protected:
    virtual std::string __get_symbol_name() const override;
private:
    std::string m_name;
};

class _Datatype_Macro : public _Datatype_Named
{
public:
    using _Datatype_Named::_Datatype_Named;
public:
    virtual Datatype<>::Type get_type() const override;
    virtual int get_size() const override;
protected:
    virtual std::string __get_symbol_name() const override;
};

typedef std::vector<Datatype<>> Parameter_Types;

class _Datatype_Function : public _Datatype
{
public:
    _Datatype_Function() = delete;
    _Datatype_Function(Datatype<> return_type, const Parameter_Types& parameter_types, bool _is_const);
public:
    Datatype<> get_return_type() const;
    const std::vector<Datatype<>>& get_parameter_types() const;
public:
    virtual Datatype<>::Type get_type() const override;
    virtual int get_size() const override;
protected:
    virtual std::string __get_symbol_name() const override;
private:
    Datatype<> m_return_type;
    Parameter_Types m_parameter_types;
};

class _Datatype_FunctionName : public _Datatype
{
public:
    _Datatype_FunctionName();
public:
    virtual Datatype<>::Type get_type() const override;
    virtual int get_size() const override;
protected:
    virtual std::string __get_symbol_name() const override;
};

class _Datatype_Array : public _Datatype_Parent
{
public:
    _Datatype_Array() = delete;
    _Datatype_Array(int num_elements, Datatype<> child, bool _is_const);
public:
    int get_num_elements() const;
public:
    virtual Datatype<>::Type get_type() const override;
    virtual int get_size() const override;
protected:
    virtual std::string __get_parent_symbol_name() const override;
private:
    int m_num_elements;
};

class _Datatype_Pointer : public _Datatype_Parent
{
public:
    _Datatype_Pointer() = delete;
    _Datatype_Pointer(Datatype<> child, bool _is_const);
public:
    virtual Datatype<>::Type get_type() const override;
    virtual int get_size() const override;
protected:
    virtual std::string __get_parent_symbol_name() const override;
};

class _Datatype_Reference : public _Datatype_Parent
{
public:
    _Datatype_Reference() = delete;
    _Datatype_Reference(Datatype<> child, bool _is_const);
public:
    virtual Datatype<>::Type get_type() const override;
    virtual int get_size() const override;
protected:
    virtual std::string __get_parent_symbol_name() const override;
};

class _Datatype_Variadic : public _Datatype
{
public:
    _Datatype_Variadic();
public:
    virtual Datatype<>::Type get_type() const override;
    virtual int get_size() const override;
protected:
    virtual std::string __get_symbol_name() const override;
};