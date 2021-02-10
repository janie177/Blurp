#include "d3d12/SwapChain_D3D12_Win32.h"
#include "BlurpEngine.h"
#include "Window_Win32.h"
#include "d3d12/RenderDevice_D3D12.h"
#include "d3d12/D3D12Utils.h"


namespace blurp
{
    void SwapChain_D3D12_Win32::Resize(const glm::vec2& a_Dimensions, bool a_FullScreen)
    {
        //TODO
    }

    std::uint16_t SwapChain_D3D12_Win32::GetNumBuffers()
    {
        //TODO
        return 0;
    }

    std::shared_ptr<RenderTarget> SwapChain_D3D12_Win32::GetRenderTarget()
    {
        //TODO
        return nullptr;
    }

    void SwapChain_D3D12_Win32::Present()
    {
        //TODO present and swap m_FrameIndex.
    }

    bool SwapChain_D3D12_Win32::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        //Retrieve a pointer to the window.
        const auto window = a_BlurpEngine.GetWindow().get();

        if (window->GetWindowType() != WindowType::WINDOW_WIN32)
        {
            throw std::exception("SwapChain_GL_Win32 can only be constructed for a Win32 window with OpenGL!");
            return false;
        }

        auto win32Window = reinterpret_cast<Window_Win32*>(window);
        const auto hwnd = win32Window->GetHwnd();

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = m_Settings.numBuffers;
        swapChainDesc.Width = static_cast<UINT>(win32Window->GetDimensions().x);
        swapChainDesc.Height = static_cast<UINT>(win32Window->GetDimensions().y);
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  //TODO convert m_Settings format and channels to match here.
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;

        //TODO load attachments (RTVs, DSV). Create these and then store them.
        //TODO initialize the render targets here and pass the correct buffers.

        auto& graphicsQueue = m_RenderDevice.GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

        Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
        ThrowIfFailed(m_RenderDevice.GetFactory()->CreateSwapChainForHwnd(
            graphicsQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
            hwnd,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain
        ));

        ThrowIfFailed(m_RenderDevice.GetFactory()->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

        ThrowIfFailed(swapChain.As(&m_SwapChain));
        m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

        return true;
    }

    bool SwapChain_D3D12_Win32::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        //TODO
        return true;
    }
}
