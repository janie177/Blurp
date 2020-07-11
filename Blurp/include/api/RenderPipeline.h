#pragma once
#include <memory>
#include <vector>

#include "RenderDevice.h"
#include "RenderPass.h"

namespace blurp
{
    class RenderPass;

    class RenderPipeline : public RenderResource
    {
    public:
        RenderPipeline(BlurpEngine& a_BlurpEngine, RenderDevice& a_RenderDevice) : m_RenderDevice(a_RenderDevice), m_Engine(a_BlurpEngine) {}

        //Don't allow copy or move.
        RenderPipeline(RenderPipeline&) = delete;
        RenderPipeline& operator=(RenderPipeline&) = delete;

        /*
         * Create and append a render pass to this pipeline.
         * Returns a shared pointer to the render pass created.
         * Cast to the right type to setup the state for rendering.
         */
        std::shared_ptr<RenderPass> AppendRenderPass(RenderPassType a_Type);

        template<typename T>
        std::shared_ptr<T> AppendRenderPass(RenderPassType a_Type);


        /*
         * Execute this RenderPipeline.
         * This consecutively executes each of the render passes.
         */
        void Execute();

        /*
         * Reset the state of this entire RenderPipeline to default.
         */
        void Reset();

        /*
         * Returns true when this RenderPipeline has finished doing GPU work.
         */
        virtual bool HasFinishedExecuting() = 0;

    protected:
        /*
         * This is called before Execute is called on the render passes in this pipeline.
         */
        virtual void PreExecute() = 0;

        /*
         * This is called after Execute is called on the render passes in this pipeline.
         */
        virtual void PostExecute() = 0;

    protected:
        RenderDevice& m_RenderDevice;
        BlurpEngine& m_Engine;

    private:
        std::vector<std::shared_ptr<RenderPass>> m_RenderPasses;
    };

    template <typename T>
    std::shared_ptr<T> RenderPipeline::AppendRenderPass(RenderPassType a_Type)
    {
        static_assert(std::is_base_of<RenderPass, T>::value, "T is not of type RenderPass!");

        //Create the instance and add it to the registry.
        auto ptr = AppendRenderPass(a_Type);

        //Make sure that T matches the type specified.
        assert(ptr->GetType() == a_Type && "RenderPassType specified does not match RenderPassType of T!");

        return std::reinterpret_pointer_cast<T>(ptr);
    }
}
