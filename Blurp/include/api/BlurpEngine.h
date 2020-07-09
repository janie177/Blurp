#pragma once
#include <memory>
#include <vector>

#include "RenderResource.h"

namespace blurp
{
    struct BlurpSettings;
    struct WindowSettings;
    class Window;

    class BlurpEngine
    {
    public:
        BlurpEngine() = default;

        /*
         * Initialize this instance of Blurp Engine.
         */
        bool Init(const BlurpSettings& a_Settings);

        /*
         * Get the instance of the window if created.
         * Only one window exists per BlurpEngine instance.
         * Inside BlurpSettings set WindowType to WindowType::NONE to not create a window.
         */
        std::shared_ptr<Window> GetWindow();

        /*
         * Clean up resources that have gone out of scope.
         * This erases them from the registry.
         */
        void CleanUp();

    private:
        //Registry containing all live resources.
        std::vector<std::shared_ptr<RenderResource>> m_Resources;

        //The window belonging to this instance of BlurpEngine.
        //This is nullptr if the initial settings specified WindowType::NONE.
        std::shared_ptr<Window> m_Window;
    };
}
