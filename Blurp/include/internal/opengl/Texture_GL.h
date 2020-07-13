#pragma once
#include <GL/glew.h>
#include "Texture.h"

namespace blurp
{
    /*
     * Texture class encapsulating 1D and 2D textures.
     */
    class Texture_GL : public Texture
    {
    public:
        Texture_GL(const TextureSettings& a_Settings) : Texture(a_Settings), m_Texture(0) {}

        /*
         * Get the GLuint ID of the internal OpenGL texture.
         */
        GLuint GetTextureId() const;

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;

    private:
        GLuint m_Texture;
    };
}