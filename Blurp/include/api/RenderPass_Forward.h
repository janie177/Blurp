#pragma once
#include "RenderTarget.h"
#include "Camera.h"
#include "Light.h"
#include "Mesh.h"
#include "RenderPass.h"
#include "Material.h"

namespace blurp
{
    /*
     * Struct containing information for drawing.
     */
    struct MeshData
    {
        MeshData()
        {
            mesh = nullptr;
            material = nullptr;
        }

        MeshData(Mesh* a_Mesh, glm::mat4 a_Transform, Material* a_Material) : mesh(a_Mesh), transform(a_Transform), material(a_Material) {}

        Mesh* mesh;
        glm::mat4 transform;
        Material* material;
    };

    /*
     * Struct containing information for drawing.
     */
    struct LightData
    {
        LightData()
        {
            light = nullptr;
            shadowMap = nullptr;
        }

        LightData(Light* a_Light, Texture* a_ShadowMap) : light(a_Light), shadowMap(a_ShadowMap) {}

        Light* light;
        Texture* shadowMap;
    };

    struct SpotLightData
    {
        glm::vec3 color;
        glm::vec3 position;
        glm::vec3 direction;
        float angle;
        float intensity;
    };

    struct PointLightData
    {
        glm::vec3 color;
        glm::vec3 position;
        float intensity;
    };

    struct DirectionalLightData
    {
        glm::vec3 color;
        glm::vec3 direction;
        float intensity;
    };

    class RenderPass_Forward : public RenderPass
    {
    public:
        RenderPassType GetType() override;

        /*
         * Set the camera to render the scene with.
         */
        void SetCamera(const std::shared_ptr<Camera>& a_Camera);

        /*
         * Set the render target to draw into.
         */
        void SetTarget(const std::shared_ptr<RenderTarget>& a_RenderTarget);

        /*
         * Add a mesh to the drawing queue.
         */
        void QueueForDraw(const std::shared_ptr<Mesh>& a_Mesh, glm::mat4 a_Transform);

        /*
         * Add a light to be taken into consideration when drawing.
         */
        void AddLight(const std::shared_ptr<Light>& a_Light, const std::shared_ptr<Texture>& a_ShadowMap);

    protected:
        void Reset() override;
        bool IsStateValid() override;

    protected:
        std::shared_ptr<Camera> m_Camera;
        std::shared_ptr<RenderTarget> m_Output;

        //Drawqueue sorted by meshes{materials{transforms}}
        std::unordered_map<Mesh*, std::unordered_map<Material*, std::vector<glm::mat4>>> m_DrawQueue;

        //Light data.
        std::vector<PointLightData> m_PointLights;
        std::vector<SpotLightData> m_SpotLights;
        std::vector<DirectionalLightData> m_DirectionalLights;

        //Ambient light.
        glm::vec3 m_AmbientColor;
        float m_AmbientIntensity;
    };
}
