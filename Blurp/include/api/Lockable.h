#pragma once
#include <mutex>

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
         * See if this lockable is currently locked.
         */
        bool IsLocked() const;

    protected:
        /*
         * Lock this resource.
         */
        void Lock();

        /*
         * Unlock this resource.
         */
        void Unlock();

        /*
         * Called when this resource is locked.
         */
        virtual void OnLock() = 0;

        /*
         * Called when this resource is unlocked.
         */
        virtual void OnUnlock() = 0;

    private:
        bool m_Locked;
        std::mutex m_Mutex;
    };
}
