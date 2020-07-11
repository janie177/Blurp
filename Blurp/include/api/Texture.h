#pragma once
#include "RenderResource.h"

namespace blurp
{
    class Texture : public RenderResource
    {
    public:
        Texture(const TextureSettings& a_Settings) : m_Settings(a_Settings) {}

        /*
         * Get the amount of mip levels of this texture.
         * When 0, it means no mip maps are generated.
         */
        std::uint16_t GetMipLevels() const;

        /*
         * Get the dimensions of this texture.
         */
        glm::vec2 GetDimensions() const;

        /*
         * Get the texture type of this texture.
         */
        TextureType GetTextureType() const;

        /*
         * Get the pixel format of this texture.
         */
        PixelFormat GetPixelFormat() const;

        /*
         * Get the data type of the pixels in this texture.
         */
        DataType GetDataType() const;

        /*
         * Get the minification filter mode for this texture.
         * This setting determines how the texture shrinks when it is displayed smaller on the screen.
         * Settings include mipmapping, linear and nearest.
         */
        MinFilterType GetMinificationFilter() const;

        /*
         * Get the magnification filter mode for this texture.
         * This setting determines how the texture behaves when it is displayed on the screen in larger dimensions
         * than the texture itself has. 
         */
        MagFilterType GetMagnificationFilter() const;

        /*
         * Get the wrap mode for this texture.
         * This determines how the texture behaves when indexed using UV coordinates.
         * Options include repeating the border pixel when out of bounds (CLAMP_TO_EDGE),
         * mirroring the image and repeating it that way, and repeating it regularly.
         */
        WrapMode GetWrapMode() const;

    protected:
        TextureSettings m_Settings;

    };
}
