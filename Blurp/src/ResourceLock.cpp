#include "ResourceLock.h"
#include "Lockable.h"

namespace blurp
{
    ResourceLock::ResourceLock(Lockable& a_Lockable, LockType a_Type) : m_Lockable(&a_Lockable), m_Locked(true), m_Type(a_Type)
    {
        m_Lockable->Lock(a_Type);
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
