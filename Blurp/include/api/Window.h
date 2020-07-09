#pragma once
#include <glm/glm.hpp>

#include "InputQueue.h"
#include "RenderResource.h"
#include "Settings.h"

namespace blurp
{
    struct WindowSettings;
    class SwapChain;

    class Window : public RenderResource
    {
        friend class RenderDevice;

    public:
        Window(const WindowSettings& a_Settings);

        /*
         * Get the SwapChain belonging to this window.
         */
        std::shared_ptr<SwapChain> GetSwapChain() const;

        /*
         * Close the window.
         */
        virtual void Close() = 0;

        /*
         * Resize this window to the specified dimensions.
         */
        virtual void Resize(glm::vec2 a_Dimensions) = 0;

        /*
         * Returns whether this window is currently full screen mode.
         */
        virtual bool IsFullScreen() = 0;

        /*
         * Toggle fullscreen state on or off.
         */
        virtual void SetFullScreen(bool a_FullScreen) = 0;

        /*
         * Set the mouse position for this window.
         * This caps to be within 0,0 and width,height.
         */
        virtual void SetMousePosition(const glm::vec2& a_MousePosition) = 0;

        /*
         * Get the mouse position.
         */
        virtual glm::vec2 GetMousePosition() const = 0;

        /*
         * Get a queue of all input events that happened since this was last called.
         */
        virtual InputData PollInput() = 0;

        /*
         * Returns whether this window has closed or not.
         */
        virtual bool IsClosed() const = 0;

    protected:
        /*
         * Associate this window with the given SwapChain.
         */
        void BindSwapChain(std::shared_ptr<SwapChain> a_SwapChain);

    protected:
        //Initial window settings.
        WindowSettings m_Settings;

    private:
        std::shared_ptr<SwapChain> m_SwapChain;
    };
}
