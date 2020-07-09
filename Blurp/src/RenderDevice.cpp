#include "RenderDevice.h"

#include <utility>
#include "Window.h"
#include "SwapChain.h"

namespace blurp
{
    void RenderDevice::BindWindowAndSwapChain(BlurpEngine& a_BlurpEngine, Window* a_Window, std::shared_ptr<SwapChain> a_SwapChain) const
    {
        if(a_Window != nullptr)
        {
            a_SwapChain->Load(a_BlurpEngine);
            a_Window->BindSwapChain(std::move(a_SwapChain));
        }
    }
}
