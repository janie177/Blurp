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
#ifndef NDEBUG
        auto pipelineStart = std::chrono::high_resolution_clock::now();
#endif

        //Before executing, let the child class set up some stuff.
        PreExecute();

#ifndef NDEBUG
        int passCounter = 0;
#endif

        //Tell each pass to execute.
        for(auto& pass : m_RenderPasses)
        {
            if(pass->IsEnabled())
            {
                assert(pass->IsStateValid() && "Cannot execute render pass with invalid state!");

                //Time tracking.
#ifndef NDEBUG
                /*auto start = std::chrono::high_resolution_clock::now();*/
#endif

                pass->Execute();

#ifndef NDEBUG
                //auto end = std::chrono::high_resolution_clock::now();
                //auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                //std::cout << "Render pass #" << passCounter << " execution time: " << duration.count() <<  " microseconds." << std::endl;
                //++passCounter;
#endif
            }
        }

        //Before finishing, let the child class clean up and possibly send GPU work.
        PostExecute();

#ifndef NDEBUG
        auto halfway = std::chrono::high_resolution_clock::now();
#endif

        //Finally, if configured stall the CPU and then free resources once the GPU is done.
        if(m_Settings.waitForGpu)
        {
            while(!HasFinishedExecuting())
            {
                //Nothing here just wait.   
            }
        }

#ifndef NDEBUG
        auto pipelineEnd = std::chrono::high_resolution_clock::now();
        auto half = std::chrono::duration_cast<std::chrono::microseconds>(halfway - pipelineStart);
        auto wait = std::chrono::duration_cast<std::chrono::microseconds>(pipelineEnd - halfway);
        auto full = half.count() + wait.count();
        std::cout << "Finished executing pipeline.\n Total time to execute all passes: " << half.count() << " micros.\nGPU wait time: " << wait.count() << " micros.\nTotal time: " << full << " micros." << std::endl;;;
#endif
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
