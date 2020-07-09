#include "Window.h"

#include <utility>
#include "Settings.h"

namespace blurp
{
    Window::Window(const WindowSettings& a_Settings) : m_Settings(a_Settings)
    {

    }

    std::shared_ptr<SwapChain> Window::GetSwapChain() const
    {
        return m_SwapChain;
    }

    void Window::BindSwapChain(std::shared_ptr<SwapChain> a_SwapChain)
    {
        m_SwapChain = std::move(a_SwapChain);
    }
}
