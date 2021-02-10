#include "d3d12/DescriptorHeap.h"
#include "d3d12/RenderDevice_D3D12.h"
#include "d3d12/D3D12Utils.h"


namespace blurp
{
    DescriptorHeap::DescriptorHeap(RenderDevice_D3D12& a_RenderDevice, size_t a_NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE a_Type,
        D3D12_DESCRIPTOR_HEAP_FLAGS a_Flags) : m_Type(a_Type), m_Flags(a_Flags), m_RenderDevice(a_RenderDevice)
    {
        assert(m_NumDescriptors > 0 && "Descriptor heaps need at least one capacity.");

        m_AllocationIndex = 0;
        m_NumDescriptors = a_NumDescriptors;
        m_DescriptorSize = a_RenderDevice.GetDevice()->GetDescriptorHandleIncrementSize(a_Type);

        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = static_cast<std::uint32_t>(a_NumDescriptors);
        heapDesc.Type = a_Type;
        heapDesc.Flags = a_Flags;
        ThrowIfFailed(a_RenderDevice.GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_Heap)));
    }

    std::uint32_t DescriptorHeap::GetNextHandleIndex()
    {
        const std::lock_guard<std::mutex> lock{ m_Mutex };
        std::uint32_t index;

        if(!m_NextFree.empty())
        {
            index = m_NextFree.top();
            m_NextFree.pop();
        }
        else if(m_AllocationIndex < m_NumDescriptors)
        {
            index = static_cast<std::uint32_t>(m_AllocationIndex);
            ++m_AllocationIndex;
        }
        else
        {
            Resize(static_cast<uint32_t>(m_NumDescriptors) * 2u);   //Double every time the cap is reached.
            index = static_cast<std::uint32_t>(m_AllocationIndex);  //Allocation index is now the first free index in the heap (exactly in the center).
        }

        return index;
    }

    void DescriptorHeap::FreeDescriptor(const DescriptorHandle& a_Handle)
    {
        const std::lock_guard<std::mutex> lock{m_Mutex};
        m_NextFree.push(a_Handle.index);
    }

    DescriptorHandle DescriptorHeap::GetHeapStart() const
    {
        return DescriptorHandle{ 0, m_Heap->GetCPUDescriptorHandleForHeapStart(), m_Heap->GetGPUDescriptorHandleForHeapStart() };
    }

    DescriptorHandle DescriptorHeap::GetHandle(std::uint32_t a_Index) const
    {
        assert(a_Index <= m_NumDescriptors && "Descriptor index out of bounds!");

        CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_Heap->GetCPUDescriptorHandleForHeapStart());
        CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(m_Heap->GetGPUDescriptorHandleForHeapStart());
        DescriptorHandle handle;
        cpuHandle.Offset(a_Index, static_cast<UINT>(m_DescriptorSize));
        gpuHandle.Offset(a_Index, static_cast<UINT>(m_DescriptorSize));
        handle.cpuHandle = cpuHandle;
        handle.gpuHandle = gpuHandle;
        handle.index = a_Index;

        return handle;
    }

    void DescriptorHeap::Resize(std::uint32_t a_NewSize)
    {
        assert(a_NewSize > m_NumDescriptors && "Descriptor heaps can only grow!");

        //Old heap information.
        const auto oldSize = m_NumDescriptors;
        const auto oldStart = m_Heap->GetCPUDescriptorHandleForHeapStart();

        m_NumDescriptors = a_NewSize;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> tempHeap;
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = static_cast<UINT>(m_NumDescriptors);
        heapDesc.Type = m_Type;
        heapDesc.Flags = m_Flags;
        ThrowIfFailed(m_RenderDevice.GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&tempHeap)));
        m_RenderDevice.GetDevice()->CopyDescriptorsSimple(static_cast<UINT>(oldSize), tempHeap->GetCPUDescriptorHandleForHeapStart(),oldStart, m_Type);
        m_Heap = tempHeap;
    }
}
