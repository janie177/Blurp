#include "Lockable.h"

namespace blurp
{
    Lockable::Lockable() : m_Locked(false)
    {

    }

    bool Lockable::IsLocked() const
    {
        return m_Locked;
    }

    void Lockable::Lock()
    {
        std::lock_guard<std::mutex> guard(m_Mutex);
        if(m_Locked)
        {
            throw std::exception("Lockable already locked! Unlock before trying to access elsewhere.");
        }
        m_Locked = true;
        OnLock();
    }

    void Lockable::Unlock()
    {
        std::lock_guard<std::mutex> guard(m_Mutex);
        m_Locked = false;
        OnUnlock();
    }
}
