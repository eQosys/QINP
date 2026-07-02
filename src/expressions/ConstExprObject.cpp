#include "ConstExprObject.h"


CEObject::CEObject(Datatype<> dt, const void* pData)
    : m_datatype(dt),
    m_data(
        (const unsigned char*)pData,
        (const unsigned char*)pData + dt->get_size()
    )
{
}