#include "Lockable.h"
#include <cassert>

namespace blurp
{
    Lockable::Lockable() : m_ReadLockCount(0), m_WriteLock(false)
    {

    }

    bool Lockable::IsReadLocked(LockType a_LockType) const
    {
        return m_ReadLockCount;
    }

    bool Lockable::IsWriteLocked(LockType a_LockType) const
    {
        return m_WriteLock;
    }

    bool Lockable::IsLocked() const
    {
        return m_WriteLock || m_ReadLockCount != 0;
    }

    void Lockable::Lock(LockType a_LockType)
    {
        std::lock_guard<std::mutex> guard(m_Mutex);
        if (a_LockType == LockType::WRITE)
        {
            if (IsLocked())
            {
                throw std::exception("Lockable already locked! Unlock before trying to write lock.");
            }

            //Set the lock and this is always the first time locking so call OnLock.
            m_WriteLock = true;
            OnLock();
        }
        else
        {
            if(m_WriteLock)
            {
                throw std::exception("Trying to lock a resource for reading when it was already write locked!");
            }

            //Increment lock count and call OnLock if this is the first lock.
            ++m_ReadLockCount;
            if(m_ReadLockCount == 1)
            {
                OnLock();
            }
        }
    }

    void Lockable::Unlock(LockType a_LockType)
    {
        std::lock_guard<std::mutex> guard(m_Mutex);
        if(a_LockType == LockType::WRITE)
        {
            assert(m_WriteLock && "Trying to unlock resource for writing, but it wasn't locked!");
            m_WriteLock = false;

            //Only call when no longer locked at all.
            if(!IsLocked())
            {
                OnUnlock();
            }
        }
        else
        {
            assert(m_ReadLockCount != 0 && "Trying to unlock resource for reading, but it was not locked!");
            --m_ReadLockCount;

            //Only call when no longer locked at all.
            if (!IsLocked())
            {
                OnUnlock();
            }
        }
    }
}
