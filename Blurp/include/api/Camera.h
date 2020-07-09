#pragma once
#include "RenderResource.h"

namespace blurp
{
    class Camera : public RenderResource
    {
    public:
        Camera(const CameraSettings& a_Settings) : m_Settings(a_Settings) {}

    protected:
        CameraSettings m_Settings;
    };
}
