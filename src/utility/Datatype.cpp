#include "Datatype.h"

#include <stdexcept>

#include "Program.h"

_Datatype::_Datatype(bool _is_const)
    : m_is_const(_is_const)
{}

bool _Datatype::is_const() const
{
    return m_is_const;
}

Datatype _Datatype::get_dereferenced() const
{
    return nullptr;
}

_Datatype_Parent::_Datatype_Parent(Datatype child, bool _is_const)
    : _Datatype(_is_const), m_child(child)
{}

Datatype _Datatype_Parent::get_dereferenced() const
{
    return m_child;
}

Datatype_Named::Datatype_Named(const std::string& name, bool _is_const)
    : _Datatype(_is_const), m_name(name)
{}

const std::string& Datatype_Named::get_name() const
{
    return m_name;
}

Datatype::Type Datatype_Named::get_type() const
{
    return Datatype::Type::Name;
}

int Datatype_Named::get_size() const
{
    auto program = Program::get();
    int size = program->get_builtin_type_size(m_name);
    if (size > 0)
        return size;

    // TODO: Implementation
    throw std::logic_error("Datatype_Named::get_size() for non-builtin types not implemented yet!");
    return -1;
}

Datatype_Function::Datatype_Function(Datatype return_type, const std::vector<Datatype>& parameter_types, bool _is_const)
    : _Datatype(_is_const), m_return_type(return_type), m_parameter_types(parameter_types)
{}

Datatype Datatype_Function::get_return_type() const
{
    return m_return_type;
}

const std::vector<Datatype>& Datatype_Function::get_parameter_types() const
{
    return m_parameter_types;
}

Datatype::Type Datatype_Function::get_type() const
{
    return Datatype::Type::Function;
}

int Datatype_Function::get_size() const
{
    return Program::get()->get_ptr_size();
}

Datatype_Array::Datatype_Array(int num_elements, Datatype child, bool _is_const)
    : _Datatype_Parent(child, _is_const), m_num_elements(num_elements)
{}

int Datatype_Array::get_num_elements() const
{
    return m_num_elements;
}

Datatype::Type Datatype_Array::get_type() const
{
    return Datatype::Type::Array;
}

int Datatype_Array::get_size() const
{
    return get_dereferenced()->get_size() * get_num_elements();
}

Datatype_Pointer::Datatype_Pointer(Datatype child, bool _is_const)
    : _Datatype_Parent(child, _is_const)
{}

Datatype::Type Datatype_Pointer::get_type() const
{
    return Datatype::Type::Pointer;
}

int Datatype_Pointer::get_size() const
{
    return Program::get()->get_ptr_size();
}

Datatype_Reference::Datatype_Reference(Datatype child, bool _is_const)
    : _Datatype_Parent(child, _is_const)
{}

Datatype::Type Datatype_Reference::get_type() const
{
    return Datatype::Type::Reference;
}

int Datatype_Reference::get_size() const
{
    return Program::get()->get_ptr_size();
}