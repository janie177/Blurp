#pragma once
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
    blurp::Transform iMTransform;
    glm::mat4 m;
    std::shared_ptr<blurp::RenderPass_Forward> forwardPass;
    int numSpasmCubes;
    std::vector<glm::mat4> transforms;
    std::shared_ptr<blurp::Camera> camera;
    std::shared_ptr<blurp::RenderPipeline> pipeline;
    blurp::ForwardDrawData iData;
    blurp::ForwardDrawData data;
    std::shared_ptr<blurp::GpuBuffer> gpuBuffer;
};