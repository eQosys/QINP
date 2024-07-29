#pragma once

#include <string>
#include <memory>
#include <vector>
#include <utility>

class Datatype : public std::shared_ptr<class _Datatype>
{
public:
    enum class Type
    {
        Name,
        Function,
        Array, Pointer,
        Reference
    };
public:
    using std::shared_ptr<class _Datatype>::shared_ptr;
public:
    template <class Dt, typename ...Args>
    static Datatype make(Args&&... args);
};

template <class Dt, typename ...Args>
Datatype Datatype::make(Args&&... args)
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
private:
    bool m_is_const;
};

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

class Datatype_Named : public _Datatype
{
public:
    Datatype_Named() = delete;
    Datatype_Named(const std::string& name, bool _is_const);
public:
    const std::string& get_name() const;
public:
    virtual Datatype::Type get_type() const override;
    virtual int get_size() const override;
private:
    std::string m_name;
};

class Datatype_Function : public _Datatype
{
public:
    Datatype_Function() = delete;
    Datatype_Function(Datatype return_type, const std::vector<Datatype>& parameter_types, bool _is_const);
public:
    Datatype get_return_type() const;
    const std::vector<Datatype>& get_parameter_types() const;
public:
    virtual Datatype::Type get_type() const override;
    virtual int get_size() const override;
private:
    Datatype m_return_type;
    std::vector<Datatype> m_parameter_types;
};

class Datatype_Array : public _Datatype_Parent
{
public:
    Datatype_Array() = delete;
    Datatype_Array(int num_elements, Datatype child, bool _is_const);
public:
    int get_num_elements() const;
public:
    virtual Datatype::Type get_type() const override;
    virtual int get_size() const override;
private:
    int m_num_elements;
};

class Datatype_Pointer : public _Datatype_Parent
{
public:
    Datatype_Pointer() = delete;
    Datatype_Pointer(Datatype child, bool _is_const);
public:
    virtual Datatype::Type get_type() const override;
    virtual int get_size() const override;
};

class Datatype_Reference : public _Datatype_Parent
{
public:
    Datatype_Reference() = delete;
    Datatype_Reference(Datatype child, bool _is_const);
public:
    virtual Datatype::Type get_type() const override;
    virtual int get_size() const override;
};