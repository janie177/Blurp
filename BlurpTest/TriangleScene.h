#pragma once

#include "RenderPass_Clear.h"
#include "Transform.h"
#include "Scene.h"
#include "RenderPass_Forward.h"
#include <RenderPass_HelloTriangle.h>

class TriangleScene : public Scene
{
public:


    TriangleScene(blurp::BlurpEngine& a_Engine, const std::shared_ptr<blurp::Window>& a_Window)
        : Scene(a_Engine, a_Window)
    {
    }

    void Init() override;
    void Update() override;

private:
    std::shared_ptr<blurp::RenderPipeline> m_Pipeline;
    std::shared_ptr<blurp::RenderPass_HelloTriangle> m_TrianglePass;
    std::shared_ptr<blurp::RenderPass_Clear> m_ClearPass;

    std::shared_ptr<blurp::Camera> m_Camera;
    std::shared_ptr<blurp::GpuBuffer> m_GpuBuffer;
};