#pragma once

#include "Datatype.h"

class ConstExprEval
{
public:
    ConstExprEval(Datatype<> datatype);
public:
    template <typename T>
    const T& interpret_as() const;
private:
    Datatype<> m_datatype;
    std::vector<unsigned char> m_data;
};
template <typename T>
const T& ConstExprEval::interpret_as() const
{
    return *(T*)m_data.data();
}