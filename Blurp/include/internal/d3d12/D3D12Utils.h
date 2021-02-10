#pragma once
#include <GL/glew.h>
#include <stdexcept>
#include <intsafe.h>

/*
 * This file contains utilities used to convert from Blurp to D3D12 interfaces and data types and vice versa.
 * Also other useful structures and functions.
 */

inline std::string HrToString(HRESULT hr)
{
    char s_str[64] = {};
    sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
    return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
    HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
    HRESULT Error() const { return m_hr; }
private:
    const HRESULT m_hr;
};

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw HrException(hr);
    }
}


inline DXGI_FORMAT ToD3D12(blurp::PixelFormat a_Format, blurp::DataType a_Type)
{
    using namespace blurp;
    switch (a_Format)
    {
    case PixelFormat::R:
    {
        switch (a_Type)
        {
        case DataType::FLOAT:
            return DXGI_FORMAT_R32G32_FLOAT;
        case DataType::BYTE:
            return DXGI_FORMAT_R8_UINT;
        case DataType::UBYTE:
            return DXGI_FORMAT_R8_SINT;
        case DataType::INT:
            return DXGI_FORMAT_R32_SINT;
    //    case DataType::UINT:
    //        return DXGI_FORMAT_;
    //    case DataType::SHORT:
    //        return DXGI_FORMAT_;
    //    case DataType::USHORT:
    //        return DXGI_FORMAT_;
    //    }
    //}
    //break;
    //case PixelFormat::RG:
    //{
    //    switch (a_Type)
    //    {
    //    case DataType::FLOAT:
    //        return DXGI_FORMAT_;
    //    case DataType::BYTE:
    //        return DXGI_FORMAT_;
    //    case DataType::UBYTE:
    //        return DXGI_FORMAT_;
    //    case DataType::INT:
    //        return DXGI_FORMAT_;
    //    case DataType::UINT:
    //        return DXGI_FORMAT_;
    //    case DataType::SHORT:
    //        return DXGI_FORMAT_;
    //    case DataType::USHORT:
    //        return DXGI_FORMAT_;
    //    }
    //}
    //break;
    //case PixelFormat::RGB:
    //{
    //    switch (a_Type)
    //    {
    //    case DataType::FLOAT:
    //        return DXGI_FORMAT_;
    //    case DataType::BYTE:
    //        return DXGI_FORMAT_;
    //    case DataType::UBYTE:
    //        return DXGI_FORMAT_;
    //    case DataType::INT:
    //        return DXGI_FORMAT_;
    //    case DataType::UINT:
    //        return DXGI_FORMAT_;
    //    case DataType::SHORT:
    //        return DXGI_FORMAT_;
    //    case DataType::USHORT:
    //        return DXGI_FORMAT_;
    //    }
    //}
    //break;
    //case PixelFormat::RGBA:
    //{
    //    switch (a_Type)
    //    {
    //    case DataType::FLOAT:
    //        return DXGI_FORMAT_;
    //    case DataType::BYTE:
    //        return DXGI_FORMAT_;
    //    case DataType::UBYTE:
    //        return DXGI_FORMAT_;
    //    case DataType::INT:
    //        return DXGI_FORMAT_;
    //    case DataType::UINT:
    //        return DXGI_FORMAT_;
    //    case DataType::SHORT:
    //        return DXGI_FORMAT_;
    //    case DataType::USHORT:
    //        return DXGI_FORMAT_;
    //    }
    //}
    //break;
    //case PixelFormat::DEPTH:
    //{
    //    switch (a_Type)
    //    {
    //    case DataType::INT:
    //    case DataType::UINT:
    //        return DXGI_FORMAT_;
    //    case DataType::FLOAT:
    //        return DXGI_FORMAT_;
    //    case DataType::SHORT:
    //    case DataType::USHORT:
    //        return DXGI_FORMAT_;
    //    }
    //}
    //break;
    //case PixelFormat::DEPTH_STENCIL:
    //{
    //    switch (a_Type)
    //    {
    //    case DataType::INT:
    //    case DataType::UINT:
    //    case DataType::FLOAT:
    //        return DXGI_FORMAT_;
        }
    }
    break;
    }

    throw std::exception("Error: Could not convert pixel format and data type to sized OpenGL type.");
    //return 0;
}