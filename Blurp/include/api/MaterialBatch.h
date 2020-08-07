#pragma once
#include "RenderResource.h"

namespace blurp
{
    class MaterialBatch : public RenderResource
    {
    public:
        MaterialBatch(const MaterialBatchSettings& a_Settings) : m_Settings(a_Settings) {}

        /*
         * Return the 16bit bitmask.
         */
        std::uint16_t GetMask() const
        {
            return m_Settings.GetMask();
        }

    protected:
        MaterialBatchSettings m_Settings;
    };
}