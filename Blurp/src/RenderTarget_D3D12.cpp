#include "d3d12/RenderTarget_D3D12.h"

#include "d3d12/RenderDevice_D3D12.h"
#include "d3d12/DescriptorHeap.h"

namespace blurp
{
    bool RenderTarget_D3D12::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        //Allocate indices in the heaps. This means that all render targets always reserve slots, which may not be wanted? Consider changing this to dynamically reserve at runtime.
        m_DsvAttachmentIndex = m_RenderDevice.GetDsvHeap().GetNextHandleIndex();
        for(int i = 0; i < MAX_NUM_COLOR_ATTACHMENTS; ++i)
        {
            m_ColorAttachmentRtvIndices[i] = m_RenderDevice.GetRtvHeap().GetNextHandleIndex();
        }

        //This RenderTarget is part of a SwapChain and it's back buffer is managed.
        if(m_SwapChainOwned)
        {
            //Initial state is present. //TODO: Swap to STATE_RENDER_TARGET when bound but then when do I swap back? Probably at the end of the pipeline.
            m_State = D3D12_RESOURCE_STATE_PRESENT;

            //TODO create RTV and DSV for existing buffer. Use the m_Settings for formats.
            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
            //rtvDesc.Format = ;
            //rtvDesc.ViewDimension = ;
            //rtvDesc.Texture2D.;
            m_RenderDevice.GetDevice()->CreateRenderTargetView(m_SwapChainBuffer.Get(), &rtvDesc, m_RenderDevice.GetRtvHeap().GetHandle(m_ColorAttachmentRtvIndices[0]).cpuHandle);
            m_RenderDevice.GetDevice()->CreateDepthStencilView(m_SwapChainBuffer.Get(), &dsvDesc, m_RenderDevice.GetRtvHeap().GetHandle(m_ColorAttachmentRtvIndices[0]).cpuHandle);
        }
        //This RenderTarget is a standalone.
        else
        {
            //TODO load settings and create RTV for attachments.
        }

        return true;
    }

    bool RenderTarget_D3D12::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        //TODO free indices.
        return true;
    }

    void RenderTarget_D3D12::Bind(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& a_CommandList)
    {
        //TODO set render targets and state if state is not correct yet.
        //a_CommandList->ResourceBarrier();
        //a_CommandList->OMSetRenderTargets();
    }
}
