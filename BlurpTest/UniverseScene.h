#pragma once
#include "RenderPass_Clear.h"
#include "Transform.h"
#include "Scene.h"
#include "RenderPass_Forward.h"

class UniverseScene : public Scene
{
public:


    UniverseScene(blurp::BlurpEngine& a_Engine, const std::shared_ptr<blurp::Window>& a_Window)
        : Scene(a_Engine, a_Window)
    {
    }

    void Init() override;
    void Update() override;

private:
    blurp::Transform iMTransform[2];
    glm::mat4 m[2];
    std::shared_ptr<blurp::RenderPass_Forward> forwardPass;
    std::shared_ptr<blurp::RenderPass_Clear> m_ClearPass;
    int numSpasmCubes;
    std::vector<glm::mat4> transforms;
    std::shared_ptr<blurp::Camera> camera;
    std::shared_ptr<blurp::RenderPipeline> pipeline;
    blurp::DrawData iData;
    blurp::DrawData data;
    blurp::DrawData sunData;

    std::shared_ptr<blurp::GpuBuffer> gpuBuffer;

    std::shared_ptr<blurp::PointLight> m_Sun;

    blurp::PipelineState pipelineState;
};