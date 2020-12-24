#pragma once
#include <mutex>
#include "LockType.h"

//TODO re-implement lockable at some point. Maybe revisit the entire design. It's not used currently.

namespace blurp
{
    /*
     * Lockable is an interface that allows an object to be locked using a ResourceLock.
     */
    class Lockable
    {
        friend class ResourceLock;

    public:
        Lockable();
        virtual ~Lockable() = default;

        /*
         * See if this lockable is currently locked for reading.
         */
        bool IsReadLocked() const;

        /*
         * See if this lockable is currently locked for writing.
         */
        bool IsWriteLocked() const;

        /*
         * See if this lockable is currently locked for either reading or writing.
         */
        bool IsLocked() const;

    protected:
        /*
         * Lock this resource.
         */
        void Lock(LockType a_LockType);

        /*
         * Unlock this resource.
         */
        void Unlock(LockType a_LockType);

        /*
         * Called when this resource is locked.
         * Returns a vector with pointers to all internal resources that need to be locked.
         */
        virtual void OnLock() = 0;

        /*
         * Called when this resource is unlocked.
         */
        virtual void OnUnlock() = 0;

    private:
        //Amount of reading threads.
        int m_ReadLockCount;

        //True when currently write locked.
        bool m_WriteLock;
        std::mutex m_Mutex;
    };
}
