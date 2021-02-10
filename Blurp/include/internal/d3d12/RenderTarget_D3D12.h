#pragma once
#include "RenderTarget.h"
#include <d3d12.h>
#include <wrl/client.h>


namespace blurp
{
    class RenderDevice_D3D12;

    class RenderTarget_D3D12 : public RenderTarget
    {
        friend class SwapChain_D3D12_Win32;
    public:
        RenderTarget_D3D12(const RenderTargetSettings& a_Settings, RenderDevice_D3D12& a_RenderDevice) : RenderTarget(a_Settings), m_RenderDevice(a_RenderDevice), m_SwapChainOwned(false) {}

        RenderTarget_D3D12(const RenderTargetSettings& a_Settings, RenderDevice_D3D12& a_RenderDevice, const Microsoft::WRL::ComPtr<ID3D12Resource>& a_SwapChainBuffer, const Microsoft::WRL::ComPtr<ID3D12Resource>& a_SwapChainDepthStencilBuffer) : RenderTarget(a_Settings), m_RenderDevice(a_RenderDevice), m_SwapChainBuffer(a_SwapChainBuffer), m_SwapChainDepthStencilBuffer(a_SwapChainDepthStencilBuffer), m_SwapChainOwned(true) {}

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;

    public:
        /*
         * Set this render target to be the target for the given command list.
         */
        void Bind(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& a_CommandList);

        glm::vec4 GetClearColor() override;
        void SetClearColor(const glm::vec4& a_ClearColor) override;
        glm::vec4 GetViewPort() override;
        void SetViewPort(const glm::vec<4, std::uint32_t>& a_ViewPort) override;
        glm::vec4 GetScissorRect() override;
        void SetScissorRect(const glm::vec<4, std::uint32_t>& a_ScissorRect) override;
        void OnColorAttachmentBound(std::uint16_t a_Slot, const std::shared_ptr<Texture>& a_Added) override;
        void OnDepthStencilAttachmentBound(const std::shared_ptr<Texture>& a_Added) override;

    private:
        glm::vec4 m_ClearColor;
        glm::vec4 m_ViewPort;
        glm::vec4 m_ScissorRect;

        RenderDevice_D3D12& m_RenderDevice;

        //When true, the buffers are external and owned by the swapchain.In this case state tracking is internal instead of in the texture.
        bool m_SwapChainOwned;  
        Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainDepthStencilBuffer;
        D3D12_RESOURCE_STATES m_State;

        //Indices into the RTV and CBV heaps.
        std::uint32_t m_ColorAttachmentRtvIndices[MAX_NUM_COLOR_ATTACHMENTS];
        std::uint32_t m_DsvAttachmentIndex;
    };
}
