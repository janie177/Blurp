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

    class RenderResourceManager;

    class RenderResource;
    class RenderDevice;
    class RenderPipeline;


    class BlurpEngine
    {
        friend class RenderDevice;
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
         * Get the resource manager instance. 
         */
        RenderResourceManager& GetResourceManager() const;

    protected:
        /*
         * Register a resource with the resource registry.
         * This loads the resource as well.
         */
        void RegisterResource(const std::shared_ptr<RenderResource>& a_Resource);

    private:
        //Registry containing all live resources.
        std::vector<std::shared_ptr<RenderResource>> m_Resources;

        //The render device containing the rendering context.
        std::shared_ptr<RenderDevice> m_RenderDevice;

        //The window belonging to this instance of BlurpEngine.
        //This is nullptr if the initial settings specified WindowType::NONE.
        std::shared_ptr<Window> m_Window;

        //The resource manager instance
        std::unique_ptr<RenderResourceManager> m_ResourceManager;
    };
}
