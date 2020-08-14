#pragma once
#include "Scene.h"

#include <RenderPipeline.h>
#include <RenderPass_Forward.h>
#include <Mesh.h>
#include <Camera.h>
#include <Material.h>


class MaterialTestScene : public Scene
{
public:
    MaterialTestScene(blurp::BlurpEngine& a_Engine, const std::shared_ptr<blurp::Window>& a_Window)
        : Scene(a_Engine, a_Window)
    {
    }

    void Init() override;
    void Update() override;

private:
    //Pipeline settings and other resources.
    std::shared_ptr<blurp::RenderPipeline> m_Pipeline;
    std::shared_ptr<blurp::RenderPass_Forward> m_ForwardPass;
    std::shared_ptr<blurp::Mesh> m_Mesh;
    std::shared_ptr<blurp::Camera> m_Camera;
    std::shared_ptr<blurp::Material> m_Material;
    std::shared_ptr<blurp::GpuBuffer> m_GpuBuffer;

    //Data directly related to drawing the mesh.
    blurp::Transform m_MeshTransform;
    blurp::ForwardDrawData m_QueueData;
};
