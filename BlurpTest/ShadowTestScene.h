#pragma once
#include "Scene.h"

#include <RenderPipeline.h>
#include <RenderPass_Forward.h>
#include <Mesh.h>
#include <Camera.h>
#include <Material.h>

#include <RenderPass_Skybox.h>

#include <RenderPass_Clear.h>

#include <RenderPass_ShadowMap.h>

class ShadowTestScene : public Scene
{
public:
    ShadowTestScene(blurp::BlurpEngine& a_Engine, const std::shared_ptr<blurp::Window>& a_Window)
        : Scene(a_Engine, a_Window)
    {
    }

    void Init() override;
    void Update() override;

private:
    //Pipeline settings and other resources.
    std::shared_ptr<blurp::RenderPipeline> m_Pipeline;
    std::shared_ptr<blurp::RenderPass_Forward> m_ForwardPass;

    //Shadowmapping
    std::shared_ptr<blurp::RenderPass_ShadowMap> m_ShadowGenerationPass;
    std::shared_ptr<blurp::Texture> m_PosShadowArray;
    std::shared_ptr<blurp::Texture> m_DirShadowArray;

    std::shared_ptr<blurp::RenderPass_Skybox> m_SkyboxPass;
    std::shared_ptr<blurp::RenderPass_Clear> m_ClearPass;
    std::shared_ptr<blurp::Camera> m_Camera;
    std::shared_ptr<blurp::GpuBuffer> m_TransformBuffer;

    //Cube used for the floor and other places.
    std::shared_ptr<blurp::Mesh> m_Cube;
    std::shared_ptr<blurp::Material> m_PlaneMaterial;
    blurp::Transform m_PlaneTransform;
    blurp::DrawData m_PlaneDrawData;

    //Object transforms and draw data.
    std::vector<blurp::Transform> m_Transforms;
    blurp::DrawData m_DrawData;

    //Mesh representing the lights.
    std::shared_ptr<blurp::Mesh> m_LightMesh;
    std::shared_ptr<blurp::Material> m_LightMaterial;
    blurp::Transform m_LightMeshTransform;
    blurp::DrawData m_LightMeshDrawData;

    //Light
    std::vector<std::shared_ptr<blurp::PointLight>> m_PointLights;

    //Moving point light data.
    std::vector<glm::vec3> m_LDirs;
    std::vector<glm::vec3> m_LDirMods;
    std::vector<float> m_LSpeeds;

    std::shared_ptr<blurp::Light> m_AmbientLight;

    //Directional
    std::vector<std::shared_ptr<blurp::DirectionalLight>> m_DirLights;
    std::shared_ptr<blurp::GpuBufferView> m_DirLightMatView;
    std::shared_ptr<blurp::GpuBufferView> m_DirLightDataOffsetView;

    //Skybox
    std::shared_ptr<blurp::Texture> m_SkyBoxTexture;
};
