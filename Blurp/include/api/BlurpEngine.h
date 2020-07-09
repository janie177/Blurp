#pragma once
#include <memory>
#include <vector>

namespace blurp
{
    //Structs to construct resources forward declarations.
    struct BlurpSettings;
    struct WindowSettings;
    struct CameraSettings;
    struct TextureSettings;
    struct MeshSettings;
    struct LightSettings;
    struct RenderTargetSettings;
    struct SwapChainSettings;
    struct MaterialSettings;

    //Resources forward declarations.
    class Window;
    class Light;
    class Camera;
    class Texture;
    class Mesh;
    class RenderTarget;
    class SwapChain;
    class Material;

    class RenderResource;
    class RenderDevice;


    class BlurpEngine
    {
        friend class RenderDevice_GL;
    public:
        BlurpEngine();

        /*
         * Initialize this instance of Blurp Engine.
         */
        bool Init(const BlurpSettings& a_Settings);

        /*
         * Get the instance of the window if created.
         * Only one window exists per BlurpEngine instance.
         * Inside BlurpSettings set WindowType to WindowType::NONE to not create a window.
         */
        std::shared_ptr<Window> GetWindow() const;

        /*
         * Create a Light resource and load it.
         * The resource is automatically added to the resource registry.
         */
        std::shared_ptr<Light> CreateLight(const LightSettings& a_Settings);

        /*
         * Create a Camera resource and load it.
         * The resource is automatically added to the resource registry.
         */
        std::shared_ptr<Camera> CreateCamera(const CameraSettings& a_Settings);

        /*
         * Create a Mesh resource and load it.
         * The resource is automatically added to the resource registry.
         */
        std::shared_ptr<Mesh> CreateMesh(const MeshSettings& a_Settings);

        /*
         * Create a Texture resource and load it.
         * The resource is automatically added to the resource registry.
         */
        std::shared_ptr<Texture> CreateTexture(const TextureSettings& a_Settings);

        /*
         * Create a RenderTarget resource and load it.
         * The resource is automatically added to the resource registry.
         */
        std::shared_ptr<RenderTarget> CreateRenderTarget(const RenderTargetSettings& a_Settings);

        /*
         * Create a Material resource and load it.
         * The resource is automatically added to the resource registry.
         */
        std::shared_ptr<Material> CreateMaterial(const MaterialSettings& a_Settings);

        /*
         * Clean up resources that have gone out of scope.
         * This erases them from the registry.
         */
        void CleanUp();

    private:
        //Registry containing all live resources.
        std::vector<std::shared_ptr<RenderResource>> m_Resources;

        //The render device containing the rendering context.
        std::shared_ptr<RenderDevice> m_RenderDevice;

        //The window belonging to this instance of BlurpEngine.
        //This is nullptr if the initial settings specified WindowType::NONE.
        std::shared_ptr<Window> m_Window;
    };
}
