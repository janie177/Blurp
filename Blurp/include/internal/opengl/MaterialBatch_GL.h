#pragma once
#include <GL/glew.h>

#include "MaterialBatch.h"

namespace blurp
{
    class MaterialBatch_GL : public MaterialBatch
    {
    public:
        explicit MaterialBatch_GL(const MaterialBatchSettings& a_Settings) : MaterialBatch(a_Settings), m_HasMaterial(false), m_HasUbo(false), m_Ubo(0) {}

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;

    private:
        bool m_HasMaterial;
        bool m_HasUbo;
        GLuint m_Ubo;
        std::shared_ptr<Texture> m_ArrayTexture;
    };
}
