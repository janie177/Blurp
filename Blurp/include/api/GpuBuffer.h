#pragma once
#include "Lockable.h"
#include "RenderResource.h"
#include "GpuBufferView.h"

namespace blurp
{
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
        GpuBufferView WriteData(void* a_Offset, std::uint32_t a_Count, std::uint32_t a_LargestMemberSize, const T* a_Data);

        /*
         * Get the size in bytes of this GPU Buffer.
         */
        std::uint32_t GetSize() const;

        /*
         * Resize this GPU Buffer to fit the new size in bytes.
         * Optionally specify whether data should be copied over or not. True by default.
         * Returns true if the buffer was successfully resized.
         */
        virtual bool Resize(std::uint32_t a_Size, bool a_CopyData = true) = 0;

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
        virtual GpuBufferView OnWrite(void* a_Offset, std::uint32_t a_Count, std::uint32_t a_LargestMemberSize, std::uint32_t a_PerDataSize, const void* a_Data) = 0;

    protected:
        GpuBufferSettings m_Settings;
    };

    inline std::uint32_t GpuBuffer::GetSize() const
    {
        return m_Settings.size;
    }

    template <typename T>
    GpuBufferView GpuBuffer::WriteData(void* a_Offset, std::uint32_t a_Count, std::uint32_t a_LargestMemberSize, const T* a_Data)
    {
        assert(!IsLocked() && "Cannot write data into a locked GPUBuffer!");
        assert(m_Settings.access != AccessMode::READ_ONLY && "Attempting to write to a read-only GPU Buffer.");
        return OnWrite(a_Offset, a_Count, a_LargestMemberSize, static_cast<std::uint32_t>(sizeof(T)), static_cast<const void*>(a_Data));
    }
}
