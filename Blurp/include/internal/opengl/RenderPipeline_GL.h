#pragma once
#include "RenderPipeline.h"

namespace blurp
{
	class RenderPipeline_GL : public RenderPipeline
	{
	public:
        RenderPipeline_GL(BlurpEngine& a_Engine, RenderDevice& a_Device) : RenderPipeline(a_Engine, a_Device) {}

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