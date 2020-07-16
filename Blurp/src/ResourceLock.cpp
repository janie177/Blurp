#include "ResourceLock.h"
#include "Lockable.h"

namespace blurp
{
    ResourceLock::ResourceLock(Lockable& a_Lockable) : m_Lockable(&a_Lockable), m_Locked(true)
    {
        m_Lockable->Lock();
    }

    ResourceLock::~ResourceLock()
    {
        if(m_Locked)
        {
            m_Lockable->Unlock();
        }
    }

    void ResourceLock::Release()
    {
        m_Locked = false;
        m_Lockable->Unlock();
    }
}
