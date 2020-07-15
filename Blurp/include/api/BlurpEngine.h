#pragma once
#include <memory>
#include <vector>

#include "Settings.h"

namespace blurp
{
    //Resources forward declarations.
    class Window;
    class RenderResourceManager;

    /*
     * Main entry point into rendering with blurp.
     * Create an instance and call Init with a settings object.
     * RenderResources can then be created through the RenderResourceManager.
     */
    class BlurpEngine
    {
        friend class RenderDevice;
    public:
        BlurpEngine();

        /*
         * Initialize this instance of Blurp Engine.
         */
        bool Init(const BlurpSettings& a_Settings);

        /*
         * Get the instance of the window if created.
         * Only one window exists per BlurpEngine instance.
         * Inside BlurpSettings set WindowType to WindowType::NONE to not create a window.
         */
        std::shared_ptr<Window> GetWindow() const;

        /*
         * Get the resource manager instance. 
         */
        RenderResourceManager& GetResourceManager() const;

        /*
         * Get the settings object that the BlurpEngine instance was created with.
         */
        BlurpSettings GetEngineSettings() const;

    private:
        //The render device containing the rendering context.
        std::shared_ptr<RenderDevice> m_RenderDevice;

        //The window belonging to this instance of BlurpEngine.
        //This is nullptr if the initial settings specified WindowType::NONE.
        std::shared_ptr<Window> m_Window;

        //The resource manager instance
        std::unique_ptr<RenderResourceManager> m_ResourceManager;

        //The settings the engine was set up with.
        BlurpSettings m_Settings;
    };
}
