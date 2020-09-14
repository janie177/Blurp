#include "opengl/GpuBuffer_GL.h"

#include <algorithm>


#include "opengl/GLUtils.h"
#include "Light.h"

namespace blurp
{
    GpuBuffer_GL::GpuBuffer_GL(const GpuBufferSettings& a_Settings): GpuBuffer(a_Settings), m_Ssbo(0)
    {

    }

    GLuint GpuBuffer_GL::GetBufferId() const
    {
        return m_Ssbo;
    }

    void GpuBuffer_GL::OnLock()
    {

    }

    void GpuBuffer_GL::OnUnlock()
    {

    }

    bool GpuBuffer_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        glGenBuffers(1, &m_Ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_Ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, m_Settings.size, nullptr, ToGL(m_Settings.memoryUsage));
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        return true;
    }

    bool GpuBuffer_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        glDeleteBuffers(1, &m_Ssbo);
        return true;
    }

    GpuBufferView GpuBuffer_GL::OnWrite(std::uintptr_t a_Offset, std::uint32_t a_Count, std::uint32_t a_LargestMemberSize,
        std::uint32_t a_PerDataSize, const void* a_Data)
    {
        //Alignment for std430 is equal to the largest member size.
        const std::uintptr_t alignment = std::min(16u, a_LargestMemberSize);
        const std::uintptr_t startPadding = (alignment - (a_Offset & (alignment - 1))) & (alignment - 1);
        const std::uintptr_t elementPadding = (alignment - (a_PerDataSize & (alignment - 1))) & (alignment - 1);
        const std::uintptr_t elementPaddedSize = elementPadding +  a_PerDataSize;
        const std::uintptr_t sizeFromAlignedStart = elementPaddedSize * a_Count;


        //The size of the newly added data when padded.
        const std::uintptr_t paddedSize = startPadding + (a_Count * elementPaddedSize);

        //The total size required with the new data.
        const auto totalSize = a_Offset + paddedSize;

        //Overwriting buffer limits.
        if (totalSize > m_Settings.size)
        {
            //Resize to double the size. Create a new buffer and move data into it.
            if (m_Settings.resizeWhenFull)
            {
                //Keep doubling till it fits.
                std::uint32_t newSize = m_Settings.size;
                while(newSize < totalSize)
                {
                    newSize *= 2;
                }

                //Resize and copy the data.
                Resize(newSize, true);
            }
            //No auto resizing allowed so crash the program.
            else
            {
                throw std::exception("Gpu Buffer size limit reached. Assign more space to the buffer by increasing setting.size or enable auto resizing.");
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
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, a_Offset, paddedSize, &paddedData[0]);

        //Unbind the buffer.
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        return GpuBufferView(a_Offset + startPadding, sizeFromAlignedStart, elementPaddedSize);
    }

    bool GpuBuffer_GL::Resize(std::uint32_t a_Size, bool a_CopyData)
    {
        assert(!IsLocked() && "Cannot resize a Gpu Buffer that is currently locked!");

        //Update settings size.
        const auto oldSize = m_Settings.size;
        m_Settings.size = a_Size;

        //Copy old data to the new buffer if specified.
        if(a_CopyData)
        {
            //Create a new bigger buffer.
            GLuint tempBuffer;
            glGenBuffers(1, &tempBuffer);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, tempBuffer);
            glBufferData(GL_SHADER_STORAGE_BUFFER, m_Settings.size, nullptr, ToGL(m_Settings.memoryUsage));

            //Copy over the contents from the old buffer to the new buffer.
            glCopyNamedBufferSubData(m_Ssbo, tempBuffer, 0, 0, oldSize);

            //Delete the old buffer.
            glDeleteBuffers(1, &m_Ssbo);

            //Store the new buffer handle and set it to the correct binding point.
            m_Ssbo = tempBuffer;
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        }
        //No copying needed, so just resize and no need to generate a new buffer.
        else
        {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_Ssbo);
            glBufferData(GL_SHADER_STORAGE_BUFFER, m_Settings.size, nullptr, ToGL(m_Settings.memoryUsage));
        }

        return true;
    }

    GpuBufferView GpuBuffer_GL::WriteData(std::uint32_t a_Offset, const PerInstanceUploadData& a_UploadData)
    {
        assert(a_UploadData.drawData != nullptr && "DrawData cannot be nullptr.");

        //The size of each element containing one of each enabled draw attribute.
        //This is correctly padded by default because each individual datatype is padded.
        std::uint32_t elementSize = 0;

        //Check which drawAttributes are enabled that are single data.
        bool transformEnabled = false;
        bool normalMatrixEnabled = false;

        if(a_UploadData.drawData->attributes.IsAttributeEnabled(DrawAttribute::TRANSFORMATION_MATRIX))
        {
            auto info = DRAW_ATTRIBUTE_INFO.find(DrawAttribute::TRANSFORMATION_MATRIX);
            assert(info != DRAW_ATTRIBUTE_INFO.end() && "Unknown draw attribute! Not registered in info map.");
            assert(a_UploadData.transforms != nullptr && "DrawAttribute enabled, but corresponding data is not provided!");
            transformEnabled = true;
            elementSize += info->second.size;   //No padding for Mat4.

        }
        if (a_UploadData.drawData->attributes.IsAttributeEnabled(DrawAttribute::NORMAL_MATRIX))
        {
            auto info = DRAW_ATTRIBUTE_INFO.find(DrawAttribute::NORMAL_MATRIX);
            assert(info != DRAW_ATTRIBUTE_INFO.end() && "Unknown draw attribute! Not registered in info map.");
            assert(a_UploadData.normalMatrices != nullptr && "DrawAttribute enabled, but corresponding data is not provided!");
            normalMatrixEnabled = true;
            elementSize += info->second.size;   //No padding for Mat4.
        }

        assert(elementSize != 0 && "Trying to upload DrawAttribute per isntance data while no attributes are enabled!");

        const std::uintptr_t alignment = 16u;   //Upload data exists out of mat4 and vec3s. This means that alignment is always that of vec4. Padding only happens for vec3.
        const std::uintptr_t startPadding = (alignment - (a_Offset & (alignment - 1))) & (alignment - 1);
        const std::uintptr_t totalElementSize = elementSize * a_UploadData.drawData->instanceCount;


        //The size of the newly added data when padded.
        const std::uintptr_t totalSizeWithOffset = startPadding + (a_UploadData.drawData->instanceCount * elementSize);

        //The total size required with the new data.
        const auto endOffset = a_Offset + totalSizeWithOffset;

        //Resize buffer if needed and enabled.
        if (endOffset > m_Settings.size)
        {
            //Resize to double the size. Create a new buffer and move data into it.
            if (m_Settings.resizeWhenFull)
            {
                //Keep doubling till it fits.
                std::uint32_t newSize = m_Settings.size;
                while (newSize < endOffset)
                {
                    newSize *= 2;
                }

                //Resize and copy the data.
                Resize(newSize, true);
            }
            //No auto resizing allowed so crash the program.
            else
            {
                throw std::exception("Gpu Buffer size limit reached. Assign more space to the buffer by increasing setting.size or enable auto resizing.");
            }
        }

        //Bind the buffer and write to it.
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_Ssbo);

        constexpr auto mat4Size = static_cast<std::uintptr_t>(sizeof(glm::mat4));

        //If only a single mat4 is passed, I can skip the padding and interleaving.
        //This optimization only works if there is no interleaving and padding.
        if(elementSize == mat4Size)
        {
            glm::mat4* ptr = normalMatrixEnabled ? a_UploadData.normalMatrices : a_UploadData.transforms;

            //Upload the padded data to the GPU, directly from the passed pointer since there is no interleaving.
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, a_Offset + startPadding, totalSizeWithOffset, &ptr[0]);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        }
        else
        {
            //Reserve enough memory to temporarily store the data and add padding.
            std::vector<char> paddedData;
            paddedData.resize(totalSizeWithOffset);

            //Add each element to the buffer with the required offset and position/padding.
            for (std::uint32_t i = 0; i < a_UploadData.drawData->instanceCount; ++i)
            {
                const auto start = startPadding + (i * elementSize);
                std::uint32_t offset = 0;   //Offset from start where to place elements.

                if (transformEnabled)
                {
                    const auto pos = start + offset;
                    memcpy(static_cast<void*>(&paddedData[pos]), static_cast<void*>(&a_UploadData.transforms[i]), mat4Size);
                    offset += mat4Size;
                }
                if (normalMatrixEnabled)
                {
                    const auto pos = start + offset;
                    memcpy(static_cast<void*>(&paddedData[pos]), static_cast<void*>(&a_UploadData.normalMatrices[i]), mat4Size);
                    offset += mat4Size;
                }
            }

            //Upload the padded data to the GPU.
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, a_Offset, totalSizeWithOffset, &paddedData[0]);

            //Unbind the buffer.
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        }

        //Create a view containing the buffer offsets.
        const auto view = GpuBufferView(a_Offset + startPadding, totalElementSize, elementSize);
        const auto selfPtr = std::static_pointer_cast<GpuBuffer>(shared_from_this());

        //Fill in the data in the DrawData provided.
        a_UploadData.drawData->transformData.dataRange = view;
        a_UploadData.drawData->transformData.dataBuffer = selfPtr;

        return view;
    }

    GpuBufferView GpuBuffer_GL::WriteData(std::uint32_t a_Offset, const GlobalUploadData& a_UploadData)
    {
        assert(a_UploadData.drawData != nullptr && "DrawData cannot be nullptr.");

        //The size of each element containing one of each enabled draw attribute.
        //This is correctly padded by default because each individual datatype is padded.
        std::uint32_t elementSize = 0;

        //Check which drawAttributes are enabled that are single data.
        bool uvModEnabled = false;

        if (a_UploadData.drawData->attributes.IsAttributeEnabled(DrawAttribute::UV_MODIFIER))
        {
            const auto info = DRAW_ATTRIBUTE_INFO.find(DrawAttribute::UV_MODIFIER);
            assert(info != DRAW_ATTRIBUTE_INFO.end() && "Unknown draw attribute! Not registered in info map.");
            uvModEnabled = true;
            elementSize += info->second.size;   //No padding for Mat4.
        }

        assert(elementSize != 0 && "Trying to upload DrawAttribute per isntance data while no attributes are enabled!");

        const std::uintptr_t alignment = 16u;   //Upload data exists out of mat4 and vec3s. This means that alignment is always that of vec4. Padding only happens for vec3.
        const std::uintptr_t startPadding = (alignment - (a_Offset & (alignment - 1))) & (alignment - 1);
        const std::uintptr_t totalElementSize = elementSize * a_UploadData.drawData->instanceCount;


        //The size of the newly added data when padded.
        const std::uintptr_t totalSizeWithOffset = startPadding + (a_UploadData.drawData->instanceCount * elementSize);

        //The total size required with the new data.
        const auto endOffset = a_Offset + totalSizeWithOffset;

        //Resize buffer if needed and enabled.
        if (endOffset > m_Settings.size)
        {
            //Resize to double the size. Create a new buffer and move data into it.
            if (m_Settings.resizeWhenFull)
            {
                //Keep doubling till it fits.
                std::uint32_t newSize = m_Settings.size;
                while (newSize < endOffset)
                {
                    newSize *= 2;
                }

                //Resize and copy the data.
                Resize(newSize, true);
            }
            //No auto resizing allowed so crash the program.
            else
            {
                throw std::exception("Gpu Buffer size limit reached. Assign more space to the buffer by increasing setting.size or enable auto resizing.");
            }
        }

        /*
         * This is a single vec4 aligned buffer so no padding needed. Can just upload straight from the passed pointer.
         * I only have to add the startPadding to the offset where it is uploaded to the buffer manually.
         */

        //Bind the buffer and write to it.
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_Ssbo);

        //Upload the padded data to the GPU.
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, a_Offset + startPadding, totalElementSize, &a_UploadData.uvModifiers[0]);

        //Unbind the buffer.
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        //Create a view containing the buffer offsets.
        auto view = GpuBufferView(a_Offset + startPadding, totalElementSize, elementSize);
        auto selfPtr = std::static_pointer_cast<GpuBuffer>(shared_from_this());

        //Fill in the data in the DrawData provided.
        a_UploadData.drawData->transformData.dataRange = view;
        a_UploadData.drawData->transformData.dataBuffer = selfPtr;

        return view;
    }

    GpuBufferView GpuBuffer_GL::WriteData(std::uint32_t a_Offset, const LightUploadData& a_UploadData)
    {
        assert(a_UploadData.lightData != nullptr && "Cannot upload light data with nullptr LightData object!");

        std::vector<PointLightData> pointData;
        pointData.reserve(8);

        std::vector<PointLightData> pointShadowData;
        pointShadowData.reserve(8);

        std::vector<SpotLightData> spotData;
        spotData.reserve(8);

        std::vector<SpotLightData> spotShadowData;
        spotShadowData.reserve(8);

        std::vector<DirectionalLightData> dirData;
        dirData.reserve(8);

        std::vector<DirectionalLightData> dirShadowData;
        dirShadowData.reserve(8);

        

        if(a_UploadData.point.lights != nullptr)
        {
            for(std::uint32_t i = 0u; i < a_UploadData.point.count; ++i)
            {
                assert(a_UploadData.point.lights[i].get() != nullptr && "Light can't be nullptr.");
                auto data = a_UploadData.point.lights[i]->GetData();

                //No shadow
                if(data.positionShadowMapIndex.w < 0)
                {
                    pointData.push_back(data);
                }
                else
                {
                    pointShadowData.push_back(data);
                }
            }
        }

        if (a_UploadData.spot.lights != nullptr)
        {
            for (std::uint32_t i = 0u; i < a_UploadData.spot.count; ++i)
            {
                assert(a_UploadData.spot.lights[i].get() != nullptr && "Light can't be nullptr.");
                auto data = a_UploadData.spot.lights[i]->GetData();

                //No shadow
                if (data.positionShadowMapIndex.w < 0)
                {
                    spotData.push_back(data);
                }
                else
                {
                    spotShadowData.push_back(data);
                }
            }
        }

        if (a_UploadData.directional.lights != nullptr)
        {
            for (std::uint32_t i = 0u; i < a_UploadData.directional.count; ++i)
            {
                assert(a_UploadData.directional.lights[i].get() != nullptr && "Light can't be nullptr.");
                auto data = a_UploadData.directional.lights[i]->GetData();

                //No shadow
                if (data.directionShadowMapIndex.w < 0)
                {
                    dirData.push_back(data);
                }
                else
                {
                    dirShadowData.push_back(data);
                }
            }
        }

        //Size of the buffers;
        std::uintptr_t pointSize = sizeof(PointLightData) * pointData.size();
        std::uintptr_t pointShadowSize = sizeof(PointLightData) * pointShadowData.size();

        std::uintptr_t spotSize = sizeof(SpotLightData) * spotData.size();
        std::uintptr_t spotShadowSize = sizeof(SpotLightData) * spotShadowData.size();

        std::uintptr_t dirSize = sizeof(DirectionalLightData) * dirData.size();
        std::uintptr_t dirShadowSize = sizeof(DirectionalLightData) * dirShadowData.size();

        //Start of the buffers.
        std::uintptr_t pointStart = 0;
        std::uintptr_t pointShadowStart = pointStart + pointSize;

        std::uintptr_t spotStart = pointShadowStart + pointShadowSize;
        std::uintptr_t spotShadowStart = spotStart + spotSize;

        std::uintptr_t dirStart = spotShadowStart + spotShadowSize;
        std::uintptr_t dirShadowStart = dirStart + dirSize;

        std::size_t bufferSize = dirShadowStart + dirShadowSize;

        //Construct a buffer containing all the data.
        std::vector<char> buffer;
        buffer.resize(bufferSize);

        /*
         * Copy to buffer, then send off to GPU.
         */

        if (pointSize > 0) memcpy(&buffer[0] + pointStart, &pointData[0], pointSize);
        if (pointShadowSize > 0) memcpy(&buffer[0] + pointShadowStart, &pointShadowData[0], pointShadowSize);

        if (spotSize > 0) memcpy(&buffer[0] + spotStart, &spotData[0], spotSize);
        if (spotShadowSize > 0) memcpy(&buffer[0] + spotShadowStart, &spotShadowData[0], spotShadowSize);

        if (dirSize > 0) memcpy(&buffer[0] + dirStart, &dirData[0], dirSize);
        if (dirShadowSize > 0) memcpy(&buffer[0] + dirShadowStart, &dirShadowData[0], dirShadowSize);





        //Alignment for std430 is equal to the largest member size.
        const std::uintptr_t alignment = 16u;
        const std::uintptr_t startPadding = (alignment - (a_Offset & (alignment - 1))) & (alignment - 1);

        //The total size required with the new data.
        const auto totalSize = a_Offset + startPadding + bufferSize;
        auto offset = a_Offset + startPadding;

        //Overwriting buffer limits.
        if (totalSize > m_Settings.size)
        {
            //Resize to double the size. Create a new buffer and move data into it.
            if (m_Settings.resizeWhenFull)
            {
                //Keep doubling till it fits.
                std::uint32_t newSize = m_Settings.size;
                while (newSize < totalSize)
                {
                    newSize *= 2;
                }

                //Resize and copy the data.
                Resize(newSize, true);
            }
            //No auto resizing allowed so crash the program.
            else
            {
                throw std::exception("Gpu Buffer size limit reached. Assign more space to the buffer by increasing setting.size or enable auto resizing.");
            }
        }

        //Bind the buffer and write to it.
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_Ssbo);

        //Upload the padded data to the GPU.
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, bufferSize, &buffer[0]);

        //Unbind the buffer.
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


        /*
         * Finally fill in the LightData object.
         */

        auto selfPtr = std::static_pointer_cast<GpuBuffer>(shared_from_this());
        auto& lightData = *a_UploadData.lightData;

        //Point
        lightData.pointLights.dataBuffer = selfPtr;
        lightData.pointLights.dataRange = GpuBufferView(offset + pointStart, pointSize + pointShadowSize, sizeof(PointLightData));
        lightData.pointLights.count = static_cast<std::uint32_t>(pointData.size());
        lightData.pointLights.shadowCount = static_cast<std::uint32_t>(pointShadowData.size());

        //Spot
        lightData.spotLights.dataBuffer = selfPtr;
        lightData.spotLights.dataRange = GpuBufferView(offset + spotStart, spotSize + spotShadowSize, sizeof(SpotLightData));
        lightData.spotLights.count = static_cast<std::uint32_t>(spotData.size());
        lightData.spotLights.shadowCount = static_cast<std::uint32_t>(spotShadowData.size());

        //Dir
        lightData.directionalLights.dataBuffer = selfPtr;
        lightData.directionalLights.dataRange = GpuBufferView(offset + dirStart, dirSize + dirShadowSize, sizeof(DirectionalLightData));
        lightData.directionalLights.count = static_cast<std::uint32_t>(dirData.size());
        lightData.directionalLights.shadowCount = static_cast<std::uint32_t>(dirShadowData.size());

        //Return a GpuBufferView that contains the entire light area of the buffer so more data can be appended behind it.
        return GpuBufferView(offset, bufferSize, 1);
    }
}
