#pragma once
#include <Window.h>
#include <Camera.h>
#include <RenderPass_Clear.h>
#include <RenderPass_Forward.h>
#include <RenderPass_Skybox.h>
#include <RenderPass_ShadowMap.h>
#include "MeshLoader.h"
#include "Mesh.h"
#include "Entity.h"
#include "MemoryPool.h"

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
    void UpdateGame(float a_DeltaTime);

    /*
     * Render the game.
     */
    void Render();

    /*
     * Create an entity.
     */
    Entity* CreateEntity(EntityType a_Type, int a_MeshId)
    {
        switch (a_Type)
        {
        case EntityType::SPACE_SHIP:
        {
            Entity* ptr = m_SpaceShips.allocate<SpaceShip>((a_MeshId));
            m_Entities.push_back(std::make_pair(ptr, &m_SpaceShips));
            return ptr;
        }
        case EntityType::LASER:
        {
            Entity* ptr = m_Lasers.allocate<Laser>((a_MeshId));
            m_Entities.push_back(std::make_pair(ptr, &m_Lasers));
            return ptr;
        }
        case EntityType::KILL_BOT:
        {
            Entity* ptr = m_KillBots.allocate<KillBot>((a_MeshId));
            m_Entities.push_back(std::make_pair(ptr, &m_KillBots));
            return ptr;
        }
        case EntityType::ASTEROID:
        {
            Entity* ptr = m_Asteroids.allocate<Asteroid>((a_MeshId));
            m_Entities.push_back(std::make_pair(ptr, &m_Asteroids));
            return ptr;
        }
        case EntityType::PLANET:
        {
            Entity* ptr = m_Planets.allocate<Planet>((a_MeshId));
            m_Entities.push_back(std::make_pair(ptr, &m_Planets));
            return ptr;
        }
        case EntityType::LIGHT:
        {
            Entity* ptr = m_Lights.allocate<Light>();
            m_Entities.push_back(std::make_pair(ptr, &m_Lights));
            return ptr;
        }
        }
        assert(0 && "No known entity type.");
        return nullptr;
    }

public:
    /*
     * GLOBAL
     *
     * Publicly accessible for object construction.
     */
    blurp::BlurpEngine& m_Engine;

private:

    /*
     * GAMEPLAY RELATED
     */
     //All meshes used by the game.
    std::vector<Mesh> m_Meshes;

    //Memory pools for each object type.
    utilities::TypelessPool m_SpaceShips;
    utilities::TypelessPool m_Asteroids;
    utilities::TypelessPool m_KillBots;
    utilities::TypelessPool m_Planets;
    utilities::TypelessPool m_Lasers;
    utilities::TypelessPool m_Lights;

    //All entities as pointers and their respective pool.
    std::vector<std::pair<Entity*, utilities::TypelessPool*>> m_Entities;

    /*
     * RENDERING RELATED
     */
    std::shared_ptr<blurp::RenderPipeline> m_Pipeline;
    std::shared_ptr<blurp::RenderPass_Forward> m_ForwardPass;
    std::shared_ptr<blurp::RenderPass_Clear> m_ClearPass;
    std::shared_ptr<blurp::RenderPass_Skybox> m_SkyboxPass;
    std::shared_ptr<blurp::RenderPass_ShadowMap> m_ShadowGenerationPass;
    std::shared_ptr<blurp::Texture> m_PosShadowArray;
    std::shared_ptr<blurp::Texture> m_DirShadowArray;
    std::shared_ptr<blurp::GpuBufferView> m_DirLightMatView;
    std::shared_ptr<blurp::GpuBufferView> m_DirLightDataOffsetView;
    std::shared_ptr<blurp::Texture> m_SkyBoxTexture;
    std::shared_ptr<blurp::Camera> m_Camera;

    //Lights
    std::shared_ptr<blurp::DirectionalLight> m_Sun;

    //Scene data and buffers
    std::shared_ptr<blurp::GpuBuffer> m_GpuBuffer;
};