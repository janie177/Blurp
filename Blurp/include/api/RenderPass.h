#pragma once
#include "RenderResource.h"

namespace blurp
{
    class RenderPipeline;

    class RenderPass : public RenderResource
    {
        friend class RenderPipeline;
    public:
        virtual ~RenderPass() = default;

        RenderPass(RenderPipeline& a_Pipeline) : m_Pipeline(a_Pipeline) {}

        //Don't allow copy.
        RenderPass(RenderPass&) = delete;
        RenderPass& operator=(RenderPass&) = delete;

        /*
         * Get the type of this render pass.
         */
        virtual RenderPassType GetType() = 0;

    protected:
        /*
         * Reset the state of this RenderPass to default.
         */
        virtual void Reset() = 0;

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
    };
}
