#pragma once

#include "RenderPipeline.h"
#include "RenderPass.h"
#include "BlurpEngine.h"
#include "RenderResourceManager.h"
#include "ResourceLock.h"
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
        //Ensure that no resources are locked before executing
        if(!m_Locks.empty())
        {
            throw std::exception("Trying to execute pipeline that still has resources locked from last Execute. Call ReleaseResourceLocks and make sure that HasFinishedExecuting() returns true.");
        }

        //Vector containing all the resource locks from all render passes.
        std::unordered_set<Lockable*> lockedResources;

        //Collect a pointer to all the resources that should be locked.
        //Store them in an unordered_set so that no duplicates are possible.
        for (auto& pass : m_RenderPasses)
        {
            if (pass->IsEnabled())
            {
                auto resources = pass->GetLockableResources();
                for(auto& ptr : resources)
                {
                    lockedResources.insert(ptr);

                    //Add all recursive lockables recursively.
                    std::vector<Lockable*> vec = ptr->GetRecursiveLockables();

                    auto size = static_cast<int>(vec.size());

                    for(int i = 0; i < size; ++i)
                    {
                        //First add the current lockable to the resource lock list.
                        lockedResources.insert(vec[i]);

                        //Get the recursive of the recursive.
                        auto nestedVec = vec[i]->GetRecursiveLockables();

                        //If there is more recursion, add each to the vector. Increment loop size.
                        if(!nestedVec.empty())
                        {
                            vec.insert(vec.end(), nestedVec.begin(), nestedVec.end());
                            size += static_cast<int>(nestedVec.size());
                        }
                    }
                }
            }
        }

        //Lock every resource collected.
        for(auto& ptr : lockedResources)
        {
            m_Locks.emplace_back(*ptr);
        }

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

            //Release all locks.
            ReleaseResourceLocks();
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

    void RenderPipeline::ReleaseResourceLocks()
    {
        assert(HasFinishedExecuting() && "Trying to reset render pipeline when still executing!");
        m_Locks.clear();
    }
}
