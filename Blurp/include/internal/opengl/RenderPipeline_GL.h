#pragma once
#include "RenderPipeline.h"
#include "ResourceLock.h"

namespace blurp
{
	class RenderPipeline_GL : public RenderPipeline
	{
	public:
        RenderPipeline_GL(const PipelineSettings& a_Settings, BlurpEngine& a_Engine, RenderDevice& a_Device) : RenderPipeline(a_Settings, a_Engine, a_Device) {}

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;

    public:
        bool HasFinishedExecuting() override;

    protected:
        void PreExecute() override;
        void PostExecute() override;
	};
}