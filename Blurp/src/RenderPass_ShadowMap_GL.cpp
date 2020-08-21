#include "opengl/RenderPass_ShadowMap_GL.h"

namespace blurp
{
    bool RenderPass_ShadowMap_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        //TODO setup shader cache with just the vertex attribute and input data.

        //TODO set up FBO.

        //TODO

        return true;
    }

    bool RenderPass_ShadowMap_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        //TODO delete fbo.

        return true;
    }

    void RenderPass_ShadowMap_GL::Execute()
    {
        //TODO assert that the index of each member does not exceed the num layers in the array texture.

        //TODO point and spot light.
        //TODO bind as many as possible to the fbo at a time. Then draw the geometry.







        //TODO directional light with cascading.
    }
}
