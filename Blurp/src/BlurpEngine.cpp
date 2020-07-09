#include "BlurpEngine.h"

#include <RenderDevice.h>
#include "opengl/RenderDevice_GL.h"
#include "Window_Win32.h"

#include "Material.h"
#include "Light.h"
#include "Mesh.h"
#include "RenderTarget.h"
#include "Texture.h"
#include "Camera.h"

namespace blurp
{
	BlurpEngine::BlurpEngine()
    {
		//TODO use something other than a vector maybe.
		m_Resources.reserve(2000);
    }

    bool BlurpEngine::Init(const BlurpSettings& a_Settings)
    {
		//Create the window if specified.
		if(a_Settings.windowSettings.type != WindowType::NONE)
		{
            switch (a_Settings.windowSettings.type)
            {
			case WindowType::WINDOW_WIN32:
				m_Window = std::make_shared<Window_Win32>(a_Settings.windowSettings);
				break;
			default:
				throw std::exception("Window type selected not implemented!");
				return false;
            }

			//Initialize the window.
			m_Window->Load();
		}

		//Create the right render device instance.
        switch (a_Settings.graphicsAPI)
        {
		case GraphicsAPI::OPENGL:
			m_RenderDevice = std::make_shared<RenderDevice_GL>();
			break;
		default:
			throw std::exception("Graphics API selected not implemented!");
			return false;
        }

		//Initialize the rendering context. Returns true on success.
		return m_RenderDevice->Init(*this, a_Settings.windowSettings);
    }

    std::shared_ptr<Window> BlurpEngine::GetWindow() const
    {
		return m_Window;
    }

    std::shared_ptr<Light> BlurpEngine::CreateLight(const LightSettings& a_Settings)
    {
		auto resource = m_RenderDevice->CreateLight(a_Settings);
		resource->Load(*this);
		m_Resources.emplace_back(resource);
		return resource;
    }

    std::shared_ptr<Camera> BlurpEngine::CreateCamera(const CameraSettings& a_Settings)
    {
		auto resource = m_RenderDevice->CreateCamera(a_Settings);
		resource->Load(*this);
		m_Resources.emplace_back(resource);
		return resource;
    }

    std::shared_ptr<Mesh> BlurpEngine::CreateMesh(const MeshSettings& a_Settings)
    {
		auto resource = m_RenderDevice->CreateMesh(a_Settings);
		resource->Load(*this);
		m_Resources.emplace_back(resource);
		return resource;
    }

    std::shared_ptr<Texture> BlurpEngine::CreateTexture(const TextureSettings& a_Settings)
    {
		auto resource = m_RenderDevice->CreateTexture(a_Settings);
		resource->Load(*this);
		m_Resources.emplace_back(resource);
		return resource;
    }

    std::shared_ptr<RenderTarget> BlurpEngine::CreateRenderTarget(const RenderTargetSettings& a_Settings)
    {
		auto resource = m_RenderDevice->CreateRenderTarget(a_Settings);
		resource->Load(*this);
		m_Resources.emplace_back(resource);
		return resource;
    }

    std::shared_ptr<Material> BlurpEngine::CreateMaterial(const MaterialSettings& a_Settings)
    {
		auto resource = m_RenderDevice->CreateMaterial(a_Settings);
		resource->Load(*this);
		m_Resources.emplace_back(resource);
		return resource;

    }

    void BlurpEngine::CleanUp()
	{
		auto itr = m_Resources.begin();
		while (itr != m_Resources.end())
		{
			if (itr->use_count() <= 1)
			{
				//Destroy the resource.
				auto& resource = *itr;
				resource->Destroy(*this);

				itr = m_Resources.erase(itr);
				return;
			}

			++itr;
		}
	}
}

