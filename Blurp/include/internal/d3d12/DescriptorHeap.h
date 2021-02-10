#pragma once
#include <queue>
#include <d3d12.h>
#include <mutex>
#include <stack>
#include <wrl/client.h>

namespace blurp
{
    class RenderDevice_D3D12;

    /*
     * Struct containing descriptor handles and index.
     * Note: These handles are invalidated when the heap is resized. Only store the index and dynamically look the handles up when needed.
     */
    struct DescriptorHandle
    {
        std::uint32_t index;    //Index into the descriptor heap of this descriptor.
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    };

    /*
     * A class managing an descriptor heap of a certain type.
     */
    class DescriptorHeap
    {
    public:
        DescriptorHeap(RenderDevice_D3D12& a_RenderDevice, size_t a_NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE a_Type, D3D12_DESCRIPTOR_HEAP_FLAGS a_Flags);

        /*
         * Get the next free handle in the descriptor heap.
         * If not enough space is available in the heap, 
         */
        std::uint32_t GetNextHandleIndex();

        /*
         * Mark a descriptor as free.
         * This will allow it's index to be overwritten again when GetFreeHandle is called.
         */
        void FreeDescriptor(const DescriptorHandle& a_Handle);

        /*
         * Get a handle to the descriptor heap start.
         */
        DescriptorHandle GetHeapStart() const;

        /*
         * Get the CPU and GPU descriptor handles for the given index.
         */
        DescriptorHandle GetHandle(std::uint32_t a_Index) const;

        /*
         * Resize this descriptor heap.
         * The new size must be larger than the old.
         */
        void Resize(std::uint32_t a_NewSize);

    private:
        size_t m_AllocationIndex;   //Index of current allocation. The nextFree container will be checked first before incrementing this.
        size_t m_NumDescriptors;    //The maximum amount of descriptors in this heap.
        size_t m_DescriptorSize;    //The size per descriptor.
        std::stack<std::uint32_t> m_NextFree;  //A vector containing the index of the next free descriptor.

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;
        D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
        D3D12_DESCRIPTOR_HEAP_FLAGS m_Flags;

        RenderDevice_D3D12& m_RenderDevice;

        std::mutex m_Mutex;         //Ensure thread safety.
    };
}
