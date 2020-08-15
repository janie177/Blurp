#pragma once
#include <GL/glew.h>

#include "MaterialBatch.h"

namespace blurp
{
    class MaterialBatch_GL : public MaterialBatch
    {
    public:
        explicit MaterialBatch_GL(const MaterialBatchSettings& a_Settings) : MaterialBatch(a_Settings), m_HasTexture(false), m_HasUbo(false), m_Ubo(0) {}

        GLuint GetUboID() const
        {
            return m_Ubo;
        }

        bool HasUbo() const
        {
            return m_HasUbo;
        }

        bool HasTexture() const
        {
            return m_HasTexture;
        }

        std::shared_ptr<Texture> GetTexture()
        {
            return m_ArrayTexture;
        }

        int GetActiveTextureCount() const
        {
            return m_Settings.textureCount;
        }

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;

    private:
        bool m_HasTexture;
        bool m_HasUbo;
        GLuint m_Ubo;
        std::shared_ptr<Texture> m_ArrayTexture;
    };
}
