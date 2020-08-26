#include "RenderTarget.h"

#include <utility>

#include "Texture.h"

namespace blurp
{
    std::uint16_t RenderTarget::GetNumColorAttachments() const
    {
        return m_NumColorAttachments;
    }

    std::uint16_t RenderTarget::GetMaxColorAttachments() const
    {
        return MAX_NUM_COLOR_ATTACHMENTS;
    }

    bool RenderTarget::HasColorAttachment() const
    {
        return m_NumColorAttachments != 0;
    }

    bool RenderTarget::HasDepthAttachment() const
    {
        return m_DepthStencilAttachment != nullptr;
    }

    bool RenderTarget::HasStencilAttachment() const
    {
        return m_DepthStencilAttachment != nullptr && m_DepthStencilAttachment->GetPixelFormat() == PixelFormat::DEPTH_STENCIL;
    }

    bool RenderTarget::SetColorAttachment(std::uint16_t a_Slot, std::shared_ptr<Texture> a_Attachment)
    {
        assert(!IsLocked() && "Cannot switch attachments in a RenderTarget that is currently locked!");
        assert(a_Attachment != nullptr && "Error: Trying to set null as color attachment.");
        assert(m_AllowAttachments && "This RenderTarget does not allow you to switch attachments!");
        assert(a_Slot <= MAX_NUM_COLOR_ATTACHMENTS && "Index for color attachments out of bounds!");
        assert(a_Attachment->GetAccessMode() == AccessMode::READ_WRITE && "Textures attached to a render target must be READ_WRITE.");
        assert((a_Attachment->GetTextureType() == TextureType::TEXTURE_1D || a_Attachment->GetTextureType() == TextureType::TEXTURE_2D) && "Only 1D and 2D textures can be added to a render target as attachment.");

        //Notify the implementation of a change.
        OnColorAttachmentBound(a_Slot, a_Attachment);

        //Add the attachment.
        m_ColorAttachments[a_Slot] = std::move(a_Attachment);

        //Recalculate the size.
        m_NumColorAttachments = 0;
        for (const auto& colorAttachment : m_ColorAttachments)
        {
            if (colorAttachment != nullptr)
            {
                ++m_NumColorAttachments;
            }
        }

        return true;
    }

    bool RenderTarget::SetDepthStencilAttachment(std::shared_ptr<Texture> a_Attachment)
    {
        assert(!IsLocked() && "Cannot switch attachments in a RenderTarget that is currently locked!");
        assert(a_Attachment != nullptr && "Error: Trying to set null as color attachment.");
        assert(m_AllowAttachments && "This RenderTarget does not allow you to switch attachments!");
        assert(a_Attachment->GetPixelFormat() == PixelFormat::DEPTH || a_Attachment->GetPixelFormat() == PixelFormat::DEPTH_STENCIL);
        assert(a_Attachment->GetAccessMode() == AccessMode::READ_WRITE && "Textures attached to a render target must be READ_WRITE.");
        assert((a_Attachment->GetTextureType() == TextureType::TEXTURE_1D || a_Attachment->GetTextureType() == TextureType::TEXTURE_2D) && "Only 1D and 2D textures can be added to a render target as attachment.");

        //Notify the implementation of a change.
        OnDepthStencilAttachmentBound(a_Attachment);
        m_DepthStencilAttachment = std::move(a_Attachment);
        return true;
    }

    std::shared_ptr<Texture> RenderTarget::GetColorAttachment(std::uint16_t a_Slot)
    {
        assert(a_Slot <= MAX_NUM_COLOR_ATTACHMENTS && "Index for color attachments out of bounds!");
        return m_ColorAttachments[a_Slot];
    }

    std::shared_ptr<Texture> RenderTarget::GetDepthStencilAttachment() const
    {
        return m_DepthStencilAttachment;
    }

    bool RenderTarget::AllowsAttachments() const
    {
        return m_AllowAttachments;
    }

    void RenderTarget::OnLock()
    {

    }

    void RenderTarget::OnUnlock()
    {

    }
}
