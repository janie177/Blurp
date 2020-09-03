#include "Texture.h"

namespace blurp
{
    std::uint16_t Texture::GetMipLevels() const
    {
        return m_Settings.numMipMaps;
    }

    glm::vec<3, std::uint32_t> Texture::GetDimensions() const
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

    AccessMode Texture::GetAccessMode() const
    {
        return m_Settings.memoryAccess;
    }

    void Texture::OnLock()
    {
        assert(m_Settings.memoryAccess != AccessMode::READ_ONLY && "Locking read only resource. This is not necessary and should not happen.");
    }

    void Texture::OnUnlock()
    {

    }
}
