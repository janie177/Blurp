#include "TriangleScene.h"
#include <RenderPipeline.h>
#include <BlurpEngine.h>
#include <RenderResourceManager.h>
#include <Data.h>

void TriangleScene::Init()
{
    using namespace blurp;
    //Set up a pipeline that draws a triangle onto a texture. Then it draws a triangle with the first texture on it on the 2nd triangle on the screen.
    PipelineSettings pSettings;
    pSettings.waitForGpu = true;
    m_Pipeline = m_Engine.GetResourceManager().CreatePipeline(pSettings);

    //Add a clear pass first.
    m_ClearPass = m_Pipeline->AppendRenderPass<RenderPass_Clear>(RenderPassType::RP_CLEAR);
    m_TrianglePass = m_Pipeline->AppendRenderPass<RenderPass_HelloTriangle>(RenderPassType::RP_HELLOTRIANGLE);

    //Set the clear color.
    auto renderTarget = m_Window->GetRenderTarget();
    renderTarget->SetClearColor({ 0.f, 1.f, 1.f, 1.f });
    m_ClearPass->AddRenderTarget(renderTarget);

    //Draw a yellow triangle on the screen.
    m_TrianglePass->SetTarget(renderTarget);
    m_TrianglePass->SetColor(glm::vec4(1.f, 1.f, 0.f, 1.f));
}

void TriangleScene::Update()
{
    using namespace blurp;

    auto input = m_Window->PollInput();

    KeyboardEvent kEvent;
    MouseEvent mEvent;

    while (input.getNextEvent(kEvent))
    {
        //Nothing here.
    }
    while (input.getNextEvent(mEvent))
    {
        //Nothing here.
    }

    m_TrianglePass->Reset();

    m_Pipeline->Execute();

    while (true)
    {
        if (m_Pipeline->HasFinishedExecuting())
        {
            break;
        }
    }
}
