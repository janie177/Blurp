#pragma once
#include "RenderDevice.h"
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dx12/d3dx12.h>

#include "DescriptorHeap.h"

namespace blurp
{
    class BlurpEngine;

    /*
     * Sets up OpenGL context and
     */
    class RenderDevice_D3D12 : public RenderDevice
    {
    public:
        RenderDevice_D3D12(BlurpEngine& a_Engine) : RenderDevice(a_Engine)
            , m_RtvHeap(*this, 1024, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
            , m_SrvUavCbvHeap(*this, 1024, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
            , m_DsvHeap(*this, 1024, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
        {}

        /*
         * Get a reference to the D3D12 Device instance.
         */
        Microsoft::WRL::ComPtr<ID3D12Device>& GetDevice();

        /*
         * Get a renference to the D3D12 factory.
         */
        Microsoft::WRL::ComPtr<IDXGIFactory6>& GetFactory();

        /*
         * Get a reference to the CommandQueue of the given type.
         */
        Microsoft::WRL::ComPtr<ID3D12CommandQueue>& GetCommandQueue(D3D12_COMMAND_LIST_TYPE a_Type);

        /*
         * Create a new command list of the given type.
         */
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CreateCommandList(D3D12_COMMAND_LIST_TYPE a_Type, Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& a_Allocator, Microsoft::WRL::ComPtr<ID3D12PipelineState>& a_DefaultState) const;

        /*
         * Create a new command allocator for the given type of command list.
         */
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE a_Type) const;

        /*
         * Get a reference to the descriptor heap.
         */
        DescriptorHeap& GetCbvUavSrvHeap();

        /*
         * Get a reference to the descriptor heap.
         */
        DescriptorHeap& GetRtvHeap();

        /*
         * Get a reference to the descriptor heap.
         */
        DescriptorHeap& GetDsvHeap();

    protected:
        bool Init(BlurpEngine& a_BlurpEngine, const WindowSettings& a_WindowSettings) override;

        std::shared_ptr<Light> CreateLight(const LightSettings& a_Settings) override;
        std::shared_ptr<Camera> CreateCamera(const CameraSettings& a_Settings) override;
        std::shared_ptr<Mesh> CreateMesh(const MeshSettings& a_Settings) override;
        std::shared_ptr<Texture> CreateTexture(const TextureSettings& a_Settings) override;
        std::shared_ptr<RenderTarget> CreateRenderTarget(const RenderTargetSettings& a_Settings) override;
        std::shared_ptr<SwapChain> CreateSwapChain(const WindowSettings& a_Settings) override;
        std::shared_ptr<Material> CreateMaterial(const MaterialSettings& a_Settings) override;
        std::shared_ptr<RenderPass> CreateRenderPass(RenderPassType& a_Type, RenderPipeline& a_Pipeline) override;
        std::shared_ptr<RenderPipeline> CreatePipeline(const PipelineSettings& a_Settings) override;
        std::shared_ptr<Shader> CreateShader(const ShaderSettings& a_Settings) override;
        std::shared_ptr<GpuBuffer> CreateGpuBuffer(const GpuBufferSettings& a_Settings) override;
        std::shared_ptr<MaterialBatch> CreateMaterialBatch(const MaterialBatchSettings& a_Settings) override;

    private:
        Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
        Microsoft::WRL::ComPtr<IDXGIFactory6> m_Factory;
        Microsoft::WRL::ComPtr<IDXGIAdapter1> m_HardwareAdapter;

        //Create a command queue for each type in D3D12.
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueues[7];

        //Descriptor heaps.
        DescriptorHeap m_RtvHeap;
        DescriptorHeap m_SrvUavCbvHeap;
        DescriptorHeap m_DsvHeap;
    };
}
