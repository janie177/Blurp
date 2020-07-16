#pragma once
#include <memory>
#include <vector>

#include "RenderDevice.h"
#include "RenderPass.h"
#include "ResourceLock.h"

namespace blurp
{
    class RenderPass;
    class ResourceLock;

    class RenderPipeline : public RenderResource
    {
    public:
        RenderPipeline(const PipelineSettings& a_Settings, BlurpEngine& a_BlurpEngine, RenderDevice& a_RenderDevice) : m_Settings(a_Settings), m_RenderDevice(a_RenderDevice), m_Engine(a_BlurpEngine) {}

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
         * If settings.waitForGpu is true, this stalls the GPU until drawing is completed.
         * In that scenario this will automatically release all resource locks upon completion.
         */
        void Execute();

        /*
         * Set the state of this entire RenderPipeline in every RenderPass to default.
         * This cleans out caches and queues that are only valid for a single frame.
         * Set state that can last more than a single frame is not touched.
         *
         * This does NOT free resource locks, and this function can be called when HasFinishedExecuting() is still false.
         */
        void Reset();

        /*
         * Returns true when this RenderPipeline has finished doing GPU work.
         */
        virtual bool HasFinishedExecuting() = 0;

        /*
         * Unlock all resources that were locked when Execute() was called.
         * HasFinishedExecuting has to return true before this can be called.
         * After calling this, resources can be used again by other pipelines.
         */
        void ReleaseResourceLocks();

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
        PipelineSettings m_Settings;
        RenderDevice& m_RenderDevice;
        BlurpEngine& m_Engine;

    private:
        std::vector<std::shared_ptr<RenderPass>> m_RenderPasses;

        //Locks that keep all resources locked while executing.
        std::vector<ResourceLock> m_Locks;
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
