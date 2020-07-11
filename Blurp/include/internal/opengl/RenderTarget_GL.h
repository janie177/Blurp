#pragma once
#include <GL/glew.h>
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/common.hpp>

#include "RenderTarget.h"

namespace blurp
{
    class RenderTarget_GL : public RenderTarget
    {
    public:
        /*
         * Create a OpenGL Render Target.
         * When a_DefaultFrameBuffer is set to true, this acts as a dummy object that binds to default frame buffer created
         * by OpenGL itself.
         */
        RenderTarget_GL(const RenderTargetSettings& a_Settings, bool a_DefaultFrameBuffer) : RenderTarget(a_Settings),
                                                                                             m_FBO(0),
                                                                                             m_IsDefault(
                                                                                                 a_DefaultFrameBuffer)
        {
        }

        GLint GetFrameBufferID();

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;

    public:
        bool GetNumColorAttachments() override;
        bool HasColorAttachment() override;
        bool HasDepthAttachment() override;
        bool HasStencilAttachment() override;
        glm::vec4 GetClearColor() override;
        void SetClearColor(const glm::vec4& a_ClearColor) override;
        glm::vec4 GetViewPort() override;
        void SetViewPort(const glm::vec4& a_ViewPort) override;
        glm::vec4 GetScissorRect() override;
        void SetScissorRect(const glm::vec4& a_ScissorRect) override;

        std::shared_ptr<Texture> GetColorAttachment() override;
        std::shared_ptr<Texture> GetDepthStencilAttachment() override;
    private:
        GLuint m_FBO;
        bool m_IsDefault;
    };
}
