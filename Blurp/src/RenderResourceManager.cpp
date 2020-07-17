#include "RenderResourceManager.h"
#include "RenderDevice.h"
#include "RenderResource.h"

#include "Material.h"
#include "Light.h"
#include "Mesh.h"
#include "RenderTarget.h"
#include "Texture.h"
#include "Camera.h"
#include "RenderPass.h"
#include "RenderPipeline.h"
#include "SwapChain.h"
#include "Shader.h"
#include "GpuBuffer.h"

namespace blurp
{
    RenderResourceManager::RenderResourceManager(BlurpEngine& a_Engine, RenderDevice& a_Device) : m_RenderDevice(a_Device), m_Engine(a_Engine)
    {
        //TODO maybe use something other than a vector for this.
        m_Resources.reserve(10000);
    }

    RenderResourceManager::~RenderResourceManager()
    {
        //Dummy for forward declaration in unique ptr.
    }

    void RenderResourceManager::CleanUpUnused()
    {
        auto itr = m_Resources.begin();
        while (itr != m_Resources.end())
        {
            if (itr->use_count() <= 1)
            {
                //Destroy the resource.
                auto& resource = *itr;
                resource->Destroy(m_Engine);

                itr = m_Resources.erase(itr);
                return;
            }
            ++itr;
        }
    }

    std::shared_ptr<Light> RenderResourceManager::CreateLight(const LightSettings& a_Settings)
    {
        auto ptr = m_RenderDevice.CreateLight(a_Settings);
        m_Resources.emplace_back(ptr);
        ptr->Load(m_Engine);
        return ptr;
    }

    std::shared_ptr<Camera> RenderResourceManager::CreateCamera(const CameraSettings& a_Settings)
    {
        auto ptr = m_RenderDevice.CreateCamera(a_Settings);
        m_Resources.emplace_back(ptr);
        ptr->Load(m_Engine);
        return ptr;
    }

    std::shared_ptr<Mesh> RenderResourceManager::CreateMesh(const MeshSettings& a_Settings)
    {
        auto ptr = m_RenderDevice.CreateMesh(a_Settings);
        m_Resources.emplace_back(ptr);
        ptr->Load(m_Engine);
        return ptr;
    }

    std::shared_ptr<Texture> RenderResourceManager::CreateTexture(const TextureSettings& a_Settings)
    {
        auto ptr = m_RenderDevice.CreateTexture(a_Settings);
        m_Resources.emplace_back(ptr);
        ptr->Load(m_Engine);
        return ptr;
    }

    std::shared_ptr<RenderTarget> RenderResourceManager::CreateRenderTarget(const RenderTargetSettings& a_Settings)
    {
        auto ptr = m_RenderDevice.CreateRenderTarget(a_Settings);
        m_Resources.emplace_back(ptr);
        ptr->Load(m_Engine);
        return ptr;
    }

    std::shared_ptr<SwapChain> RenderResourceManager::CreateSwapChain(const WindowSettings& a_Settings)
    {
        auto ptr = m_RenderDevice.CreateSwapChain(a_Settings);
        m_Resources.emplace_back(ptr);
        ptr->Load(m_Engine);
        return ptr;
    }

    std::shared_ptr<Material> RenderResourceManager::CreateMaterial(const MaterialSettings& a_Settings)
    {
        auto ptr = m_RenderDevice.CreateMaterial(a_Settings);
        m_Resources.emplace_back(ptr);
        ptr->Load(m_Engine);
        return ptr;
    }

    std::shared_ptr<RenderPass> RenderResourceManager::CreateRenderPass(RenderPassType& a_Type, RenderPipeline& a_Pipeline)
    {
        auto ptr = m_RenderDevice.CreateRenderPass(a_Type, a_Pipeline);
        m_Resources.emplace_back(ptr);
        ptr->Load(m_Engine);
        return ptr;
    }

    std::shared_ptr<RenderPipeline> RenderResourceManager::CreatePipeline(const PipelineSettings& a_Settings)
    {
        auto ptr = m_RenderDevice.CreatePipeline(a_Settings);
        m_Resources.emplace_back(ptr);
        ptr->Load(m_Engine);
        return ptr;
    }

    std::shared_ptr<Shader> RenderResourceManager::CreateShader(const ShaderSettings& a_Settings)
    {
        auto ptr = m_RenderDevice.CreateShader(a_Settings);
        m_Resources.emplace_back(ptr);
        ptr->Load(m_Engine);
        return ptr;
    }

    std::shared_ptr<GpuBuffer> RenderResourceManager::CreateGpuBuffer(const GpuBufferSettings& a_Settings)
    {
        auto ptr = m_RenderDevice.CreateGpuBuffer(a_Settings);
        m_Resources.emplace_back(ptr);
        ptr->Load(m_Engine);
        return ptr;
    }
}
