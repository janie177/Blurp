#pragma once
#include "Scene.h"

#include <RenderPipeline.h>
#include <RenderPass_Forward.h>
#include <Mesh.h>
#include <Camera.h>
#include <Material.h>

class LightTestScene : public Scene
{
public:
    LightTestScene(blurp::BlurpEngine& a_Engine, const std::shared_ptr<blurp::Window>& a_Window)
        : Scene(a_Engine, a_Window)
    {
    }

    void Init() override;
    void Update() override;

private:
    //Pipeline settings and other resources.
    std::shared_ptr<blurp::RenderPipeline> m_Pipeline;
    std::shared_ptr<blurp::RenderPass_Forward> m_ForwardPass;
    std::shared_ptr<blurp::Camera> m_Camera;
    std::shared_ptr<blurp::GpuBuffer> m_TransformBuffer;

    //Plane representing the floor.
    std::shared_ptr<blurp::Mesh> m_Plane;
    std::shared_ptr<blurp::Material> m_PlaneMaterial;
    blurp::Transform m_PlaneTransform;
    blurp::ForwardDrawData m_PlaneDrawData;

    //Vectors containing all the lights.
    std::vector<std::shared_ptr<blurp::PointLight>> m_PointLights;
    std::vector<std::shared_ptr<blurp::SpotLight>> m_SpotLights;
    std::vector<std::shared_ptr<blurp::DirectionalLight>> m_DirectionalLights;
    std::shared_ptr<blurp::Light> m_AmbientLight;

    //Mesh representing the lights.
    std::shared_ptr<blurp::Mesh> m_LightMesh;
    std::shared_ptr<blurp::Material> m_LightMaterial;
    blurp::Transform m_LightMeshTransform;
    blurp::ForwardDrawData m_LightMeshDrawData;

    //Light movement
    std::vector<glm::vec3> m_LDirs;
    std::vector<glm::vec3> m_LDirMods;
    std::vector<float> m_LSpeeds;
};
