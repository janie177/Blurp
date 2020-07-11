#include "BlurpEngine.h"

#include <RenderDevice.h>
#include "opengl/RenderDevice_GL.h"
#include "Window_Win32.h"
#include "RenderResourceManager.h"


namespace blurp
{
	BlurpEngine::BlurpEngine()
    {

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
			m_RenderDevice = std::make_shared<RenderDevice_GL>(*this);
			break;
		default:
			throw std::exception("Graphics API selected not implemented!");
			return false;
        }

		//Initialize the rendering context. Returns true on success.
        const bool rDeviceInitialized = m_RenderDevice->Init(*this, a_Settings.windowSettings);

		if(!rDeviceInitialized)
		{
			throw std::exception("Could not initialize render device.");
			return false;
		}

		m_ResourceManager = std::make_unique<RenderResourceManager>(*this, *m_RenderDevice);

		return true;
    }

    std::shared_ptr<Window> BlurpEngine::GetWindow() const
    {
		return m_Window;
    }

    RenderResourceManager& BlurpEngine::GetResourceManager() const
    {
		assert(m_ResourceManager && "BlurpEngine was not yet initialized!");
		return *m_ResourceManager;
    }
}

