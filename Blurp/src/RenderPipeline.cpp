#pragma once

#include "RenderPipeline.h"

#include <iostream>

#include "RenderPass.h"
#include "BlurpEngine.h"
#include "RenderResourceManager.h"
#include <unordered_set>
#include "Settings.h"
#include "Lockable.h"

namespace blurp
{
    std::shared_ptr<RenderPass> RenderPipeline::AppendRenderPass(RenderPassType a_Type)
    {
        //Create and emplace in the vector.
        std::shared_ptr<RenderPass> ptr = m_Engine.GetResourceManager().CreateRenderPass(a_Type, *this);
        m_RenderPasses.emplace_back(ptr);
        return ptr;
    }

    void RenderPipeline::Execute()
    {
        //Before executing, let the child class set up some stuff.
        PreExecute();

        //Tell each pass to execute.
        for(auto& pass : m_RenderPasses)
        {
            if(pass->IsEnabled())
            {
                assert(pass->IsStateValid() && "Cannot execute render pass with invalid state!");
                pass->Execute();
            }
        }

        //Before finishing, let the child class clean up and possibly send GPU work.
        PostExecute();

        //Finally, if configured stall the CPU and then free resources once the GPU is done.
        if(m_Settings.waitForGpu)
        {
            while(!HasFinishedExecuting())
            {
                //Nothing here just wait.   
            }
        }
    }

    void RenderPipeline::Reset()
    {
        //Tell every render pass to reset their logic and state.
        for (auto& pass : m_RenderPasses)
        {
            pass->Reset();
        }
    }
}
