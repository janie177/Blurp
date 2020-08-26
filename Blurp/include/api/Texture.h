#pragma once
#include "Lockable.h"
#include "RenderResource.h"

namespace blurp
{
    class Texture : public RenderResource, public Lockable
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

        /*
         * Get how this texture can be accessed.
         * Can be READ or READ_WRITE.
         */
        AccessMode GetAccessMode() const;


        //TODO remove this. Get pixels from an image.
        virtual unsigned char* GetPixels(glm::vec3 a_Start, glm::vec3 a_Size, int a_Channels) = 0;

    protected:
        void OnLock() override;
        void OnUnlock() override;

    protected:

        TextureSettings m_Settings;

    };
}
