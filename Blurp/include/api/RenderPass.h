#pragma once
#include "LockType.h"
#include "RenderResource.h"

namespace blurp
{
    class RenderPipeline;
    class Lockable;

    class RenderPass : public RenderResource
    {
        friend class RenderPipeline;
    public:
        virtual ~RenderPass() = default;

        RenderPass(RenderPipeline& a_Pipeline) : m_Pipeline(a_Pipeline), m_Enabled(true) {}

        //Don't allow copy.
        RenderPass(RenderPass&) = delete;
        RenderPass& operator=(RenderPass&) = delete;

        /*
         * Enable or disable this RenderPass.
         */
        void SetEnabled(bool a_Enabled);

        /*
         * Returns true if this RenderPass is enabled. False otherwise.
         */
        bool IsEnabled() const;

        /*
         * Get the type of this render pass.
         */
        virtual RenderPassType GetType() = 0;

        /*
         * Reset the state of this renderpass.
         * This will clean out cached and queued data.
         * Data that does not differ between frames will not be reset.
         */
        virtual void Reset() = 0;

    protected:
        /*
         * Checks if the state inside this render target is valid.
         * Returns true if ready for drawing, false otherwise.
         */
        virtual bool IsStateValid() = 0;

        /*
         * Execute this RenderPass with the currently bound settings.
         */
        virtual void Execute() = 0;

    private:
        
        RenderPipeline& m_Pipeline;
        bool m_Enabled;
    };
}
