#pragma once
#include "LockType.h"

namespace blurp
{
    class Lockable;

    /*
     * ResourceLock ensures a Lockable object remains locked.
     * It utilizes the RAII principle to ensure unlocking happens.
     */
    class ResourceLock
    {
    public:
        //Attain lock.
        ResourceLock(Lockable& a_Lockable, LockType a_Type);

        //Release lock.
        ~ResourceLock();

        //Default move
        ResourceLock(ResourceLock&&) noexcept = default;
        ResourceLock& operator=(ResourceLock&&) noexcept = default;

        //Deleted functionality.
        ResourceLock(const ResourceLock&) = delete;
        ResourceLock& operator=(const ResourceLock&) = delete;

        /*
         * Manually release the lock.
         */
        void Release();

    private:
        Lockable* m_Lockable;
        bool m_Locked;
        LockType m_Type;
    };
}