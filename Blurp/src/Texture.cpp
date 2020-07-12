#include "Texture.h"

namespace blurp
{
    std::uint16_t Texture::GetMipLevels() const
    {
        return m_Settings.mipLevels;
    }

    glm::vec2 Texture::GetDimensions() const
    {
        return m_Settings.dimensions;
    }

    TextureType Texture::GetTextureType() const
    {
        return m_Settings.textureType;
    }

    PixelFormat Texture::GetPixelFormat() const
    {
        return m_Settings.pixelFormat;
    }

    DataType Texture::GetDataType() const
    {
        return m_Settings.dataType;
    }

    MinFilterType Texture::GetMinificationFilter() const
    {
        return m_Settings.minFilter;
    }

    MagFilterType Texture::GetMagnificationFilter() const
    {
        return m_Settings.magFilter;
    }

    WrapMode Texture::GetWrapMode() const
    {
        return m_Settings.wrapMode;
    }
}