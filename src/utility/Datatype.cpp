#include "Datatype.h"

#include <stdexcept>

#include "Program.h"
#include "StringHelpers.h"

_Datatype::_Datatype(bool _is_const)
    : m_is_const(_is_const)
{}

void _Datatype::make_const()
{
    m_is_const = true;
}

bool _Datatype::is_const() const
{
    return m_is_const;
}

std::string _Datatype::get_symbol_name() const
{
    return (is_const() ? "c" : "") + __get_symbol_name();
}

Datatype<> _Datatype::get_dereferenced() const
{
    return nullptr;
}

_Datatype_Parent::_Datatype_Parent(Datatype<> child, bool _is_const)
    : _Datatype(_is_const), m_child(child)
{}

Datatype<> _Datatype_Parent::get_dereferenced() const
{
    return m_child;
}

std::string _Datatype_Parent::__get_symbol_name() const
{
    return __get_parent_symbol_name() + m_child->get_symbol_name();
}

_Datatype_Named::_Datatype_Named(const std::string& name, bool _is_const)
    : _Datatype(_is_const), m_name(name)
{}

const std::string& _Datatype_Named::get_name() const
{
    return m_name;
}

Datatype<>::Type _Datatype_Named::get_type() const
{
    return Datatype<>::Type::Name;
}

int _Datatype_Named::get_size() const
{
    auto program = Program::get();
    int size = program->get_builtin_type_size(m_name);
    if (size > 0)
        return size;

    // TODO: Implementation
    throw std::runtime_error("Datatype_Named::get_size() for non-builtin types not implemented yet!");
    return -1;
}

std::string _Datatype_Named::__get_symbol_name() const
{
    return "n<" + get_name() + ">";
}

Datatype<>::Type _Datatype_Macro::get_type() const
{
    return Datatype<>::Type::Macro;
}

int _Datatype_Macro::get_size() const
{
    throw std::runtime_error("Datatype_Macro::get_size() not implemented yet!");
    return -1;
}

std::string _Datatype_Macro::__get_symbol_name() const
{
    return "m<" + get_name() + ">";
}

_Datatype_Function::_Datatype_Function(Datatype<> return_type, const std::vector<Datatype<>>& parameter_types, bool _is_const)
    : _Datatype(_is_const), m_return_type(return_type), m_parameter_types(parameter_types)
{}

Datatype<> _Datatype_Function::get_return_type() const
{
    return m_return_type;
}

const std::vector<Datatype<>>& _Datatype_Function::get_parameter_types() const
{
    return m_parameter_types;
}

Datatype<>::Type _Datatype_Function::get_type() const
{
    return Datatype<>::Type::Function;
}

int _Datatype_Function::get_size() const
{
    return Program::get()->get_ptr_size();
}

std::string _Datatype_Function::__get_symbol_name() const
{
    std::vector<std::string> params;
    for (auto& param : m_parameter_types)
        params.push_back(param->get_symbol_name());

    return "f<" + join(params.begin(), params.end(), ",") + ">";
}

_Datatype_Array::_Datatype_Array(int num_elements, Datatype<> child, bool _is_const)
    : _Datatype_Parent(child, _is_const), m_num_elements(num_elements)
{}

int _Datatype_Array::get_num_elements() const
{
    return m_num_elements;
}

Datatype<>::Type _Datatype_Array::get_type() const
{
    return Datatype<>::Type::Array;
}

int _Datatype_Array::get_size() const
{
    return get_dereferenced()->get_size() * get_num_elements();
}

std::string _Datatype_Array::__get_parent_symbol_name() const
{
    return "a<" + std::to_string(m_num_elements) + ">";
}

_Datatype_Pointer::_Datatype_Pointer(Datatype<> child, bool _is_const)
    : _Datatype_Parent(child, _is_const)
{}

Datatype<>::Type _Datatype_Pointer::get_type() const
{
    return Datatype<>::Type::Pointer;
}

int _Datatype_Pointer::get_size() const
{
    return Program::get()->get_ptr_size();
}

std::string _Datatype_Pointer::__get_parent_symbol_name() const
{
    return "p";
}

_Datatype_Reference::_Datatype_Reference(Datatype<> child, bool _is_const)
    : _Datatype_Parent(child, _is_const)
{}

Datatype<>::Type _Datatype_Reference::get_type() const
{
    return Datatype<>::Type::Reference;
}

int _Datatype_Reference::get_size() const
{
    return Program::get()->get_ptr_size();
}

std::string _Datatype_Reference::__get_parent_symbol_name() const
{
    return "r";
}

_Datatype_Variadic::_Datatype_Variadic()
    : _Datatype(false)
{}

Datatype<>::Type _Datatype_Variadic::get_type() const
{
    return Datatype<>::Type::Variadic;
}

int _Datatype_Variadic::get_size() const
{
    throw std::runtime_error("Cannot get_size() from variadic datatype!");
}

std::string _Datatype_Variadic::__get_symbol_name() const
{
    return "v";
}