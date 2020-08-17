#include "ResourceLock.h"
#include "Lockable.h"

#include <iostream>

namespace blurp
{
    ResourceLock::ResourceLock(Lockable& a_Lockable, LockType a_Type) : m_Lockable(&a_Lockable), m_Locked(true), m_Type(a_Type)
    {
        m_Lockable->Lock(a_Type);
    }

    ResourceLock::ResourceLock(ResourceLock&& a_Other) noexcept
    {
        if(&a_Other != this)
        {
            m_Type = a_Other.m_Type;
            m_Lockable = a_Other.m_Lockable;
            m_Locked = a_Other.m_Locked;

            a_Other.m_Lockable = nullptr;
            a_Other.m_Locked = false;
        }
    }

    ResourceLock& ResourceLock::operator=(ResourceLock&& a_Other) noexcept
    {
        if(&a_Other != this)
        {
            m_Type = a_Other.m_Type;
            m_Lockable = a_Other.m_Lockable;
            m_Locked = a_Other.m_Locked;

            a_Other.m_Lockable = nullptr;
            a_Other.m_Locked = false;
        }

        return *this;
    }

    ResourceLock::~ResourceLock()
    {
        if(m_Locked)
        {
            m_Lockable->Unlock(m_Type);
        }
    }

    void ResourceLock::Release()
    {
        m_Locked = false;
        m_Lockable->Unlock(m_Type);
    }
}
