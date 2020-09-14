#pragma once
#include "Settings.h"

namespace blurp
{
    class RenderDevice;
    class BlurpEngine;

    /*
     * RenderResource is an interface that describes something used for rendering that needs special setup and destruction.
     */
    class RenderResource : public std::enable_shared_from_this<RenderResource>
    {
        friend class RenderResourceManager;
        friend class RenderDevice;

    public:
        virtual ~RenderResource() = default;
        RenderResource() : m_Loaded(false) {}

    protected:

        RenderResource(const RenderResource&) = delete;
        RenderResource(RenderResource&&) = delete;
        RenderResource& operator= (const RenderResource&) = delete;
        RenderResource& operator= (RenderResource&&) = delete;

        /*
         * Load this resource.
         */
        bool Load(BlurpEngine& a_BlurpEngine)
        {
            if(m_Loaded)
            {
                throw std::exception("Trying to load resource that was already loaded!");
                return false;
            }
            m_Loaded = true;
            return OnLoad(a_BlurpEngine);
        }

        /*
         * Destroy this resource.
         */
        bool Destroy(BlurpEngine& a_BlurpEngine)
        {
            if (!m_Loaded)
            {
                throw std::exception("Trying to destroy resource that was never loaded!");
                return false;
            }
            m_Loaded = false;
            return OnDestroy(a_BlurpEngine);
        }

        /*
         * Do any construction work for this resource that needs to be properly synchronized.
         * a_RenderDevice is null when this is called.
         * This method is for internal use, and should not be called. Instead call Load().
         */
        virtual bool OnLoad(BlurpEngine& a_BlurpEngine) = 0;

        /*
         * Clean up this resource in a properly synchronized way.
         * This method is for internal use, and should not be called. Instead call Destroy().
         */
        virtual bool OnDestroy(BlurpEngine& a_BlurpEngine) = 0;

    private:
        bool m_Loaded;
    };
}