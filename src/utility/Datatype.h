#pragma once

#include <string>
#include <memory>
#include <vector>
#include <utility>

#define MAKE_DT(dt, ...) Datatype::make<dt>(__VA_ARGS__)

#define DT_NAMED(...) MAKE_DT(_Datatype_Named, __VA_ARGS__)
#define DT_MACRO(...) MAKE_DT(_Datatype_Macro, __VA_ARGS__)
#define DT_FUNCTION(...) MAKE_DT(_Datatype_Function, __VA_ARGS__)
#define DT_ARRAY(...) MAKE_DT(_Datatype_Array, __VA_ARGS__)
#define DT_POINTER(...) MAKE_DT(_Datatype_Pointer, __VA_ARGS__)
#define DT_REFERENCE(...) MAKE_DT(_Datatype_Reference, __VA_ARGS__)

class Datatype : public std::shared_ptr<class _Datatype>
{
public:
    enum class Type
    {
        Name, Macro,
        Function,
        Array, Pointer,
        Reference
    };
public:
    using std::shared_ptr<class _Datatype>::shared_ptr;
public:
    template <class Dt, typename ...Args>
    static std::shared_ptr<Dt> make(Args&&... args);
};

template <class Dt, typename ...Args>
std::shared_ptr<Dt> Datatype::make(Args&&... args)
{
    return std::make_shared<Dt>(std::forward<Args>(args)...);
}

class _Datatype
{
public:
    _Datatype() = delete;
    _Datatype(bool _is_const);
    virtual ~_Datatype() = default;
public:
    bool is_const() const;
    virtual Datatype::Type get_type() const = 0;
    virtual int get_size() const = 0;
    virtual Datatype get_dereferenced() const;
public:
    template <class Dt>
    Dt& as_type();
private:
    bool m_is_const;
};

template <class Dt>
Dt& _Datatype::as_type()
{
    return dynamic_cast<Dt&>(*this);
}

class _Datatype_Parent : public _Datatype
{
public:
    _Datatype_Parent() = delete;
    _Datatype_Parent(Datatype child, bool _is_const);
public:
    virtual Datatype get_dereferenced() const override;
private:
    Datatype m_child;
};

class _Datatype_Named : public _Datatype
{
public:
    _Datatype_Named() = delete;
    _Datatype_Named(const std::string& name, bool _is_const);
public:
    const std::string& get_name() const;
public:
    virtual Datatype::Type get_type() const override;
    virtual int get_size() const override;
private:
    std::string m_name;
};

class _Datatype_Macro : public _Datatype_Named
{
public:
    using _Datatype_Named::_Datatype_Named;
public:
    virtual Datatype::Type get_type() const override;
    virtual int get_size() const override;
};

typedef std::vector<Datatype> Parameter_Types;

class _Datatype_Function : public _Datatype
{
public:
    _Datatype_Function() = delete;
    _Datatype_Function(Datatype return_type, const Parameter_Types& parameter_types, bool _is_const);
public:
    Datatype get_return_type() const;
    const std::vector<Datatype>& get_parameter_types() const;
public:
    virtual Datatype::Type get_type() const override;
    virtual int get_size() const override;
private:
    Datatype m_return_type;
    Parameter_Types m_parameter_types;
};

class _Datatype_Array : public _Datatype_Parent
{
public:
    _Datatype_Array() = delete;
    _Datatype_Array(int num_elements, Datatype child, bool _is_const);
public:
    int get_num_elements() const;
public:
    virtual Datatype::Type get_type() const override;
    virtual int get_size() const override;
private:
    int m_num_elements;
};

class _Datatype_Pointer : public _Datatype_Parent
{
public:
    _Datatype_Pointer() = delete;
    _Datatype_Pointer(Datatype child, bool _is_const);
public:
    virtual Datatype::Type get_type() const override;
    virtual int get_size() const override;
};

class _Datatype_Reference : public _Datatype_Parent
{
public:
    _Datatype_Reference() = delete;
    _Datatype_Reference(Datatype child, bool _is_const);
public:
    virtual Datatype::Type get_type() const override;
    virtual int get_size() const override;
};