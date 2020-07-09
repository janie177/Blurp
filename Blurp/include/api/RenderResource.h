#pragma once
#include "Settings.h"

namespace blurp
{
    class RenderDevice;
    class BlurpEngine;

    /*
     * RenderResource is an interface that describes something used for rendering that needs special setup and destruction.
     */
    class RenderResource
    {
        friend class BlurpEngine;
        friend class RenderDevice;

    public:
        virtual ~RenderResource() = default;
        RenderResource() = default;

    protected:

        RenderResource(const RenderResource&) = delete;
        RenderResource(RenderResource&&) = delete;
        RenderResource& operator= (const RenderResource&) = delete;
        RenderResource& operator= (RenderResource&&) = delete;

        /*
         * Do any construction work for this resource that needs to be properly synchronized.
         * a_RenderDevice is null when this is called.
         */
        virtual bool Load(BlurpEngine& a_BlurpEngine) = 0;

        /*
         * Clean up this resource in a properly synchronized way.
         */
        virtual bool Destroy(BlurpEngine& a_BlurpEngine) = 0;
    };
}