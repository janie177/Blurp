#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>

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
                                                                                            m_Fbo(0),
                                                                                            m_IsDefault(a_DefaultFrameBuffer),
                                                                                            m_ClearColor(0.f, 0.f, 0.f, 1.f),
                                                                                            m_ViewPort(0.f, 0.f, a_Settings.colorSettings.dimensions),
                                                                                            m_ScissorRect(0.f, 0.f, 99999999.f, 99999999.f)
        {
        }

        GLint GetFrameBufferID();

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;

    public:
        std::uint16_t GetNumColorAttachments() override;
        bool HasColorAttachment() override;
        bool HasDepthAttachment() override;
        bool HasStencilAttachment() override;
        glm::vec4 GetClearColor() override;
        void SetClearColor(const glm::vec4& a_ClearColor) override;
        glm::vec4 GetViewPort() override;
        void SetViewPort(const glm::vec4& a_ViewPort) override;
        glm::vec4 GetScissorRect() override;
        void SetScissorRect(const glm::vec4& a_ScissorRect) override;

        /*
         * Get the color attachment texture of this FrameBuffer.
         * Note: If this is the default FrameBuffer made by the OpenGL context,
         * this cannot be accessed. In that case nullptr is returned.
         */
        std::shared_ptr<Texture> GetColorAttachment() override;

        /*
         * Get the depth stancel attachment texture of this FrameBuffer.
         * Note: If this is the default FrameBuffer made by the OpenGL context,
         * this cannot be accessed. In that case nullptr is returned.
         */
        std::shared_ptr<Texture> GetDepthStencilAttachment() override;
    private:
        GLuint m_Fbo;
        bool m_IsDefault;

        glm::vec4 m_ClearColor;
        glm::vec4 m_ViewPort;
        glm::vec4 m_ScissorRect;

        std::shared_ptr<Texture> m_ColorAttachment;
        std::shared_ptr<Texture> m_DepthStencilAttachment;

    };
}
