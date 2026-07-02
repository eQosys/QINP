#pragma once

#include "utility/Datatype.h"
#include "errors/QinpError.h"

class CEObject
{
public:
    CEObject(Datatype<> dt, const void* pData);
public:
    Datatype<> get_datatype() const;
    const void* get_raw() const;
    template<typename T>
    const T& get_as() const;
private:
    Datatype<> m_datatype;
    std::vector<unsigned char> m_data;
};

template <typename T>
const T& CEObject::get_as() const
{
    if (sizeof(T) > m_data.size())
        throw QinpError(
            std::string("Cannot cast CEObject(")
            + std::to_string(m_data.size())
            + ") to "
            + typeid(T).name()
            + "(" +
            std::to_string(sizeof(T))
            +
            ")"
        );

    return *(const T*)m_data.data();
}