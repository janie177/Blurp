#include <Data.h>

namespace blurp
{
    //Initialize global ID counter.
    int PipelineState::m_IdCounter = 0;

    PipelineState PipelineState::Compile(const BlendData& a_BlendData, const TopologyType& a_Topology,
        const CullMode& a_CullMode, const WindingOrder& a_Winding, const DepthStencilData& a_DepthStencilData)
    {
        return PipelineState(a_BlendData, a_Topology, a_CullMode, a_Winding, a_DepthStencilData);
    }

    PipelineState& PipelineState::GetDefault()
    {
        static PipelineState defaultState;
        return defaultState;
    }
}
