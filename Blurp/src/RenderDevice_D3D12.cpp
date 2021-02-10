#include "d3d12/RenderDevice_D3D12.h"

#include "BlurpEngine.h"
#include "Camera.h"
#include "d3d12/D3D12Utils.h"
#include "d3d12/SwapChain_D3D12_Win32.h"
#include "Light.h"
#include "Material.h"

namespace blurp
{
    Microsoft::WRL::ComPtr<ID3D12Device>& RenderDevice_D3D12::GetDevice()
    {
        return m_Device;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory6>& RenderDevice_D3D12::GetFactory()
    {
        return m_Factory;
    }

    Microsoft::WRL::ComPtr<ID3D12CommandQueue>& RenderDevice_D3D12::GetCommandQueue(D3D12_COMMAND_LIST_TYPE a_Type)
    {
        return m_CommandQueues[a_Type];
    }

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> RenderDevice_D3D12::CreateCommandList(
        D3D12_COMMAND_LIST_TYPE a_Type, Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& a_Allocator,
        Microsoft::WRL::ComPtr<ID3D12PipelineState>& a_DefaultState) const
    {
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> list;
        ThrowIfFailed(m_Device->CreateCommandList(0, a_Type, a_Allocator.Get(), a_DefaultState.Get(), IID_PPV_ARGS(&list)));
        list->Close();
        return list;
    }

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> RenderDevice_D3D12::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE a_Type) const
    {
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
        ThrowIfFailed(m_Device->CreateCommandAllocator(a_Type, IID_PPV_ARGS(&allocator)));
        return allocator;
    }

    DescriptorHeap& RenderDevice_D3D12::GetCbvUavSrvHeap()
    {
        return m_SrvUavCbvHeap;
    }

    DescriptorHeap& RenderDevice_D3D12::GetRtvHeap()
    {
        return m_RtvHeap;
    }

    DescriptorHeap& RenderDevice_D3D12::GetDsvHeap()
    {
        return m_DsvHeap;
    }

    bool RenderDevice_D3D12::Init(BlurpEngine& a_BlurpEngine, const WindowSettings& a_WindowSettings)
    {
        const auto window = a_BlurpEngine.GetWindow();
        if (window == nullptr)
        {
            throw std::exception("For D3D12 contexts, a window is required! Technically it's not but I didn't implement it that way.");
            return false;
        }

        UINT dxgiFactoryFlags = 0;

#ifdef _DEBUG
        {
            Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }
#endif

        //Use the latest D3D12 version.
        auto featureLevel = D3D_FEATURE_LEVEL_12_1;

        //Create the factory with debug layer if in debug mode.
        ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_Factory)));

        //No software adapter option for now, just find the first hardware adapter.
        for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != m_Factory->EnumAdapters1(adapterIndex, &m_HardwareAdapter); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            m_HardwareAdapter->GetDesc1(&desc);

            if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) && SUCCEEDED(D3D12CreateDevice(m_HardwareAdapter.Get(), featureLevel, _uuidof(ID3D12Device), nullptr)))
            {
                //Adapter found.
                break;
            }
        }

        ThrowIfFailed(D3D12CreateDevice(
            m_HardwareAdapter.Get(),
            featureLevel,
            IID_PPV_ARGS(&m_Device)
        ));

        //Create the command queues. One for each type of operation.
        for(int i = 0; i < 7; ++i)
        {
            D3D12_COMMAND_QUEUE_DESC queueDesc = {};
            queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            queueDesc.Type = static_cast<D3D12_COMMAND_LIST_TYPE>(i);

            ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueues[i])));
        }

        //Create a swapchain, which will internally access the window and bind to it.
        const auto swapChain = CreateSwapChain(a_WindowSettings);

        //Bind the swapchain to the window.This loads the swapchain.
        BindWindowAndSwapChain(a_BlurpEngine, window.get(), swapChain);

        return true;
    }

    std::shared_ptr<SwapChain> RenderDevice_D3D12::CreateSwapChain(const WindowSettings& a_Settings)
    {
        switch (a_Settings.type)
        {
        case WindowType::WINDOW_WIN32:
            return std::make_shared<SwapChain_D3D12_Win32>(a_Settings.swapChainSettings, *this);
        default:
        {
            throw std::exception("Swap chain for given window not implemented for OpenGL!");
            return nullptr;
        }
        }
    }
}
