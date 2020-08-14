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

        GpuBufferView OnWrite(void* a_Offset, std::uint32_t a_Count, std::uint32_t a_LargestMemberSize, std::uint32_t a_PerDataSize, const void* a_Data) override;

    public:
        std::vector<Lockable*> GetRecursiveLockables() override;

    private:
        GLuint m_Ssbo;
    };
}