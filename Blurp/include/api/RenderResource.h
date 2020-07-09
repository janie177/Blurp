#pragma once

namespace blurp
{
    /*
     * RenderResource is an interface that describes something used for rendering that needs special setup and destruction.
     */
    class RenderResource
    {
        friend class BlurpEngine;

    public:
        virtual ~RenderResource() = default;

    protected:
        /*
         * Do any construction work for this resource that needs to be properly synchronized.
         */
        virtual void Load() = 0;

        /*
         * Clean up this resource in a properly synchronized way.
         */
        virtual void Destroy() = 0;
    };
}