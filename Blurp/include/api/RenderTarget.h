#pragma once
#include "RenderResource.h"
#include "Settings.h"
#include <memory>

#include "Lockable.h"

#define MAX_NUM_COLOR_ATTACHMENTS 8

namespace blurp
{
    class Texture;

    /*
     * RenderTarget is something that can be rendered into by the shader pipeline.
     * In OpenGL, this is implemented through the use of FrameBufferObjects.
     * In Direct3D12, RenderTargetViews are used and bound to the command lists.
     */
    class RenderTarget : public RenderResource, public Lockable
    {
    public:
        RenderTarget(const RenderTargetSettings& a_Settings) : m_Settings(a_Settings), m_AllowAttachments(true)
        {
            
        }

        /*
         * Get the amount of color attachments bound to this RenderTarget.
         */
        std::uint16_t GetNumColorAttachments() const;

        /*
         * Get the maximum number of color attachments.
         */
        std::uint16_t GetMaxColorAttachments() const;

        /*
         * Returns true when this RenderTarget has at least one color attachment.
         */
        bool HasColorAttachment() const;

        /*
         * Returns true when this RenderTarget has a depth attachment.
         */
        virtual bool HasDepthAttachment() const;

        /*
         * Returns true when this RenderTarget has a stencil attachment.
         */
        virtual bool HasStencilAttachment() const;

        /*
         * Add an attachment to this render target in the given slot. 
         */
        bool SetColorAttachment(std::uint16_t a_Slot, std::shared_ptr<Texture> a_Attachment);

        /*
         * Add a depth stencil texture attachment to this render target.
         */
        bool SetDepthStencilAttachment(std::shared_ptr<Texture> a_Attachment);

        /*
         * Get the color attachment at the given slot.
         * If not bound, returns nullptr.
         */
        std::shared_ptr<Texture> GetColorAttachment(std::uint16_t a_Slot);

        /*
         * Get the depth stencil texture attachment.
         * Returns nullptr if not bound.
         */
        std::shared_ptr<Texture> GetDepthStencilAttachment() const;

        /*
         * Check to see if attachments are allowed.
         * If false, no attachment switches can be made.
         */
        bool AllowsAttachments() const;

        //Virtual functions
    public:

        /*
         * Called when a color attachment is added to this render target.
         */
        virtual void OnColorAttachmentBound(std::uint16_t a_Slot, const std::shared_ptr<Texture>& a_Added) = 0;

        /*
         * Called when a depth stencil attachment is added to this render target.
         */
        virtual void OnDepthStencilAttachmentBound(const std::shared_ptr<Texture>& a_Added) = 0;

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
        virtual void SetViewPort(const glm::vec<4, std::uint32_t>& a_ViewPort) = 0;

        /*
         * Get the scissor rect of this RenderTarget.
         */
        virtual glm::vec4 GetScissorRect() = 0;

        /*
         * Set the scissor rect of this RenderTarget.
         */
        virtual void SetScissorRect(const glm::vec<4, std::uint32_t>& a_ScissorRect) = 0;

    protected:
        void OnLock() override;
        void OnUnlock() override;

        RenderTargetSettings m_Settings;

        //Bound textures.
        std::shared_ptr<Texture> m_DepthStencilAttachment;
        std::shared_ptr<Texture> m_ColorAttachments[MAX_NUM_COLOR_ATTACHMENTS];

        //The amount of color attachments currently bound.
        std::uint16_t m_NumColorAttachments;

        //Attachment switching is only allowed when this is true.
        bool m_AllowAttachments;
    };
}
