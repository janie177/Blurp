#pragma once
#include <glm/glm.hpp>
#include <string>

/*
 * This file contains all structs used to describe a resource before creation.
 */

namespace blurp
{
    /*
     * Enumerations used in the settings structs.
     */

    enum class RenderPassType
    {
        RP_CLEAR,
        RP_FORWARD,
        RP_DEFERRED,
        RP_SHADOWMAP,
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
        TEXTURE_2D,
        TEXTURE_3D,
        TEXTURE_ARRAY
    };

    enum class ColorFormat
    {
        COLOR_RGBA32,
    };

    enum class WindowFlags : std::uint16_t
    {
        OPEN_FULLSCREEN = 1 << 0,
        HIDE_CURSOR = 1 << 1,
        CAPTURE_CURSOR = 1 << 2,
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

    struct WindowSettings
    {
        glm::vec2 dimensions;
        bool fullScreen;
        WindowType type;
        std::string name;
        WindowFlags flags;
    };

    struct CameraSettings
    {
        
    };

    struct TextureSettings
    {
        
    };

    struct BlurpSettings
    {
        WindowSettings windowSettings;
        GraphicsAPI graphicsAPI;
    };

    struct MeshSettings
    {
        
    };

    struct LightSettings
    {
        
    };

    struct RenderTargetSettings
    {
        
    };

    struct SwapChainSettings
    {
        
    };

    struct MaterialSettings
    {
        
    };
    
}