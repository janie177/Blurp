#include "RenderPass_Forward.h"

namespace blurp
{
    RenderPassType RenderPass_Forward::GetType()
    {
        return RenderPassType::RP_FORWARD;
    }

    void RenderPass_Forward::SetCamera(const std::shared_ptr<Camera>& a_Camera)
    {
        m_Camera = a_Camera;
    }

    void RenderPass_Forward::SetTarget(const std::shared_ptr<RenderTarget>& a_RenderTarget)
    {
        m_Output = a_RenderTarget;
    }

    void RenderPass_Forward::QueueForDraw(InstanceData a_Data)
    {
        m_DrawQueue.emplace_back(a_Data);
    }

    void RenderPass_Forward::AddLight(const std::shared_ptr<Light>& a_Light,
        const std::shared_ptr<Texture>& a_ShadowMap)
    {
        //Nothing TODO
    }

    void RenderPass_Forward::Reset()
    {
        m_DrawQueue.clear();
    }

    bool RenderPass_Forward::IsStateValid()
    {
        //TODO
        return m_Output != nullptr && m_Camera != nullptr;
    }
}
