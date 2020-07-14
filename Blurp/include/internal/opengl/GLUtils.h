#pragma once
#include "Settings.h"
#include <GL/glew.h>

namespace blurp
{
    static constexpr GLenum MAG_TYPES[]{ GL_NEAREST, GL_LINEAR };
    static constexpr GLenum TEXTURE_TYPES[]{ GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_2D_ARRAY };
    static constexpr GLenum MIN_TYPES[]{ GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR};
    static constexpr GLenum WRAP_TYPES[]{ GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT, GL_REPEAT };

    static constexpr GLenum PIXEL_FORMATS[]{ GL_RED, GL_RG, GL_RGB, GL_RGBA, GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL};
    static constexpr GLenum DATA_FORMATS[]{ GL_FLOAT, GL_INT, GL_UNSIGNED_INT, GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT };

    /*
     * Map MagFilterType to the corresponding GLenum value.
     */
    inline constexpr GLenum ToGL(MagFilterType a_Filter)
    {
        return MAG_TYPES[static_cast<int>(a_Filter)];
    }

    inline constexpr GLenum ToGL(TextureType a_Type)
    {
        return TEXTURE_TYPES[static_cast<int>(a_Type)];
    }

    /*
     * Map MinFilterType to the corresponding GLenum value.
     */
    inline constexpr GLenum ToGL(MinFilterType a_Filter)
    {
        return MIN_TYPES[static_cast<int>(a_Filter)];
    }

    /*
     * Map PixelFormat to the corresponding GLenum value.
     */
    inline constexpr GLenum ToGL(PixelFormat a_Format)
    {
        return PIXEL_FORMATS[static_cast<int>(a_Format)];
    }

    /*
     * Map DataType to the corresponding GLenum value.
     */
    inline constexpr GLenum ToGL(DataType a_Data)
    {
        return DATA_FORMATS[static_cast<int>(a_Data)];
    }

    /*
     * Map WrapMode to the corresponding GLenum value.
     */
    inline constexpr GLenum ToGL(WrapMode a_Data)
    {
        return WRAP_TYPES[static_cast<int>(a_Data)];
    }

    /*
     * Convert an internal format and datatype to the correct sized opengl type.
     * If a format could not be found, an exception is thrown.
     */
    inline constexpr GLenum ToSizedFormat(PixelFormat a_Format, DataType a_DataType)
    {
        switch (a_Format)
        {
        case PixelFormat::R:
            {
            switch (a_DataType)
            {
            case DataType::FLOAT:
                return GL_R32F;
            case DataType::BYTE:
                return GL_R8;
            case DataType::UBYTE:
                return GL_R8UI;
            case DataType::INT:
                return GL_R32I;
            case DataType::UINT:
                return GL_R32UI;
            case DataType::SHORT:
                return GL_R16I;
            case DataType::USHORT:
                return GL_R16UI;
            }
            }
            break;
        case PixelFormat::RG:
        {
            switch (a_DataType)
            {
            case DataType::FLOAT:
                return GL_RG32F;
            case DataType::BYTE:
                return GL_RG8;
            case DataType::UBYTE:
                return GL_RG8UI;
            case DataType::INT:
                return GL_RG32I;
            case DataType::UINT:
                return GL_RG32UI;
            case DataType::SHORT:
                return GL_RG16I;
            case DataType::USHORT:
                return GL_RG16UI;
            }
        }
            break;
        case PixelFormat::RGB:
        {
            switch (a_DataType)
            {
            case DataType::FLOAT:
                return GL_RGB32F;
            case DataType::BYTE:
                return GL_RGB8;
            case DataType::UBYTE:
                return GL_RGB8UI;
            case DataType::INT:
                return GL_RGB32I;
            case DataType::UINT:
                return GL_RGB32UI;
            case DataType::SHORT:
                return GL_RGB16I;
            case DataType::USHORT:
                return GL_RGB16UI;
            }
        }
            break;
        case PixelFormat::RGBA:
        {
            switch (a_DataType)
            {
            case DataType::FLOAT:
                return GL_RGBA32F;
            case DataType::BYTE:
                return GL_RGBA8;
            case DataType::UBYTE:
                return GL_RGBA8UI;
            case DataType::INT:
                return GL_RGBA32I;
            case DataType::UINT:
                return GL_RGBA32UI;
            case DataType::SHORT:
                return GL_RGBA16I;
            case DataType::USHORT:
                return GL_RGBA16UI;
            }
        }
            break;
        case PixelFormat::DEPTH:
        {
            switch (a_DataType)
            {
            case DataType::INT:
            case DataType::UINT:
            case DataType::FLOAT:
                return GL_DEPTH_COMPONENT32F;
            case DataType::SHORT:
            case DataType::USHORT:
                return GL_DEPTH_COMPONENT16;
            }
        }
            break;
        case PixelFormat::DEPTH_STENCIL:
        {
            switch (a_DataType)
            {
            case DataType::INT:
            case DataType::UINT:
            case DataType::FLOAT:
                return GL_DEPTH24_STENCIL8;
            }
        }
            break;
        }

        throw std::exception("Error: Could not convert pixel format and data type to sized OpenGL type.");
        return 0;
    }
}
