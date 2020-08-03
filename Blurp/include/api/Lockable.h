#pragma once
#include <mutex>
#include <vector>

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

        /*
         * Get a vector of pointers to all lockables that need to be recursively locked.
         */
        virtual std::vector<Lockable*> GetRecursiveLockables() = 0;

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
         * Returns a vector with pointers to all internal resources that need to be locked.
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
