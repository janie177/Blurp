#include "opengl/MaterialBatch_GL.h"
#include "BlurpEngine.h"
#include "RenderResourceManager.h"

namespace blurp
{
    bool MaterialBatch_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {

        /*
         * Below an array texture is created for all the materials that use a texture in this material batch.
         */

        //Ensure that the settings are valid.
        assert(m_Settings.GetMask() != 0 && "MaterialBatch needs at least one attribute enabled!");
        assert(m_Settings.materialCount > 0 && m_Settings.textureSettings.dimensions.x > 0 && m_Settings.textureSettings.dimensions.y > 0 && "Material count, width and height in a batch need to be at least 1.");

        //If at least one texture is enabled, make sure that the provided data is not nullptr.
        for(auto& attrib : TEXTURE_MATERIAL_ATTRIBUTES)
        {
            if (m_Settings.IsAttributeEnabled(attrib))
            {
                m_HasTexture = true;
                break;
            }
        }

        assert(m_HasTexture == (m_Settings.textureData != nullptr) && "Material batch that has texture enabled requires textureData to be provided!");

        //Upload the texture data to a texture array.
        if(m_HasTexture)
        {
            //Ensure that a texture count was provided.
            assert(m_Settings.textureCount != 0 && "Please provide how many textures are in the texture array per material!");

            TextureSettings tSettings;
            tSettings.textureType = TextureType::TEXTURE_2D_ARRAY;
            tSettings.pixelFormat = PixelFormat::RGB;
            tSettings.dataType = m_Settings.textureSettings.dataType;
            tSettings.wrapMode = m_Settings.textureSettings.wrapMode;
            tSettings.dimensions = { m_Settings.textureSettings.dimensions.x, m_Settings.textureSettings.dimensions.y, m_Settings.materialCount * m_Settings.textureCount};    //Total layers is the amount of materials times the amount of textures per material.
            tSettings.generateMipMaps = m_Settings.textureSettings.generateMipMaps;
            tSettings.numMipMaps = m_Settings.textureSettings.numMipMaps;
            tSettings.magFilter = m_Settings.textureSettings.magFilter;
            tSettings.minFilter = m_Settings.textureSettings.minFilter;
            tSettings.memoryAccess = AccessMode::READ_ONLY;
            tSettings.memoryUsage = MemoryUsage::GPU;
            tSettings.texture2DArray.data = m_Settings.textureData;

            m_ArrayTexture = a_BlurpEngine.GetResourceManager().CreateTexture(tSettings);
        }


        /*
         * Below a UBO is created for all materials constant data.
         * Padding and stride is calculated to follow the std140 layout rules. 4 byte alignment is required for structs.
         */


        //Calculate the size of the constant data buffer.
        std::uint32_t stride = 0;
        constexpr std::uint32_t numAttribs = sizeof(CONSTANT_MATERIAL_ATTRIBUTES) / sizeof(CONSTANT_MATERIAL_ATTRIBUTES[0]);

        //Array to store the padding for each element in.
        std::uint32_t paddedSize[numAttribs];
        std::uint32_t numElements[numAttribs];
        float* attribData[numAttribs];

        //NOTE: This only works if all vec3/vec4s are first, followed by all loose floats.
        int i = 0;
        for(auto& attrib : CONSTANT_MATERIAL_ATTRIBUTES)
        {
            if (m_Settings.IsAttributeEnabled(attrib))
            {
                auto found = MATERIAL_ATTRIBUTE_INFO.find(attrib);
                assert(found != MATERIAL_ATTRIBUTE_INFO.end());

                const auto elementCount = found->second.numElements;
                auto attribStride = 0u;

                attribStride += elementCount;

                //If numElements > 1, add a stride to align to 4.
                //This works because float and vec2 don't need padding. Vec3 does need padding.
                if(elementCount > 2)
                {
                    attribStride += (4 - elementCount);
                }

                //Add onto the total stride.
                stride += attribStride;

                //Add the total padded size of this attribute for later use.
                paddedSize[i] = attribStride;
                numElements[i] = elementCount;

                switch (attrib)
                {
                case MaterialAttribute::DIFFUSE_CONSTANT_VALUE:
                    attribData[i] = m_Settings.constantData.diffuseConstantData;
                    break;
                case MaterialAttribute::EMISSIVE_CONSTANT_VALUE:
                    attribData[i] = m_Settings.constantData.emissiveConstantData;
                    break;
                case MaterialAttribute::METALLIC_CONSTANT_VALUE:
                    attribData[i] = m_Settings.constantData.metallicConstantData;
                    break;
                case MaterialAttribute::ROUGHNESS_CONSTANT_VALUE:
                    attribData[i] = m_Settings.constantData.roughnessConstantData;
                    break;
                case MaterialAttribute::ALPHA_CONSTANT_VALUE:
                    attribData[i] = m_Settings.constantData.alphaConstantData;
                    break;
                default:
                    throw std::exception("Oops I forgot to add a new material attribute constant data to the OpenGL implementation.");
                    break;
                }
            }
            ++i;
        }

        //If the current stride is not a multiple of 4, then make it so.
        const auto leftOver = stride % 4u;
        if(leftOver != 0)
        {
            stride += (4u - leftOver);
        }

        //Create the contiguous array of floats.
        std::vector<float> constantData;
        constantData.resize(static_cast<std::size_t>(stride) * m_Settings.materialCount);

        //Loop over each attribute, and then put the data in the right spot for each including padding.
        auto accumulatedStride = 0u;
        i = 0;
        for (auto& attrib : CONSTANT_MATERIAL_ATTRIBUTES)
        {
            if (m_Settings.IsAttributeEnabled(attrib))
            {
                auto attribStride = paddedSize[i];
                auto elementCount = numElements[i];

                //Add each materials data in the right index.
                for (auto matIndex = 0u; matIndex < m_Settings.materialCount; ++matIndex)
                {
                    auto startIndex = (matIndex * stride) + accumulatedStride;
                    for(unsigned int elementIndex = 0; elementIndex <= elementCount; ++elementIndex)
                    {
                        auto dataPtr = attribData[i];
                        assert(dataPtr != nullptr && "MaterialBatch that has constant data enabled had nullptr provided for that data!");
                        constantData[static_cast<std::size_t>(startIndex) + elementIndex] = dataPtr[(matIndex * elementCount) + elementIndex];
                    }
                }

                //Append the current stride for the next element to use.
                accumulatedStride += attribStride;
            }
            ++i;
        }

        //Create the UBO if there is data present. Upload the padded data to that UBO.
        if(stride != 0)
        {
            m_HasUbo = true;

            glGenBuffers(1, &m_Ubo);
            glBindBuffer(GL_UNIFORM_BUFFER, m_Ubo);
            glBufferData(GL_UNIFORM_BUFFER, constantData.size() * sizeof(float), &constantData[0], GL_STATIC_DRAW); // allocate 152 bytes of memory
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }


        return true;
    }

    bool MaterialBatch_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        m_ArrayTexture = nullptr;
        glDeleteBuffers(1, &m_Ubo);
        return true;
    }
}
