#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#define NUM_VERTEX_ATRRIBS 12
#define NUM_MATERIAL_ATRRIBS 13

/*
 * This file contains all structs used to describe a resource before creation.
 */

namespace blurp
{
    class Texture;

    /*
     * Enumerations used in the settings structs.
     */

    enum class ProjectionMode
    {
        ORTHOGRAPHIC,
        PERSPECTIVE
    };

    enum class Direction : std::uint8_t
    {
        RIGHT,
        LEFT,
        UP,
        DOWN,
        FORWARD,
        BACKWARD
    };

    enum class RenderPassType
    {
        RP_HELLOTRIANGLE,
        RP_CLEAR,
        RP_FORWARD,
        RP_DEFERRED,
        RP_SHADOWMAP,
        RP_CUBEMAP,
        RP_SKYBOX,
        RP_DOF,
        RP_2D,
        RP_BLOOM,
        RP_BLUR,
        RP_ANIMATION
    };

    enum class WindowType
    {
        WINDOW_WIN32,
        NONE
    };

    enum class LightType
    {
        LIGHT_AMBIENT,
        LIGHT_SPOT,
        LIGHT_DIRECTIONAL,
        LIGHT_POINT
    };

    enum class GraphicsAPI
    {
        OPENGL,
        OPENGLES,
        VULKAN,
        DIRECTX12
    };

    enum class TextureType
    {
        TEXTURE_1D,
        TEXTURE_2D,
        TEXTURE_3D,
        TEXTURE_CUBEMAP,
        TEXTURE_2D_ARRAY,
        TEXTURE_CUBEMAP_ARRAY
    };

    enum class WrapMode
    {
        CLAMP_TO_EDGE,
        MIRRORED_REPEAT,
        REPEAT
    };


    /*
     * Enumeration that can be used to specify how memory will be accessed.
     * CPU_R means that the memory should be optimized for CPU reading. GPU access is not optimal. Used for reading back compute output.
     * CPU_W means that the memory should be optimized for CPU writing. GPU access is not optimal. Used for frequently changing data like transforms.
     * GPU means that the memory should be optimized for GPU access. CPU access will be slow. Used for data that does not change often.
     *
     * Note: This does not prevent the CPU or GPU from accessing the memory. It just allows for the memory to be faster when used in this scenario.
     */
    enum class MemoryUsage
    {
        CPU_R,
        CPU_W,
        GPU
    };

    /*
     * Enumeration that can be used to specify whether a resource is mutable or not.
     * READ_ONLY means a resource will never be written to once it is uploaded.
     * READ_WRITE means that a resource may change, and requires locking and state tracking.
     */
    enum class AccessMode
    {
        READ_ONLY,
        READ_WRITE
    };

    enum class MagFilterType
    {
        NEAREST,
        LINEAR
    };

    enum class MinFilterType
    {
        NEAREST,
        LINEAR,
        MIPMAP_NEAREST,
        MIPMAP_LINEAR
    };

    enum class ShaderType
    {
        COMPUTE,
        GRAPHICS
    };

    enum class PixelFormat
    {
        R,
        RG,
        RGB,
        RGBA,
        DEPTH,
        DEPTH_STENCIL
    };

    enum class DataType
    {
        FLOAT,
        INT,
        BYTE,
        SHORT,
        UINT,
        UBYTE,
        USHORT
    };

    /*
     * Helper function to see if a DataType is signed or unsigned.
     */
    bool IsSigned(DataType a_Type);

    enum class WindowFlags : std::uint16_t
    {
        OPEN_FULLSCREEN = 1 << 0,
        HIDE_CURSOR = 1 << 1,
        CAPTURE_CURSOR = 1 << 2,
        NONE = 1 << 3
    };

    /*
     * Bitwise OR operator.
     * Combine WindowFlags into a single new value.
     */
    inline WindowFlags operator|(WindowFlags a_Lhs, WindowFlags a_Rhs)
    {
        return static_cast<WindowFlags>(static_cast<int>(a_Lhs) | static_cast<int>(a_Rhs));
    }

    /*
     * Bitwise AND operator.
     * Compare if the flags match.
     */
    inline bool operator&(WindowFlags a_Lhs, WindowFlags a_Rhs)
    {
        return (static_cast<int>(a_Lhs) & static_cast<int>(a_Rhs)) != 0;
    }

    enum class VertexAttribute : std::uint16_t
    {
        //X and Y position.
        POSITION_2D = 1 << 0,

        //X, Y and Z position.
        POSITION_3D = 1 << 1,

        //UV coordinates for a texture.
        //Multiple UV coords can be enabled for different material
        UV_COORDS = 1 << 2,

        //X, Y and Z direction of the surface.
        NORMAL = 1 << 3,

        //R, G and B color channels.
        COLOR = 1 << 4,

        //X, Y and Z indicating a curved direction.
        TANGENT = 1 << 5,

        //X, Y and Z indicating a curved surface. Used in combination with Tangent.
        BI_TANGENT = 1 << 6,

        //Vec3 of the bone indices affecting vertex.
        BONE_INDEX = 1 << 7,

        //Vec3 of the weights of each bone (to be used with BONE_INDEX).
        BONE_WEIGHT = 1 << 8,

        //Matrices for instancing.
        MATRIX = 1 << 9,

        //The Integer material ID to use. Used in combination with Material Batches.
        MATERIAL_ID = 1 << 10,

        //Transpose of the inverse instance matrix. Used with MATRIX to allow for uneven scaling with correct lighting normals.
        ITMATRIX = 1 << 11,
    };

    //All vertex attributes in an iterable format.
    const static VertexAttribute VERTEX_ATTRIBUTES[NUM_VERTEX_ATRRIBS]{
        VertexAttribute::POSITION_2D,
        VertexAttribute::POSITION_3D,
        VertexAttribute::UV_COORDS,
        VertexAttribute::NORMAL,
        VertexAttribute::COLOR,
        VertexAttribute::TANGENT,
        VertexAttribute::BI_TANGENT,
        VertexAttribute::BONE_INDEX,
        VertexAttribute::BONE_WEIGHT,
        VertexAttribute::MATRIX,
        VertexAttribute::MATERIAL_ID,
        VertexAttribute::ITMATRIX,
    };

    inline VertexAttribute operator|(VertexAttribute a_Lhs, VertexAttribute a_Rhs)
    {
        return static_cast<VertexAttribute>(static_cast<std::uint16_t>(a_Lhs) | static_cast<std::uint16_t>(a_Rhs));
    }

    inline VertexAttribute operator^(VertexAttribute a_Lhs, VertexAttribute a_Rhs)
    {
        return static_cast<VertexAttribute>(static_cast<std::uint16_t>(a_Lhs) ^ static_cast<std::uint16_t>(a_Rhs));
    }

    inline VertexAttribute operator~(VertexAttribute a_Lhs)
    {
        return static_cast<VertexAttribute>(~static_cast<std::uint16_t>(a_Lhs));
    }

    inline VertexAttribute operator&(VertexAttribute a_Lhs, VertexAttribute a_Rhs)
    {
        return static_cast<VertexAttribute>(static_cast<std::uint16_t>(a_Lhs) & static_cast<std::uint16_t>(a_Rhs));
    }

    inline bool operator==(VertexAttribute a_Lhs, VertexAttribute a_Rhs)
    {
        return static_cast<std::uint16_t>(a_Lhs) == static_cast<std::uint16_t>(a_Rhs);
    }

    /*
     * Enumeration containing the bitmasks for various material attributes.
     */
    enum class MaterialAttribute : std::uint16_t
    {
        //Diffuse texture used for coloring this material.
        //Enabling this disables DIFFUSE_CONSTANT_VALUE
        //RGB format.
        DIFFUSE_TEXTURE = 1 << 0,

        //Normal texture containing the direction of each pixel in XYZ format. Can only be used with meshes that contain normal information.
        //RGB format.
        NORMAL_TEXTURE = 1 << 1,

        //Emissive texture indicating the brightness in R, G and B channels. Added on top of calculated light value to appear glowing.
        //Enabling this disables EMISSIVE_CONSTANT_VALUE.
        //RGB format.
        EMISSIVE_TEXTURE = 1 << 2,

        //Metallicness of the surface stored as a single byte value.
        //Enabling this disables the METALLIC_CONSTANT_VALUE setting.
        //Stored in the R channel of an Metallic/Roughness/Alpha texture.
        METALLIC_TEXTURE = 1 << 3,

        //Roughness of the surface.
        //Enabling this disables this ROUGHNESS_CONSTANT_VALUE setting.
        //Stored in the G channel of an Metallic/Roughness/Alpha texture.
        ROUGHNESS_TEXTURE = 1 << 4,

        //Alpha of the surface. Stored as an RG texture.
        //Enabling this disables the ALPHA_CONSTANT_VALUE setting.
        //Stored in the B channel of an Metallic/Roughness/Alpha texture.
        ALPHA_TEXTURE = 1 << 5,

        //Occlusion texture containing a single byte value per pixel indicating how much it should be darkened.
        //Stored in the R channel of an Occlusion/Height texture.
        OCCLUSION_TEXTURE = 1 << 6,

        //Height offset from the surface. This moves vertices along their normal direction.
        //Stored as the G and B channels of the Occlusion/Height texture.
        HEIGHT_TEXTURE = 1 << 7,

        //Allows using a single color for the diffuse texture of this material.
        //Enabling this disables DIFFUSE_TEXTURE texture.
        //Colors scaled between 0 and 1.
        DIFFUSE_CONSTANT_VALUE = 1 << 8,

        //Allows the setting of a single color for the emissive texture of this material.
        //Enabling this disables EMISSIVE_TEXTURE texture.
        //Emissive color specified as RGB normalized between 0 and 1.
        EMISSIVE_CONSTANT_VALUE = 1 << 9,

        //Allows the setting of the metallicness of the entire surface.
        //Enabling this disables METALLIC_TEXTURE texture.
        //Float normalized between 0 and 1.
        METALLIC_CONSTANT_VALUE = 1 << 10,

        //Allows the setting of a single float value to measure the roughness of the entire material.
        //Enabling this disables the ROUGHNESS_TEXTURE texture.
        //Normalized between 0 and 1.
        ROUGHNESS_CONSTANT_VALUE = 1 << 11,

        //Allows the setting of a single float value representing the opacity of the entire material.
        //This disables the ALPHA_TEXTURE texture.
        //Normalized between 0 (translucent) and 1 (opaque).
        ALPHA_CONSTANT_VALUE = 1 << 12,
    };

    /*
     * All material attributes sorted in order of the bitmask.
     */
    const static MaterialAttribute MATERIAL_ATTRIBUTES[NUM_MATERIAL_ATRRIBS]{
        MaterialAttribute::DIFFUSE_TEXTURE,
        MaterialAttribute::NORMAL_TEXTURE,
        MaterialAttribute::EMISSIVE_TEXTURE,
        MaterialAttribute::METALLIC_TEXTURE,
        MaterialAttribute::ROUGHNESS_TEXTURE,
        MaterialAttribute::ALPHA_TEXTURE,
        MaterialAttribute::OCCLUSION_TEXTURE,
        MaterialAttribute::HEIGHT_TEXTURE,

        MaterialAttribute::DIFFUSE_CONSTANT_VALUE,
        MaterialAttribute::EMISSIVE_CONSTANT_VALUE,
        MaterialAttribute::METALLIC_CONSTANT_VALUE,
        MaterialAttribute::ROUGHNESS_CONSTANT_VALUE,
        MaterialAttribute::ALPHA_CONSTANT_VALUE,
    };


    /*
     * Array of all texture material attributes.
     */
    const static MaterialAttribute TEXTURE_MATERIAL_ATTRIBUTES[]{
        MaterialAttribute::DIFFUSE_TEXTURE,
        MaterialAttribute::NORMAL_TEXTURE,
        MaterialAttribute::EMISSIVE_TEXTURE,
        MaterialAttribute::METALLIC_TEXTURE,
        MaterialAttribute::ROUGHNESS_TEXTURE,
        MaterialAttribute::ALPHA_TEXTURE,
        MaterialAttribute::OCCLUSION_TEXTURE,
        MaterialAttribute::HEIGHT_TEXTURE,
    };

    /*
     * Array of all constant value material attributes.
     */
    const static MaterialAttribute CONSTANT_MATERIAL_ATTRIBUTES[]{
        MaterialAttribute::DIFFUSE_CONSTANT_VALUE,
        MaterialAttribute::EMISSIVE_CONSTANT_VALUE,
        MaterialAttribute::METALLIC_CONSTANT_VALUE,
        MaterialAttribute::ROUGHNESS_CONSTANT_VALUE,
        MaterialAttribute::ALPHA_CONSTANT_VALUE,
    };

    struct MaterialAttributeInfo
    {
        //Preprocessor definition name of this attribute.
        std::string defineName;
        std::uint8_t numElements;
    };

    /*
     * Unordered map containing information about each material attribute.
     */
    static const std::unordered_map<MaterialAttribute, MaterialAttributeInfo> MATERIAL_ATTRIBUTE_INFO = {
        {MaterialAttribute::DIFFUSE_TEXTURE, {"MAT_DIFFUSE_TEXTURE_DEFINE", 3}},
        {MaterialAttribute::NORMAL_TEXTURE, {"MAT_NORMAL_TEXTURE_DEFINE", 3}},
        {MaterialAttribute::EMISSIVE_TEXTURE, {"MAT_EMISSIVE_TEXTURE_DEFINE", 3}},
        {MaterialAttribute::METALLIC_TEXTURE, {"MAT_METALLIC_TEXTURE_DEFINE", 1}},
        {MaterialAttribute::ROUGHNESS_TEXTURE, {"MAT_ROUGHNESS_TEXTURE_DEFINE", 1}},
        {MaterialAttribute::ALPHA_TEXTURE, {"MAT_ALPHA_TEXTURE_DEFINE", 1}},
        {MaterialAttribute::OCCLUSION_TEXTURE, {"MAT_OCCLUSION_TEXTURE_DEFINE", 1}},
        {MaterialAttribute::HEIGHT_TEXTURE, {"MAT_HEIGHT_TEXTURE_DEFINE", 2}},

        {MaterialAttribute::DIFFUSE_CONSTANT_VALUE, {"MAT_DIFFUSE_CONSTANT_DEFINE", 3}},
        {MaterialAttribute::EMISSIVE_CONSTANT_VALUE, {"MAT_EMISSIVE_CONSTANT_DEFINE", 3}},
        {MaterialAttribute::METALLIC_CONSTANT_VALUE, {"MAT_METALLIC_CONSTANT_DEFINE", 1}},
        {MaterialAttribute::ROUGHNESS_CONSTANT_VALUE, {"MAT_ROUGHNESS_CONSTANT_DEFINE", 1}},
        {MaterialAttribute::ALPHA_CONSTANT_VALUE, {"MAT_ALPHA_CONSTANT_DEFINE", 1}},
    };

    /*
     * The settings structs.
     */

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

        //The dimensions of this texture.
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
        }

        //Buffer format.
        RenderTargetSettings renderTargetSettings;

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

        //The field of view for this camera.
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

    /*
     * Static lookup table for vertex attribute information.
     */
    struct VertexAttributeInfo
    {
        //The amount of elements for this vertex attribute.
        std::uint32_t numElements;

        //The data type of each element.
        DataType dataType;

        //The unique name of this vertex attribute
        std::string name;

        //The define name used in preprocessor definitions in shaders.
        std::string defineName;
    };

    struct VertexAttributeData
    {
        VertexAttributeData()
        {
            byteOffset = 0;
            byteStride = 0;
            normalize = false;
            instanceDivisor = 0;
        }

        //The offset from the start of the buffer to the first vertex attribute of this type.
        std::uint32_t byteOffset;

        //The stride in bytes between the data elements. When 0, they are tightly packed.
        std::uint32_t byteStride;

        //Normalize or not (between 0 and 1).
        bool normalize;

        //The instance divisor of this attribute. If 0, no instancing is used.
        //Any other value indicates after how many full instance draws the attribute updates in the shader.
        std::uint16_t instanceDivisor;
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
            {VertexAttribute::POSITION_2D, {2, DataType::FLOAT, "va_Position2D", "VA_POS2D_DEF"}},
            {VertexAttribute::POSITION_3D, {3, DataType::FLOAT, "va_Position3D", "VA_POS3D_DEF"}},
            {VertexAttribute::UV_COORDS, {2, DataType::FLOAT, "va_UVCoords", "VA_UVCOORD_DEF"}},
            {VertexAttribute::NORMAL, {3, DataType::FLOAT, "va_Normal", "VA_NORMAL_DEF"}},
            {VertexAttribute::COLOR, {3, DataType::FLOAT, "va_Color", "VA_COLOR_DEF"}},
            {VertexAttribute::TANGENT, {3, DataType::FLOAT, "va_Tangent", "VA_TANGENT_DEF"}},
            {VertexAttribute::BI_TANGENT, {3, DataType::FLOAT, "va_BiTangent", "VA_BITANGENT_DEF"}},
            {VertexAttribute::BONE_INDEX, {3, DataType::UINT, "va_BoneIndex", "VA_BONEINDEX_DEF"}},
            {VertexAttribute::BONE_WEIGHT, {3, DataType::FLOAT, "va_BoneWeight", "VA_BONEWEIGHT_DEF"}},
            {VertexAttribute::MATRIX, {16, DataType::FLOAT, "va_Matrix", "VA_MATRIX_DEF"}},
            {VertexAttribute::MATERIAL_ID, {1, DataType::USHORT, "va_MaterialID", "VA_MATERIALID_DEF"}},
            {VertexAttribute::ITMATRIX, {16, DataType::FLOAT, "va_ITMatrix", "VA_ITMATRIX_DEF"}},
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
            spotLight.position = { 0.f, 0.f, 0.f };
            spotLight.direction = { 0.f, -1.f, 0.f };
            spotLight.angle = 45.f;
        }

        //Type of the light.
        LightType type;

        //Color of the light.
        glm::vec3 color;

        //Intensity of the light.
        float intensity;

        union
        {
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
            m_Mask = static_cast<MaterialAttribute>(0);

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
        //Mask of enabled attributes.
        MaterialAttribute m_Mask;

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
            m_Mask = static_cast<MaterialAttribute>(0);

            textureData = nullptr;

            diffuseConstantData = nullptr;
            emissiveConstantData = nullptr;
            metallicConstantData = nullptr;
            roughnessConstantData = nullptr;
            alphaConstantData = nullptr;

            materialCount = 0;
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
         * Get the unsigned 16 bit int mask of all enabled attributes.
         */
        std::uint16_t GetMask() const;

    public:
        //The amount of materials in this batch.
        std::uint32_t materialCount;

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
        float* diffuseConstantData;
        float* emissiveConstantData;

        //Array of floats. Size of MaterialCount. One float per material.
        float* metallicConstantData;
        float* roughnessConstantData;
        float* alphaConstantData;

    private:
        MaterialAttribute m_Mask;
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