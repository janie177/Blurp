#include "Window.h"

#include <utility>
#include "Settings.h"
#include "SwapChain.h"

namespace blurp
{
    Window::Window(const WindowSettings& a_Settings) : m_Settings(a_Settings)
    {
        //Set the render target dimensions to match the window.
        m_Settings.swapChainSettings.renderTargetSettings.viewPort = { 0.f, 0.f, m_Settings.dimensions };
        m_ResizeCallback = [](int, int) {};
    }

    std::shared_ptr<SwapChain> Window::GetSwapChain() const
    {
        return m_SwapChain;
    }

    std::shared_ptr<RenderTarget> Window::GetRenderTarget() const
    {
        return m_SwapChain->GetRenderTarget();
    }

    void Window::SetResizeCallback(std::function<void(int, int)> a_Function)
    {
        m_ResizeCallback = std::move(a_Function);
    }

    WindowType Window::GetWindowType() const
    {
        return m_Settings.type;
    }

    void Window::Present() const
    {
        if(m_SwapChain != nullptr)
        {
            m_SwapChain->Present();
        }
    }

    void Window::BindSwapChain(std::shared_ptr<SwapChain> a_SwapChain)
    {
        m_SwapChain = std::move(a_SwapChain);
    }
}
