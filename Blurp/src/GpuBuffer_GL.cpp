#include "opengl/GpuBuffer_GL.h"

#include <algorithm>


#include "opengl/GLUtils.h"

namespace blurp
{
    //Init the static counter variable.
    //Start at 1 so that binding 0 is always available for copying while resizing buffers.
    std::uint32_t GpuBuffer_GL::m_BufferCounter = 1;

    GpuBuffer_GL::GpuBuffer_GL(const GpuBufferSettings& a_Settings): GpuBuffer(a_Settings), m_Ssbo(0), m_BufferBaseBinding(0)
    {

    }

    GLuint GpuBuffer_GL::GetBufferId() const
    {
        return m_Ssbo;
    }

    std::uint32_t GpuBuffer_GL::GetBufferBaseBinding() const
    {
        return m_BufferBaseBinding;
    }

    void GpuBuffer_GL::OnLock()
    {

    }

    void GpuBuffer_GL::OnUnlock()
    {

    }

    bool GpuBuffer_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        //Up the base binding ID and store it.
        m_BufferBaseBinding = m_BufferCounter;
        ++m_BufferCounter;

        //Ensure that the ID is not higher than the max amount
        GLint maxBuffers;
        glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &maxBuffers);
        assert(static_cast<std::uint32_t>(maxBuffers) > m_BufferCounter && ("Too many GpuBuffers! The current GPU does not support this many."));

        glGenBuffers(1, &m_Ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_Ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, m_Settings.size, nullptr, ToGL(m_Settings.memoryUsage));
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_BufferBaseBinding, m_Ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        return true;
    }

    bool GpuBuffer_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        glDeleteBuffers(1, &m_Ssbo);
        return true;
    }

    GpuBufferView GpuBuffer_GL::OnWrite(void* a_Offset, std::uint32_t a_Count, std::uint32_t a_LargestMemberSize,
        std::uint32_t a_PerDataSize, void* a_Data)
    {
        //Alignment for std430 is equal to the largest member size.
        const std::uintptr_t alignment = std::min(16u, a_LargestMemberSize);
        const std::uintptr_t startPadding = (alignment - (reinterpret_cast<std::uintptr_t>(a_Offset) & (alignment - 1))) & (alignment - 1);
        const std::uintptr_t elementPadding = (alignment - (a_PerDataSize & (alignment - 1))) & (alignment - 1);
        const std::uintptr_t elementPaddedSize = elementPadding +  a_PerDataSize;
        const std::uintptr_t sizeFromAlignedStart = elementPaddedSize * a_Count;


        //The size of the newly added data when padded.
        const std::uintptr_t paddedSize = startPadding + (a_Count * elementPaddedSize);

        //The total size required with the new data.
        const auto totalSize = reinterpret_cast<std::uintptr_t>(a_Offset) + paddedSize;

        //Overwriting buffer limits.
        if (totalSize > m_Settings.size)
        {
            //Resize to double the size. Create a new buffer and move data into it.
            if (m_Settings.resizeWhenFull)
            {
                const auto oldSize = m_Settings.size;

                //Keep doubling the size until it fits.
                while (m_Settings.size < totalSize)
                {
                    m_Settings.size *= 2;
                }

                //Create a new bigger buffer.
                GLuint tempBuffer;
                glGenBuffers(1, &tempBuffer);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, tempBuffer);
                glBufferData(GL_SHADER_STORAGE_BUFFER, m_Settings.size, nullptr, ToGL(m_Settings.memoryUsage));
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, tempBuffer);

                //Copy over the contents from the old buffer to the new buffer.
                glCopyNamedBufferSubData(m_Ssbo, tempBuffer, 0, 0, oldSize);

                //Delete the old buffer.
                glDeleteBuffers(1, &m_Ssbo);

                //Store the new buffer handle and set it to the correct binding point.
                m_Ssbo = tempBuffer;
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_BufferBaseBinding, m_Ssbo);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            }
            //Assert and warn.
            else
            {
                throw std::exception("Gpu Buffer size limit reached. Assign more space to the buffer by increasing setting.size.");
            }
        }

        //Bind the buffer and write to it.
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_Ssbo);

        //Reserve enough memory to temporarily store the data and add padding.
        std::vector<char> paddedData;
        paddedData.resize(paddedSize);

        //Add the data, skipping the start alignment padding.
        for(std::uint32_t i = 0; i < a_Count; ++i)
        {
            const auto index = startPadding + (i * elementPaddedSize);
            memcpy(static_cast<void*>(&paddedData[index]), reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(a_Data) + (i * static_cast<std::uintptr_t>(a_PerDataSize))), a_PerDataSize);
        }

        //Upload the padded data to the GPU.
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, reinterpret_cast<std::uintptr_t>(a_Offset), paddedSize, &paddedData[0]);

        //Unbind the buffer.
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        return GpuBufferView{reinterpret_cast<std::uintptr_t>(a_Offset) + startPadding, sizeFromAlignedStart, elementPaddedSize, totalSize};
    }

    std::vector<Lockable*> GpuBuffer_GL::GetRecursiveLockables()
    {
        //Return an empty vector because there's no internal lockables.
        return std::vector<Lockable*>();
    }
}