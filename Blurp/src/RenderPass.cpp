#include "RenderPass.h"

namespace blurp
{
    void RenderPass::SetEnabled(bool a_Enabled)
    {
        m_Enabled = a_Enabled;
    }

    bool RenderPass::IsEnabled() const
    {
        return m_Enabled;
    }
}
