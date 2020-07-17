#pragma once
#include <GL/glew.h>

#include "GpuBuffer.h"

namespace blurp
{
    class GpuBuffer_GL : public GpuBuffer
    {
    public:
        GpuBuffer_GL(const GpuBufferSettings& a_Settings) : GpuBuffer(a_Settings) {}


    protected:
        void OnLock() override;
        void OnUnlock() override;
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;
        GpuBufferRange OnWrite(void* a_Offset, std::uint32_t a_Size, std::uint32_t a_PerDataSize, void* a_Data) override;

    private:
        GLuint m_Ssbo;
    };
}