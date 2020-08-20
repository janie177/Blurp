#pragma once
#include "Scene.h"

#include <RenderPipeline.h>
#include <RenderPass_Forward.h>
#include <Mesh.h>
#include <Camera.h>
#include <Material.h>
#include <RenderPass_Skybox.h>

#include <RenderPass_Clear.h>


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
    std::shared_ptr<blurp::RenderPass_Clear> m_ClearPass;

    std::shared_ptr<blurp::Mesh> m_Mesh;
    std::shared_ptr<blurp::Camera> m_Camera;
    std::shared_ptr<blurp::Material> m_Material;
    std::shared_ptr<blurp::GpuBuffer> m_TransformBuffer;
    std::shared_ptr<blurp::GpuBuffer> m_UvModifierBuffer;

    std::shared_ptr<blurp::PointLight> m_Light;

    //Skybox
    std::shared_ptr<blurp::RenderPass_Skybox> m_SkyboxPass;
    std::shared_ptr<blurp::Texture> m_SkyBoxTexture;

    //Data directly related to drawing the mesh.
    blurp::Transform m_MeshTransform;
    blurp::DrawData m_QueueData;
    blurp::DrawData m_LightQueueData;
};
