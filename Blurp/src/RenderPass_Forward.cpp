#include "RenderPass_Forward.h"

namespace blurp
{
    RenderPassType RenderPass_Forward::GetType()
    {
        return RenderPassType::RP_FORWARD;
    }

    void RenderPass_Forward::Reset()
    {
        //TODO
    }

    bool RenderPass_Forward::IsStateValid()
    {
        //TODO
        return true;
    }
}
