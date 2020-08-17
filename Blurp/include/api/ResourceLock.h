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

        //Move operations.
        ResourceLock(ResourceLock&& a_Other) noexcept;
        ResourceLock& operator=(ResourceLock&& a_Other) noexcept;

        //Release lock.
        ~ResourceLock();

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