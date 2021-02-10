#pragma once
#include "Window.h"
#include "SwapChain.h"
#include <windows.h>

#include "RenderTarget_D3D12.h"

#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dx12/d3dx12.h>

namespace blurp
{
    class RenderDevice_D3D12;

    class SwapChain_D3D12_Win32 : public SwapChain
    {
    public:
        SwapChain_D3D12_Win32(const SwapChainSettings& a_Settings, RenderDevice_D3D12& a_RenderDevice) : SwapChain(a_Settings), m_RenderDevice(a_RenderDevice)
        {
        }

        void Resize(const glm::vec2& a_Dimensions, bool a_FullScreen) override;

        std::uint16_t GetNumBuffers() override;

        std::shared_ptr<RenderTarget> GetRenderTarget() override;

        void Present() override;
    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;

    private:
        int m_FrameIndex;
        Microsoft::WRL::ComPtr<IDXGISwapChain3> m_SwapChain;
        std::vector<std::shared_ptr<RenderTarget_D3D12>> m_RenderTargets;   //One RenderTarget per frame.
        RenderDevice_D3D12& m_RenderDevice;
    };
}