#pragma once
#include <glm/glm.hpp>

#include "InputQueue.h"
#include "RenderResource.h"
#include "Settings.h"

namespace blurp
{
    struct WindowSettings;
    class SwapChain;

    class Window
    {
        friend class RenderDevice;
        friend class BlurpEngine;

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
         *
         * This regards the native OS mouse.
         */
        virtual void SetNativeMousePosition(const glm::vec2& a_MousePosition) = 0;

        /*
         * Get the mouse position of the native OS mouse.
         */
        virtual glm::vec2 GetNativeMousePosition() const = 0;

        /*
         * Get a queue of all input events that happened since this was last called.
         */
        virtual InputData PollInput() = 0;

        /*
         * Returns whether this window has closed or not.
         */
        virtual bool IsClosed() const = 0;

        /*
         * Get the type of this window.
         */
        WindowType GetWindowType() const;

        /*
         * Swap the front and back buffers in the swapchain.
         */
        void Present() const;

    protected:
        /*
         * Associate this window with the given SwapChain.
         */
        void BindSwapChain(std::shared_ptr<SwapChain> a_SwapChain);

        /*
         * Load this window and initialize it.
         */
        virtual void Load() = 0;

        /*
         * Destroy this window. Clean up what needs cleaning up.
         */
        virtual void Destroy() = 0;

    protected:
        //Initial window settings.
        WindowSettings m_Settings;

    private:
        std::shared_ptr<SwapChain> m_SwapChain;
    };
}
