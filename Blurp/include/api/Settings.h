#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

#define NUM_VERTEX_ATRRIBS 9
#include <unordered_map>

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

    enum class AccessMode
    {
        READ,
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
        UINT,
        BYTE,
        UBYTE,
        SHORT,
        USHORT
    };

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

        //Vec3 of the bone indices affecting vertex.
        BONE_INDEX = 1 << 6,

        //Vec3 of the weights of each bone (to be used with BONE_INDEX).
        BONE_WEIGHT = 1 << 7,

        //Matrices for instancing.
        MATRIX
    };

    //All vertex attributes in an iterable format.
    const static VertexAttribute VERTEX_ATTRIBUTES[NUM_VERTEX_ATRRIBS]{
        VertexAttribute::POSITION_3D,
        VertexAttribute::POSITION_2D,
        VertexAttribute::UV_COORDS,
        VertexAttribute::NORMAL,
        VertexAttribute::COLOR,
        VertexAttribute::TANGENT,
        VertexAttribute::BONE_INDEX,
        VertexAttribute::BONE_WEIGHT,
        VertexAttribute::MATRIX
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
            memoryAccess = AccessMode::READ;
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
            throw std::exception("Fatal error: This should never happen. Were not vertex attributes added but not to VERTEX_ATTRIBUTE_INFO?");
        }

    private:
        VertexAttribute m_Mask;
        VertexAttributeData m_Data[NUM_VERTEX_ATRRIBS];

        inline static const std::unordered_map<VertexAttribute, VertexAttributeInfo> VERTEX_ATTRIBUTE_INFO = {
            {VertexAttribute::POSITION_3D, {3, DataType::FLOAT, "va_Position3D", "VA_POS3D_DEF"}},
            {VertexAttribute::POSITION_2D, {2, DataType::FLOAT, "va_Position2D", "VA_POS2D_DEF"}},
            {VertexAttribute::UV_COORDS, {2, DataType::FLOAT, "va_UVCoords", "VA_UVCOORD_DEF"}},
            {VertexAttribute::NORMAL, {3, DataType::FLOAT, "va_Normal", "VA_NORMAL_DEF"}},
            {VertexAttribute::COLOR, {3, DataType::FLOAT, "va_Color", "VA_COLOR_DEF"}},
            {VertexAttribute::TANGENT, {3, DataType::FLOAT, "va_Tangent", "VA_TANGENT_DEF"}},
            {VertexAttribute::BONE_INDEX, {3, DataType::UINT, "va_BoneIndex", "VA_BONEINDEX_DEF"}},
            {VertexAttribute::BONE_WEIGHT, {3, DataType::FLOAT, "va_BoneWeight", "VA_BONEWEIGHT_DEF"}},
            {VertexAttribute::MATRIX, {16, DataType::FLOAT, "va_Matrix", "VA_MATRIX_DEF"}},
        };
    };

    struct MeshSettings
    {
        MeshSettings()
        {
            usage = AccessMode::READ;
            vertexData = nullptr;
            indexData = nullptr;
            numIndices = 0;
            indexDataType = DataType::SHORT;
            vertexDataSizeBytes = 0;
        }

        //Which vertex attributes are enabled for this mesh?
        VertexSettings vertexSettings;

        //The memory access mode for this mesh.
        //This is not enforced, but should be carefully considered.
        //If a mesh does not change for more than a single frame, READ is recommended.
        //If a mesh data is updated every frame, then READ_WRITE should be used.
        AccessMode usage;

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

    struct MaterialSettings
    {
        //TODO   
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
}