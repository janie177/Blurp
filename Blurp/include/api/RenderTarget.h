#pragma once
#include "RenderResource.h"
#include "Settings.h"
#include <memory>

namespace blurp
{
    class Texture;

    /*
     * RenderTarget is something that can be rendered into by the shader pipeline.
     * In OpenGL, this is implemented through the use of FrameBufferObjects.
     * In Direct3D12, RenderTargetViews are used and bound to the command lists.
     */
    class RenderTarget : public RenderResource
    {
    public:
        RenderTarget(const RenderTargetSettings& a_Settings) : m_Settings(a_Settings){}

        //TODO make it so that a render target can only be bound to a single pipeline at the same time.
        //Some sort of lock? 
        
    public:
        /*
         * Get the amount of color attachments bound to this RenderTarget.
         */
        virtual std::uint16_t GetNumColorAttachments() = 0;

        /*
         * Returns true when this RenderTarget has a color attachment.
         */
        virtual bool HasColorAttachment() = 0;

        /*
         * Returns true when this RenderTarget has a depth attachment.
         */
        virtual bool HasDepthAttachment() = 0;

        /*
         * Returns true when this RenderTarget has a stencil attachment.
         */
        virtual bool HasStencilAttachment() = 0;

        /*
         * Get the color attachment bound to this RenderTarget.
         */
        virtual std::shared_ptr<Texture> GetColorAttachment() = 0;

        /*
         * Get the depth stencil attachment bound to this RenderTarget.
         */
        virtual std::shared_ptr<Texture> GetDepthStencilAttachment() = 0;

        /*
         * Get the clear color of this RenderTarget.
         */
        virtual glm::vec4 GetClearColor() = 0;

        /*
         * Set the clear color of this RenderTarget.
         */
        virtual void SetClearColor(const glm::vec4& a_ClearColor) = 0;

        /*
         * Get the viewport of this RenderTarget.
         */
        virtual glm::vec4 GetViewPort() = 0;

        /*
         * Set the viewport of this RenderTarget.
         */
        virtual void SetViewPort(const glm::vec4& a_ViewPort) = 0;

        /*
         * Get the scissor rect of this RenderTarget.
         */
        virtual glm::vec4 GetScissorRect() = 0;

        /*
         * Set the scissor rect of this RenderTarget.
         */
        virtual void SetScissorRect(const glm::vec4& a_ScissorRect) = 0;
        
    protected:
        RenderTargetSettings m_Settings;
    };
}
