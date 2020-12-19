#pragma once
#include "Data.h"

/*
 * This file contains all structs used to describe a resource before creation.
 * These settings objects are passed to the render device to create the appropriate resources.
 */

namespace blurp
{
    /*
     * TextureSettings describes a texture resource.
     * It is passed to the ResourceManager to create a Texture resource instance.
     *
     * A const char pointer is passed to the start of the texture data.
     * The size of this pointer should be the amount of channels (PixelFormat) * DataType * width * height * depth.
     */
    struct TextureSettings
    {
        TextureSettings()
        {
            //Default values
            dimensions = { 2, 2, 1 };
            pixelFormat = PixelFormat::RGBA;
            dataType = DataType::FLOAT;
            textureType = TextureType::TEXTURE_2D;
            generateMipMaps = false;
            minFilter = MinFilterType::LINEAR;
            magFilter = MagFilterType::LINEAR;
            wrapMode = WrapMode::REPEAT;
            memoryUsage = MemoryUsage::GPU;
            memoryAccess = AccessMode::READ_ONLY;
            numMipMaps = 0;

            textureCubeMap.data[0] = nullptr;
            textureCubeMap.data[1] = nullptr;
            textureCubeMap.data[2] = nullptr;
            textureCubeMap.data[3] = nullptr;
            textureCubeMap.data[4] = nullptr;
            textureCubeMap.data[5] = nullptr;
        }

        //The dimensions of this texture. X and Y are the texture dimensions while Z is the depth.
        //For cubemap array textures, depth has be divisible by 6.
        //Cubemap textures require X and Y to be equal as well.
        glm::vec<3, std::uint32_t> dimensions;

        //The pixel format for this texture. This determines how many channels this texture has and how they are used.
        PixelFormat pixelFormat;

        //The data type for this texture. This determines the size of each channel in the pixel format.
        DataType dataType;

        //The type of texture.
        TextureType textureType;

        //Whether to generate mip maps or not.
        bool generateMipMaps;

        //The amount of mipmaps to generate.
        //Leave this 0 to automatically determine.
        std::uint16_t numMipMaps;

        //How should this texture behave when scaled down?
        MinFilterType minFilter;

        //How should this texture behave when scaled up?
        MagFilterType magFilter;

        //How should this texture behave when sampled outside of the 0-1 UVW coordinate range.
        WrapMode wrapMode;

        //How will this texture be accessed? If READ, can not be set as render target.
        AccessMode memoryAccess;

        //How will the memory be used? GPU means only the GPU reads and writes from this texture.
        //CPU modes imply that the CPU either often reads and writes to this texture.
        MemoryUsage memoryUsage;

        //Raw texture data pointer. Leave this as nullptr to not upload any data.
        union
        {
            //Texture1D
            struct
            {
                const void* data;
            } texture1D;

            //Texture2D
            struct
            {
                const void* data;

            } texture2D;

            //Texture3D
            struct
            {
                //The amount of layers in this 3D texture
                const void* data;
            } texture3D;

            //TextureCubeMap
            struct
            {
                const void* data[6];
            } textureCubeMap;

            //Texture2DArray
            struct
            {
                //The amount of layers in this texture array.
                const void* data;
            } texture2DArray;

            //Texture2DArray
            struct
            {
                //The amount of layers in this texture array.
                const void* data;
            } textureCubeMapArray;

        };

    };

    struct RenderTargetSettings
    {
        RenderTargetSettings()
        {
            //Default values.
            viewPort = { 0.f, 0.f, 2.f, 2.f };
            scissorRect = { 0.f, 0.f, 99999, 99999 };
            clearColor = { 1.f, 1.f, 1.f, 1.f };
            allowAttachments = true;
        }
        //When true, attachment switching is allowed at runtime.
        bool allowAttachments;

        //The dimensions of the viewport and scissor rect.
        glm::vec<4, std::uint32_t> viewPort;
        glm::vec<4, std::uint32_t> scissorRect;
        glm::vec4 clearColor;

        //Default attachments. Leave null for none.
        std::shared_ptr<Texture> defaultColorAttachment;
        std::shared_ptr<Texture> defaultDepthStencilAttachment;
    };

    struct SwapChainSettings
    {
        SwapChainSettings()
        {
            //Default values.
            numBuffers = 2;
            vsync = false;
            renderTargetSettings.viewPort = { 0.f, 0.f, 2.f, 2.f };
            renderTargetSettings.scissorRect = { 0.f, 0.f, 99999, 99999 };
            renderTargetSettings.clearColor = { 1.f, 1.f, 1.f, 1.f };

            renderTargetSettings.colorChannels = 4; //RGBA by default.
            renderTargetSettings.channelBits = 8;   //BYTE per channel.
            renderTargetSettings.depthBits = 24;    //24 bit depth combined with 8 bit stencil.
            renderTargetSettings.stencilBits = 8;
        }

        //Buffer format.
        struct
        {
            //The amount of color channels present.
            std::uint16_t colorChannels;

            //The amount of bits per channel.
            std::uint16_t channelBits;

            //The amount of bits used for the depth buffer.
            std::uint16_t depthBits;

            //The amount of bits used for the stencil buffer.
            std::uint16_t stencilBits;

            //The viewport dimensions: originx, originy, width, height.
            glm::vec<4, std::uint32_t> viewPort;

            //The scissorrect dimensions: originx, originy, width, height.
            glm::vec<4, std::uint32_t> scissorRect;

            //The clear color: r, g, b, a.
            glm::vec4 clearColor;

        } renderTargetSettings;

        //Amount of buffers. Minimum of two required.
        std::uint16_t numBuffers;

        //Enable or disable vsync.
        bool vsync;
    };


    struct WindowSettings
    {
        WindowSettings()
        {
            //Default settings.
            dimensions = { 800, 600 };
            type = WindowType::WINDOW_WIN32;
            name = "Blurp Window";
            flags = WindowFlags::NONE;

            //Make sure the internal render target has the same dimensions.
            swapChainSettings.renderTargetSettings.viewPort = { 0.f, 0.f, dimensions};
        }

        //Window information
        glm::vec2 dimensions;
        WindowType type;
        std::string name;
        WindowFlags flags;

        //Swapchain information
        SwapChainSettings swapChainSettings;
    };

    struct CameraSettings
    {
        CameraSettings()
        {
            projectionMode = ProjectionMode::PERSPECTIVE;
            fov = 90.f;
            nearPlane = 0.001f;
            farPlane = 9999.f;
            width = 800.f;
            height = 400.f;
        }

        //The projection mode.
        ProjectionMode projectionMode;

        //The field of view for this camera in degrees.
        std::float_t fov;

        //How close to the camera do objects have to be to be cut off.
        std::float_t nearPlane;

        //How far from the camera do objects have to be to be cut off.
        std::float_t farPlane;

        //Width of the camera resolution.
        std::float_t width;

        //Height of the camera resolution.
        std::float_t height;
    };

    struct BlurpSettings
    {
        BlurpSettings()
        {
            //Default settings.
            graphicsAPI = GraphicsAPI::OPENGL;
            shadersPath = "/shaders/";
        }

        //Settings for the window. To not create a window, set type to NONE.
        WindowSettings windowSettings;

        //Which graphics API is used?
        GraphicsAPI graphicsAPI;

        //Path to the shaders directory.
        std::string shadersPath;
    };

    struct VertexSettings
    {
        VertexSettings()
        {
            //Nothing enabled by default.
            m_Mask = static_cast<VertexAttribute>(0);
        }

        /*
         * Enable a vertex attribute with the given offset and stride in bytes.
         * The given vertex attribute has to be a single attribute without any masking.
         * The instance divisor determines if instancing is enabled and per how many draws it is updated.
         */
        void EnableAttribute(VertexAttribute a_Attribute, std::uint32_t a_Offset, std::uint32_t a_Stride, std::uint16_t a_InstanceDivisor)
        {
            assert(static_cast<std::uint16_t>(a_Attribute) != 0 && (static_cast<std::uint16_t>(a_Attribute) & (static_cast<std::uint16_t>(a_Attribute) - 1)) == 0);
            m_Mask = m_Mask | a_Attribute;

            //Get the data index and set it.
            auto& data = m_Data[static_cast<std::uint16_t>(std::floor(std::log(static_cast<std::uint16_t>(a_Attribute) | 0) / std::log(2)))];
            data.byteOffset = a_Offset;
            data.byteStride = a_Stride;
            data.instanceDivisor = a_InstanceDivisor;
        }

        /*
         * Disable a vertex attribute.
         */
        void DisableAttribute(VertexAttribute a_Attribute)
        {
            m_Mask = m_Mask & ~a_Attribute;
        }

        /*
         * Get the masked value of the vertex attributes that are enabled.
         */
        VertexAttribute GetMask() const
        {
            return m_Mask;
        }

        /*
         * See if the given attributes are enabled.
         */
        bool IsEnabled(VertexAttribute a_Attribute) const
        {
            return (m_Mask & a_Attribute) == a_Attribute;
        }

        /*
         * Get the configured data for the given attribute.
         * The given vertex attribute has to be a single attribute without any masking.
         */
        VertexAttributeData GetAttributeData(VertexAttribute a_Attribute)
        {
            assert(static_cast<std::uint16_t>(a_Attribute) != 0 && (static_cast<std::uint16_t>(a_Attribute) & (static_cast<std::uint16_t>(a_Attribute) - 1)) == 0);
            return m_Data[static_cast<std::uint16_t>(std::floor(std::log(static_cast<std::uint16_t>(a_Attribute) | 0) / std::log(2)))];
        }

        /*
         * Get static information about the given vertex type like the name, size and type.
         * The vertex attribute provided has to be a single attribute that was not masked.
         */
        static VertexAttributeInfo GetVertexAttributeInfo(VertexAttribute a_Attribute)
        {
            assert(static_cast<std::uint16_t>(a_Attribute) != 0 && (static_cast<std::uint16_t>(a_Attribute) & (static_cast<std::uint16_t>(a_Attribute) - 1)) == 0);
            const auto found =  VERTEX_ATTRIBUTE_INFO.find(a_Attribute);
            if(found != VERTEX_ATTRIBUTE_INFO.end())
            {
                return found->second;
            }
            throw std::exception("Fatal error: This should never happen. Were more vertex attributes added but not to VERTEX_ATTRIBUTE_INFO?");
        }

    private:
        VertexAttribute m_Mask;
        VertexAttributeData m_Data[NUM_VERTEX_ATRRIBS];

        inline static const std::unordered_map<VertexAttribute, VertexAttributeInfo> VERTEX_ATTRIBUTE_INFO = {
            {VertexAttribute::POSITION_3D, {3, DataType::FLOAT, "va_Position3D", "VA_POS3D_DEF", "VA_POS3D_LOCATION_DEF"}},
            {VertexAttribute::UV_COORDS, {2, DataType::FLOAT, "va_UVCoords", "VA_UVCOORD_DEF", "VA_UVCOORD_LOCATION_DEF"}},
            {VertexAttribute::NORMAL, {3, DataType::FLOAT, "va_Normal", "VA_NORMAL_DEF", "VA_NORMAL_LOCATION_DEF"}},
            {VertexAttribute::COLOR, {3, DataType::FLOAT, "va_Color", "VA_COLOR_DEF", "VA_COLOR_LOCATION_DEF"}},
            {VertexAttribute::TANGENT, {3, DataType::FLOAT, "va_Tangent", "VA_TANGENT_DEF", "VA_TANGENT_LOCATION_DEF"}},
            {VertexAttribute::BI_TANGENT, {3, DataType::FLOAT, "va_BiTangent", "VA_BITANGENT_DEF", "VA_BITANGENT_LOCATION_DEF"}},
            {VertexAttribute::BONE_INDEX, {3, DataType::UINT, "va_BoneIndex", "VA_BONEINDEX_DEF", "VA_BONEINDEX_LOCATION_DEF"}},
            {VertexAttribute::BONE_WEIGHT, {3, DataType::FLOAT, "va_BoneWeight", "VA_BONEWEIGHT_DEF", "VA_BONEWEIGHT_LOCATION_DEF"}},
            {VertexAttribute::MATERIAL_ID, {1, DataType::FLOAT, "va_MaterialID", "VA_MATERIALID_DEF", "VA_MATERIALID_LOCATION_DEF"}},
            {VertexAttribute::UV_MODIFIER_ID, {1, DataType::FLOAT, "va_UvModifierID", "VA_UVMODIFIERID_DEF", "VA_UVMODIFIERID_LOCATION_DEF"}},
            {VertexAttribute::MATRIX, {16, DataType::FLOAT, "va_Matrix", "VA_MATRIX_DEF", "VA_MATRIX_LOCATION_DEF"}},
            {VertexAttribute::ITMATRIX, {16, DataType::FLOAT, "va_ITMatrix", "VA_ITMATRIX_DEF", "VA_ITMATRIX_LOCATION_DEF"}},
        };
    };

    struct MeshSettings
    {
        MeshSettings()
        {
            usage = MemoryUsage::GPU;
            access = AccessMode::READ_ONLY;
            vertexData = nullptr;
            indexData = nullptr;
            numIndices = 0;
            indexDataType = DataType::SHORT;
            vertexDataSizeBytes = 0;
            instanceCount = 1;
        }

        //Which vertex attributes are enabled for this mesh?
        VertexSettings vertexSettings;

        /*
         * How will the memory be used?
         * CPU_R means the CPU will often read from this memory.
         * CPU_W means the CPU will often write to this memory.
         * GPU means the CPU does not often interact with this memory.
         */
        MemoryUsage usage;

        /*
         * How will this mesh be accessed? READ_ONLY means the mesh will never be modified after initially uploading data.
         * READ_WRITE means the data can be altered.
         */
        AccessMode access;

        //The raw data of this mesh in the provided vertex format.
        //Offsets and strides between attributes are configured in vertexSettings.
        const void* vertexData;

        //The size of the total vertexData in bytes.
        std::uint32_t vertexDataSizeBytes;

        //Pointer to the raw index data for the index buffer.
        const void* indexData;

        //Size of the index buffer.
        std::uint32_t numIndices;

        //Data type for the index buffer
        DataType indexDataType;

        //Instance count (how many to draw).
        //One by default. Only enable if there is instanced vertex attributes.
        std::uint32_t instanceCount;
    };

    struct LightSettings
    {
        LightSettings()
        {
            //Default values.
            type = LightType::LIGHT_AMBIENT;
            color = { 1.f, 1.f, 1.f };
            intensity = 1.f;
            shadowMapIndex = -1;

            spotLight.position = { 0.f, 0.f, 0.f };
            spotLight.direction = { 0.f, -1.f, 0.f };
            spotLight.angle = 45.f;

            pointLight.position = { 0.f, 0.f, 0.f };

            directionalLight.direction = { 0.f, -1.f, 0.f };
        }

        //Type of the light.
        LightType type;

        //Color of the light.
        glm::vec3 color;

        //Intensity of the light.
        float intensity;

        //The index in the shadowmap to use. When set to -1, no shadows are cast.
        //IDs for directional and positions lights are separately tracked.
        std::int32_t shadowMapIndex;

        struct
        {
            //Where is the light located.
            glm::vec3 position;

            //Where does the light point.
            glm::vec3 direction;

            //Angle of the light cone.
            float angle;
        } spotLight;

        struct
        {
            //Where is the light located.
            glm::vec3 position;
        } pointLight;

        struct
        {
            //Where does the light point.
            glm::vec3 direction;
        } directionalLight;
    };

    struct MaterialMask
    {
    public:
        MaterialMask()
        {
            m_Mask = static_cast<MaterialAttribute>(0);
        }

        /*
         * Enable specific attributes for this material.
         */
        void EnableAttribute(MaterialAttribute a_Attribute);

        /*
         * Disable a specific attribute for this material.
         */
        void DisableAttribute(MaterialAttribute a_Attribute);

        /*
         * Set the bitmask for this materials enabled attributes.
         */
        void SetMask(std::uint16_t a_Mask);

        /*
         * Returns true if the provided attribute is enabled.
         */
        bool IsAttributeEnabled(MaterialAttribute a_Attribute) const;

        /*
         * Get the unsigned 16 bit int mask of all enabled attributes.
         */
        std::uint16_t GetMask() const;

    private:
        //Mask of enabled attributes.
        MaterialAttribute m_Mask;
    };

    /*
     * Object used to construct a Material.
     * To use, set constant values or textures.
     * Then enable the corresponding attribute by calling EnableAttribute.
     *
     * Some textures require a specific format to be compatible with the shader samplers.
     */
    struct MaterialSettings
    {
    public:
        MaterialSettings()
        {
            //Raw data stored in this material.
            m_DiffuseValue = {1.f, 1.f, 1.f};
            m_EmissiveValue = {0.8f, 0.3f, 0.9f};
            m_MetallicValue = 1.f;
            m_RoughnessValue = 1.f;
            m_AlphaValue = 1.f;
        }

        /*
         * Enable specific attributes for this material.
         */
        void EnableAttribute(MaterialAttribute a_Attribute);

        /*
         * Disable a specific attribute for this material.
         */
        void DisableAttribute(MaterialAttribute a_Attribute);

        /*
         * Returns true if the provided attribute is enabled.
         */
        bool IsAttributeEnabled(MaterialAttribute a_Attribute) const;

        /*
         * Set the bitmask for enabled attributes.
         */
        void SetMask(std::uint16_t a_Mask);

        /*
         * Get the unsigned 16 bit int mask of all enabled attributes.
         */
        std::uint16_t GetMask() const;

        /*
         * Set the texture to be used for the diffuse color of the material.
         * RGB format required.
         */
        void SetDiffuseTexture(const std::shared_ptr<Texture>& a_Texture);

        /*
         * Set the texture to be used for the emissive color of the material.
         * RGB format required.
         */
        void SetEmissiveTexture(const std::shared_ptr<Texture>& a_Texture);

        /*
         * Set the texture to be used for the normals of the material.
         * RGB format required.
         */
        void SetNormalTexture(const std::shared_ptr<Texture>& a_Texture);

        /*
         * Set the texture to be used for the height offset of the material and the occlusion.
         * R channel is used for the occlusion map.
         * G and B channels are used for the heightmap.
         * RGB format required.
         */
        void SetOHTexture(const std::shared_ptr<Texture>& a_Texture);

        /*
         * Set the texture to be used for the O/M/R/A of the material.
         * R = Metallic byte.
         * G = Roughness byte.
         * B = Alpha byte.
         *
         * RGB format required.
         */
        void SetMRATexture(const std::shared_ptr<Texture>& a_Texture);

        /*
         * Set the constant to be used as the diffuse color of the material.
         * Only values normalized between 0 and 1 are accepted.
         */
        void SetDiffuseConstant(const glm::vec3& a_Value);

        /*
         * Set the constant to be used as the emissive color of the material.
         * Only values normalized between 0 and 1 are accepted.
         */
        void SetEmissiveConstant(const glm::vec3& a_Value);

        /*
         * Set the constant to be used as the metallicness of the material.
         * Only values between 0 and 1 are accepted.
         */
        void SetMetallicConstant(float a_Value);

        /*
         * Set the constant to be used as the roughness of the material.
         * Only values between 0 and 1 are accepted.
         */
        void SetRoughnessConstant(float a_Value);

        /*
         * Set the constant to be used as the alpha of the material.
         * Only values between 0 and 1 are accepted.
         */
        void SetAlphaConstant(float a_Value);

        /*
         * Get the texture currently set as DiffuseTexture.
         */
        std::shared_ptr<Texture> GetDiffuseTexture() const;

        /*
         * Get the texture currently set as emissive texture.
         */
        std::shared_ptr<Texture> GetEmissiveTexture() const;

        /*
         * Get the texture currently set as normal texture.
         */
        std::shared_ptr<Texture> GetNormalTexture() const;

        /*
         * Get the texture currently set as occlusion and height of the material.
         */
        std::shared_ptr<Texture> GetOHTexture() const;

        /*
         * Get the texture currently set for the Metallic, Roughness and Alpha of the material.
         */
        std::shared_ptr<Texture> GetMRATexture() const;

        /*
         * Get the constant value currently set as the diffuse color.
         */
        glm::vec3 GetDiffuseValue() const;

        /*
         * Get the constant value currently set as the emissive color.
         */
        glm::vec3 GetEmissiveValue() const;

        /*
         * Get the constant value currently set as the metallicness.
         */
        float GetMetallicValue() const;

        /*
         * Get the constant value currently set as the roughness.
         */
        float GetRoughnessValue() const;

        /*
         * Get the constanct value currently set as the alpha of the material.
         */
        float GetAlphaValue() const;
        

    private:
        MaterialMask m_Mask;

        //Textures stored in this material. Each texture is RGB format.
        std::shared_ptr<Texture> m_DiffuseTexture;
        std::shared_ptr<Texture> m_EmissiveTexture;
        std::shared_ptr<Texture> m_NormalTexture;
        std::shared_ptr<Texture> m_OcclusionHeightTexture;          //Contains R for occlusion, and GB for height.
        std::shared_ptr<Texture> m_MetallicRoughnessAlphaTexture;   //Contains R for metallic, G for roughness and B for alpha.

        //Raw data stored in this material.
        glm::vec3 m_DiffuseValue;
        glm::vec3 m_EmissiveValue;
        std::float_t m_MetallicValue;
        std::float_t m_RoughnessValue;
        std::float_t m_AlphaValue;
    };

    /*
     * Object used to construct a material batch.
     * Attributes for the materials can be enabled or disabled for all materials. Each material requires the same attributes.
     * All textures of all materials need to be the same dimensions and format.
     */
    struct MaterialBatchSettings
    {
    public:

        MaterialBatchSettings()
        {
            textureData = nullptr;

            constantData.diffuseConstantData = nullptr;
            constantData.emissiveConstantData = nullptr;
            constantData.metallicConstantData = nullptr;
            constantData.roughnessConstantData = nullptr;
            constantData.alphaConstantData = nullptr;

            materialCount = 0;
            textureCount = 0;
            textureSettings.dimensions = {0, 0};
            textureSettings.dataType = DataType::BYTE;
            textureSettings.generateMipMaps = true;
            textureSettings.numMipMaps = 0;
            textureSettings.minFilter = MinFilterType::MIPMAP_NEAREST;
            textureSettings.magFilter = MagFilterType::NEAREST;
            textureSettings.wrapMode = WrapMode::REPEAT; 
        }

        /*
         * Enable specific attributes for this material.
         */
        void EnableAttribute(MaterialAttribute a_Attribute);

        /*
         * Disable a specific attribute for this material.
         */
        void DisableAttribute(MaterialAttribute a_Attribute);

        /*
         * Returns true if the provided attribute is enabled.
         */
        bool IsAttributeEnabled(MaterialAttribute a_Attribute) const;

        /*
         * Set the mask for this material batch.
         */
        void SetMask(std::uint16_t a_Mask);

        /*
         * Get the unsigned 16 bit int mask of all enabled attributes.
         */
        std::uint16_t GetMask() const;

    public:
        //The amount of materials in this batch.
        std::uint32_t materialCount;

        //The amount of textures that are enabled in this batch PER MATERIAL. This is not equal to attributes. Diffuse, Emissive, OH, MRA all count as one texture each.
        std::uint32_t textureCount;

        /*
         * The texture settings for this material batch.
         */
        struct
        {
            //The dimensions of this texture.
            glm::vec<2, std::uint32_t> dimensions;

            //The data type for this texture. This determines the size of each channel in the pixel format.
            DataType dataType;

            //Whether to generate mip maps or not.
            bool generateMipMaps;

            //The amount of mipmaps to generate.
            //Leave this 0 to automatically determine.
            std::uint16_t numMipMaps;

            //How should this texture behave when scaled down?
            MinFilterType minFilter;

            //How should this texture behave when scaled up?
            MagFilterType magFilter;

            //How should this texture behave when sampled outside of the 0-1 UVW coordinate range.
            WrapMode wrapMode;
        } textureSettings;

        /*
         * A pointer to the start of the texture data in the correct format.
         * Each texture is provided in RGB format with the provided data type for each channel.
         * The data has to be ordered according to the MaterialAttribute enumeration.
         *
         * Example:
         * If DIFFUSE, NORMAL and METALLIC are enabled, they would appear in that order.
         * Diffuse and Normal would both be RGB format with each channel used.
         * Metallic would be an RGB texture that only uses the R channel with the other data unused but still uploaded.
         * Data{D{RGB...}, N{RGB...}, M{R--...}}.
         */
        void* textureData;

        //Array of float3. MaterialCount * 3 * float. Three floats per material.
        struct
        {
            //Arrays of RGB floats.
            float* diffuseConstantData;
            float* emissiveConstantData;

            //Array of floats. Size of MaterialCount. One float per material.
            float* metallicConstantData;
            float* roughnessConstantData;
            float* alphaConstantData;

        } constantData;


    private:
        MaterialMask m_Mask;
    };

    /*
     * Struct containing shader data.
     * The char pointers are only valid on construction, later usage is unsafe.
     * When a shader stage is not used, leave it as nullptr.
     */
    struct ShaderSettings
    {
        ShaderSettings()
        {
            //Default to nullptr to be safe.
            vertexShaderSource = nullptr;
            tessellationHullShaderSource = nullptr;
            tessellationDomainShaderSource = nullptr;
            geometryShaderSource = nullptr;
            fragmentShaderSource = nullptr;
            computeShaderSource = nullptr;

            type = ShaderType::GRAPHICS;
        }

        //Raw pointers to each shader stage. Nullptr if not used.
        const char* vertexShaderSource;
        const char* tessellationHullShaderSource;
        const char* tessellationDomainShaderSource;
        const char* geometryShaderSource;
        const char* fragmentShaderSource;

        //Compute shader source
        const char* computeShaderSource;

        //Strings to define in the shader before compiling.
        std::vector<std::string> preprocessorDefinitions;

        //The type of shader
        ShaderType type;
    };

    /*
     * Settings related to RenderPipelines.
     */
    struct PipelineSettings
    {
        PipelineSettings()
        {
            waitForGpu = true;
        }

        /*
         * When true, calling Execute() on a pipeline will stall the CPU until the GPU has finished executing.
         * Once execution has finished, all locked resources are automatically freed.
         */
        bool waitForGpu;
    };

    /*
     * Settings related to GPU buffers.
     */
    struct GpuBufferSettings
    {
        GpuBufferSettings()
        {
            size = 1024;
            resizeWhenFull = false;
            memoryUsage = MemoryUsage::CPU_W;
            access = AccessMode::READ_WRITE;
        }

        /*
         * How big should this GPU buffer be?
         */
        std::uint32_t size;

        /*
         * If true, the buffer will silently double in size if size is exceeded.
         */
        bool resizeWhenFull;

        /*
         * How will this GPU buffer be used?
         * If data is often uploaded from the GPU between frames, use GPU_W.
         * If data is not changed often, use GPU.
         * If the CPU has to read the data often, use CPU_R.
         */
        MemoryUsage memoryUsage;

        /*
         * Is this buffer mutable after first writing to it?
         * If yes, set to READ_ONLY.
         * Otherwise choose READ_WRITE.
         */
        AccessMode access;
    };
}