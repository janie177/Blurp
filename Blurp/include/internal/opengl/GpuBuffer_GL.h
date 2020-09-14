#pragma once
#include <GL/glew.h>

#include "GpuBuffer.h"

namespace blurp
{
    class GpuBuffer_GL : public GpuBuffer
    {
    public:
        GpuBuffer_GL(const GpuBufferSettings& a_Settings);

        GLuint GetBufferId() const;

    protected:
        void OnLock() override;
        void OnUnlock() override;
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;

        GpuBufferView OnWrite(std::uintptr_t a_Offset, std::uint32_t a_Count, std::uint32_t a_LargestMemberSize, std::uint32_t a_PerDataSize, const void* a_Data) override;

    public:

        bool Resize(std::uint32_t a_Size, bool a_CopyData = true) override;
        GpuBufferView WriteData(std::uint32_t a_Offset, const PerInstanceUploadData& a_UploadData) override;
        GpuBufferView WriteData(std::uint32_t a_Offset, const GlobalUploadData& a_UploadData) override;
        GpuBufferView WriteData(std::uint32_t a_Offset, const LightUploadData& a_UploadData) override;
    private:
        GLuint m_Ssbo;
    };
}