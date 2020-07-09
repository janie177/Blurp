#include "BlurpEngine.h"

#include "Window_Win32.h"

namespace blurp
{
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

			m_Window->Load();
		}

		//TODO make a shared pointer to a RenderDevice here. Then pass the window.get() to it in Init(window*);

		//Initialize the API
        switch (a_Settings.graphicsAPI)
        {
		case GraphicsAPI::OPENGL:
			break;
		default:
			throw std::exception("Graphics API selected not implemented!");
			return false;
        }

		return true;
    }

    std::shared_ptr<Window> BlurpEngine::GetWindow()
    {
		return m_Window;
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
				resource->Destroy();

				itr = m_Resources.erase(itr);
				return;
			}

			++itr;
		}
	}
}

