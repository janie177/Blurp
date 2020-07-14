#include "opengl/Texture_GL.h"

#include "opengl/GLUtils.h"

namespace blurp
{
    GLuint Texture_GL::GetTextureId() const
    {
        return m_Texture;
    }

    bool Texture_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        assert(m_Settings.dimensions.x > 0 && m_Settings.dimensions.y >= 0 && m_Settings.dimensions.z >= 0 && "Texture needs positive dimensions.");

        const GLenum dataType = ToGL(m_Settings.dataType);
        const GLenum pixelFormat = ToGL(m_Settings.pixelFormat);
        const GLenum wrapMode = ToGL(m_Settings.wrapMode);
        const GLenum minFilter = ToGL(m_Settings.minFilter);
        const GLenum magFilter = ToGL(m_Settings.magFilter);
        const GLenum sizedFormat = ToSizedFormat(m_Settings.pixelFormat, m_Settings.dataType);

        switch (m_Settings.textureType)
        {
        case TextureType::TEXTURE_1D:
            glGenTextures(1, &m_Texture);
            glBindTexture(GL_TEXTURE_1D, m_Texture);
            // set the texture wrapping/filtering options (on the currently bound texture object)
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, wrapMode);
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, minFilter);
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, magFilter);
            glTexImage1D(GL_TEXTURE_1D, 0, pixelFormat, m_Settings.dimensions.x, 0, pixelFormat, dataType, m_Settings.texture1D.data);
            glBindTexture(GL_TEXTURE_1D, 0);
            break;

        case TextureType::TEXTURE_2D:
            glGenTextures(1, &m_Texture);
            glBindTexture(GL_TEXTURE_2D, m_Texture);
            // set the texture wrapping/filtering options (on the currently bound texture object)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
            glTexImage2D(GL_TEXTURE_2D, 0, pixelFormat, m_Settings.dimensions.x, m_Settings.dimensions.y, 0, pixelFormat, dataType, m_Settings.texture2D.data);

            if (m_Settings.generateMipMaps)
            {
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            glBindTexture(GL_TEXTURE_2D, 0);
            break;

        case TextureType::TEXTURE_3D:
            glGenTextures(1, &m_Texture);
            glBindTexture(GL_TEXTURE_3D, m_Texture);
            // set the texture wrapping/filtering options (on the currently bound texture object)
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrapMode);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrapMode);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrapMode);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, minFilter);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, magFilter);
            glTexImage3D(GL_TEXTURE_3D, 0, pixelFormat, m_Settings.dimensions.x, m_Settings.dimensions.y, m_Settings.dimensions.z, 0, pixelFormat, dataType, m_Settings.texture3D.data);

            if (m_Settings.generateMipMaps)
            {
                glGenerateMipmap(GL_TEXTURE_3D);
            }
            glBindTexture(GL_TEXTURE_3D, 0);
            break;

        case TextureType::TEXTURE_2D_ARRAY:

            glGenTextures(1, &m_Texture);
            glBindTexture(GL_TEXTURE_2D_ARRAY, m_Texture);
            glTexStorage3D(GL_TEXTURE_2D_ARRAY, m_Settings.mipLevels, sizedFormat, m_Settings.dimensions.x, m_Settings.dimensions.y, m_Settings.dimensions.z);
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, m_Settings.dimensions.x, m_Settings.dimensions.y, m_Settings.dimensions.z, pixelFormat, dataType, m_Settings.texture2DArray.data);

            // Always set reasonable texture parameters
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, minFilter);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, magFilter);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrapMode);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrapMode);

            if(m_Settings.generateMipMaps)
            {
                glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
            }

            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
            break;

        case TextureType::TEXTURE_CUBEMAP:
            assert(m_Settings.dimensions.x == m_Settings.dimensions.y && "Error: Cubemap texture has to be square!");
            glGenTextures(1, &m_Texture);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_Texture);
            for (auto i = 0; i < 6; ++i)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, pixelFormat, m_Settings.dimensions.x, m_Settings.dimensions.y, 0, pixelFormat, dataType, m_Settings.textureCubeMap.data[i]);
            }

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, magFilter);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilter);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapMode);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapMode);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapMode);

            if (m_Settings.generateMipMaps)
            {
                glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
            }

            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            break;

        default:
            throw std::exception("Error: texture type not supported for OpenGL.");
            return false;
            break;
        }

        return true;
    }

    bool Texture_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        //Delete the texture.
        glDeleteTextures(1, &m_Texture);
        return true;
    }
}
