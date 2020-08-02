#pragma once
#include "Lockable.h"
#include "RenderResource.h"

namespace blurp
{
    struct GpuBufferView
    {
        //Pointer to the start of the data added in the GPU buffer.
        //This is measured as an offset from the start of the buffer.
        std::uintptr_t start;

        //The total size in bytes of the data between start and end.
        std::uintptr_t size;

        //Stride in bytes between each element on the GPU with padding.
        std::uintptr_t stride;

        //Pointer to the end of the data added in the GPU buffer.
        //This is measured as the offset from the start of the buffer.
        std::uintptr_t end;
    };

    class GpuBuffer : public RenderResource, public Lockable
    {
    public:
        GpuBuffer(const GpuBufferSettings& a_Settings) : m_Settings(a_Settings) {}

        /*
         * Write raw data into this GPU buffer.
         * a_Offset indicates the offset into the GPU buffer to start writing.
         * a_Count is the amount of elements of type T to upload.
         * a_LargestMemberSize is the size in bytes of the largest member of the data type provided.
         * a_Data is a pointer to the first element of type T.
         *
         * Padding is automatically added to ensure hardware compatibility.
         *
         * A void pointer indicating the offset into the buffer is returned.
         */
        template<typename T>
        GpuBufferView WriteData(void* a_Offset, std::uint32_t a_Count, std::uint32_t a_LargestMemberSize, T* a_Data);

    protected:
        /*
         * Called when data has to be written to the GPU buffer.
         * a_Offset is the offset from the start of the buffer.
         * a_Size is the size of the amount of elements in the dataset.
         * a_PerDataSize is the size in bytes for a single element.
         * a_Data is a pointer to the start of the dataset.
         *
         * Padding should be added after each element depending on buffer hardware requirements.
         *
         * A struct containing pointers to the start and end of the data in the 
         */
        virtual GpuBufferView OnWrite(void* a_Offset, std::uint32_t a_Count, std::uint32_t a_LargestMemberSize, std::uint32_t a_PerDataSize, void* a_Data) = 0;

    protected:
        GpuBufferSettings m_Settings;
    };

    template <typename T>
    GpuBufferView GpuBuffer::WriteData(void* a_Offset, std::uint32_t a_Count, std::uint32_t a_LargestMemberSize, T* a_Data)
    {
        assert(!IsLocked() && "Cannot write data into a locked GPUBuffer!");
        return OnWrite(a_Offset, a_Count, a_LargestMemberSize, static_cast<std::uint32_t>(sizeof(T)), static_cast<void*>(a_Data));
    }
}
