#pragma once
#include <Window.h>
#include <Camera.h>
#include <RenderPass_Clear.h>
#include <RenderPass_Forward.h>
#include <RenderPass_Skybox.h>
#include <RenderPass_ShadowMap.h>
#include "MeshLoader.h"

/*
 * This is a game! Games are fun! 
 */

class Game
{
public:
    Game(blurp::BlurpEngine& a_RenderEngine);

    /*
     * Setup the game and related systems.
     */
    void Init();

    /*
     * Update user input.
     */
    void UpdateInput(std::shared_ptr<blurp::Window>& a_Window);

    /*
     * Update game logic.
     */
    void UpdateGame();

    /*
     * Render the game.
     */
    void Render();

private:
    /*
     * GLOBAL
     */
    blurp::BlurpEngine& m_Engine;

    /*
     * GAMEPLAY RELATED
     */
    blurp::Transform m_ObjectTransform;

    /*
     * RENDERING RELATED
     */
    std::shared_ptr<blurp::RenderPipeline> m_Pipeline;
    std::shared_ptr<blurp::RenderPass_Forward> m_ForwardPass;
    std::shared_ptr<blurp::RenderPass_Clear> m_ClearPass;
    std::shared_ptr<blurp::RenderPass_Skybox> m_SkyboxPass;

    //Shadow mapping
    std::shared_ptr<blurp::RenderPass_ShadowMap> m_ShadowGenerationPass;
    std::shared_ptr<blurp::Texture> m_PosShadowArray;
    std::shared_ptr<blurp::Texture> m_DirShadowArray;
    std::shared_ptr<blurp::GpuBufferView> m_DirLightMatView;
    std::shared_ptr<blurp::GpuBufferView> m_DirLightDataOffsetView;

    std::shared_ptr<blurp::Texture> m_SkyBoxTexture;
    std::shared_ptr<blurp::Camera> m_Camera;

    //Lights
    std::vector<std::shared_ptr<blurp::PointLight>> m_Lights;
    std::shared_ptr<blurp::DirectionalLight> m_Sun;

    //Scene data and buffers
    std::shared_ptr<blurp::GpuBuffer> m_TransformBuffer;
    float m_TransformEnd;
    GLTFScene m_Scene;

    //All draw datas, transparent at the back.
    std::vector<blurp::DrawData> drawDatas;
    std::uint32_t m_TransparentStart;

};