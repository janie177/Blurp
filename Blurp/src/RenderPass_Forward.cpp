#include "RenderPass_Forward.h"
#include "Texture.h"

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

    void RenderPass_Forward::SetDrawData(const DrawDataSet& a_DrawDataSet)
    {
        m_DrawDataSet = a_DrawDataSet;
    }

    void RenderPass_Forward::SetLights(const LightData& a_LightData)
    {
        m_LightData = a_LightData;
    }

    void RenderPass_Forward::SetShadowData(const ShadowData& a_ShadowData)
    {
        //Ensure valid data is passed.
        assert(a_ShadowData.positional.shadowMaps == nullptr || a_ShadowData.positional.shadowMaps->GetTextureType() == TextureType::TEXTURE_CUBEMAP_ARRAY && "Shadowmaps for pointlights need to be cubemap arrays!");
        assert(a_ShadowData.directional.shadowMaps == nullptr || (a_ShadowData.directional.shadowMaps->GetTextureType() == TextureType::TEXTURE_2D_ARRAY && a_ShadowData.directional.dataBuffer != nullptr && a_ShadowData.directional.dataRange != nullptr && a_ShadowData.directional.numCascades > 0));

        m_ShadowData = a_ShadowData;
    }

    void RenderPass_Forward::Reset()
    {
        m_DrawDataSet = DrawDataSet();
        m_LightData = LightData();
    }

    bool RenderPass_Forward::IsStateValid()
    {
        return m_Output != nullptr && m_Camera != nullptr;
    }
}
