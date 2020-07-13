#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>

/*
 * This file contains all structs used to describe a resource before creation.
 */

namespace blurp
{
    /*
     * Enumerations used in the settings structs.
     */

    enum class Direction : std::uint8_t
    {
        FORWARD = 0,
        RIGHT,
        LEFT,
        BACKWARD,
        UP,
        DOWN
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
        NONE,
        TEXTURE_1D,
        TEXTURE_2D,
        TEXTURE_3D,
        TEXTURE_CUBEMAP,
        TEXTURE_2D_ARRAY
    };

    enum class WrapMode
    {
        CLAMP_TO_EDGE,
        MIRRORED_REPEAT,
        REPEAT
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
            mipLevels = 0;
            minFilter = MinFilterType::LINEAR;
            magFilter = MagFilterType::LINEAR;
            wrapMode = WrapMode::REPEAT;

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

        //The amount of mip levels to generate. When 0, automatically determines.
        std::uint16_t mipLevels;

        //How should this texture behave when scaled down?
        MinFilterType minFilter;

        //How should this texture behave when scaled up?
        MagFilterType magFilter;

        //How should this texture behave when sampled outside of the 0-1 UVW coordinate range.
        WrapMode wrapMode;

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

        };

    };

    struct RenderTargetSettings
    {
        RenderTargetSettings()
        {
            //Default values.
            depthStencilSettings.enable = true;
            depthStencilSettings.format = PixelFormat::DEPTH_STENCIL;
        }

        //Color buffer attachment settings.
        TextureSettings colorSettings;

        /*
         * Settings related to the depth/stencil buffer.
         */
        struct DepthStencilSettings
        {
            bool enable;        //Set to false to prevent a depth and/or stencil buffer from being created.
            PixelFormat format; //Should be DEPTH or DEPTH_STENCIL
        } depthStencilSettings;
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
            swapChainSettings.renderTargetSettings.colorSettings.dimensions = { dimensions, 1 };
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
        //TODO
    };

    struct BlurpSettings
    {
        BlurpSettings()
        {
            //Default settings.
            graphicsAPI = GraphicsAPI::OPENGL;
        }

        WindowSettings windowSettings;
        GraphicsAPI graphicsAPI;
    };

    struct MeshSettings
    {
        //TODO maybe add a static/dynamic thing here? mutable/unmutable? To optimize GPU memory.
    };

    struct LightSettings
    {
        //TODO
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