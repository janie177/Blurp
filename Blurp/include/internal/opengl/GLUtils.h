#pragma once
#include "Settings.h"
#include <GL/glew.h>

namespace blurp
{
    static constexpr GLenum MAG_TYPES[]{ GL_NEAREST, GL_LINEAR };
    static constexpr GLenum TEXTURE_TYPES[]{ GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_2D_ARRAY };
    static constexpr GLenum MIN_TYPES[]{ GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR };
    static constexpr GLenum WRAP_TYPES[]{ GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT, GL_REPEAT };

    static constexpr GLenum PIXEL_FORMATS[]{ GL_RED, GL_RG, GL_RGB, GL_RGBA, GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL };
    static constexpr GLenum DATA_FORMATS[]{ GL_FLOAT, GL_INT, GL_BYTE, GL_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT };
    static constexpr GLenum MEMORY_USAGE[]{ GL_DYNAMIC_READ, GL_DYNAMIC_DRAW, GL_STATIC_DRAW};

    static constexpr GLenum COMPARISON_FUNCTION[]{ GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS };
    static constexpr GLenum STENCIL_OPERATION[]{GL_KEEP, GL_ZERO, GL_REPLACE, GL_INCR_WRAP, GL_DECR_WRAP, GL_INVERT, GL_INCR, GL_DECR};
    static constexpr GLenum CULL_MODE[]{GL_NONE, GL_FRONT, GL_BACK};
    static constexpr GLenum FACE[]{ GL_FRONT, GL_BACK };
    static constexpr GLenum WINDING_ORDER[]{GL_CW, GL_CCW};
    static constexpr GLenum TOPOLOGY_TYPE[]{GL_POINTS, GL_LINES, GL_TRIANGLES, GL_LINE_STRIP, GL_TRIANGLE_STRIP};

    static constexpr GLenum BLEND_FUNCTION[]{GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA_SATURATE, GL_SRC1_COLOR, GL_ONE_MINUS_SRC1_COLOR, GL_SRC1_ALPHA, GL_ONE_MINUS_SRC1_ALPHA};
    static constexpr GLenum BLEND_OPERATION[]{GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT, GL_MIN, GL_MAX};

    static constexpr size_t SIZES[]{sizeof(float), sizeof(int), sizeof(char), sizeof(short), sizeof(unsigned int), sizeof(unsigned char), sizeof(unsigned short)};


    /*
     * Get the OpenGL enum type for access modes.
     * Returns STATIC_DRAW or DYNAMIC_DRAW.
     * These are used to determine in which type of video memory something should reside.
     */
    inline constexpr GLenum ToGL(MemoryUsage a_Usage)
    {
        return MEMORY_USAGE[static_cast<int>(a_Usage)];
    }

    inline constexpr GLenum ToGL(ComparisonFunction a_ComparisonFunction)
    {
        return COMPARISON_FUNCTION[static_cast<int>(a_ComparisonFunction)];
    }

    inline constexpr GLenum ToGL(TopologyType a_Topology)
    {
        return TOPOLOGY_TYPE[static_cast<int>(a_Topology)];
    }

    inline constexpr GLenum ToGL(StencilOperation a_Operation)
    {
        return STENCIL_OPERATION[static_cast<int>(a_Operation)];
    }

    inline constexpr GLenum ToGL(WindingOrder a_Winding)
    {
        return WINDING_ORDER[static_cast<int>(a_Winding)];
    }

    inline constexpr GLenum ToGL(CullMode a_CullMode)
    {
        return CULL_MODE[static_cast<int>(a_CullMode)];
    }

    inline constexpr GLenum ToGL(Face a_Face)
    {
        return FACE[static_cast<int>(a_Face)];
    }

    inline constexpr GLenum ToGL(BlendType a_Type)
    {
        return BLEND_FUNCTION[static_cast<int>(a_Type)];
    }

    inline constexpr GLenum ToGL(BlendOperation a_Operation)
    {
        return BLEND_OPERATION[static_cast<int>(a_Operation)];
    }

    /*
     * Convert a boolean to the OpenGL equivalent data type.
     */
    inline constexpr GLenum ToGL(bool a_Bool)
    {
        return a_Bool ? GL_TRUE : GL_FALSE;
    }

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
     * Retrieve the size in bytes of a data type.
     */
    inline constexpr std::size_t Size_Of(DataType a_Data)
    {
        return SIZES[static_cast<int>(a_Data)];
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
     *
     * NOTE:
     * OpenGL samplers will sample between {0, 1} for unsigned input, and between {-1, 1} for signed input.
     * Samplers return a float vec4 when the datatype is not suffixed with U or I.
     * This means that for RGB and UBYTE, I return GL_RGB8 instead of GL_RGB8UI.
     * This may seem unintuitive, but the reason is that otherwise in the shader I will get ints between 0 and 255.
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
                return GL_R8;
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
                return GL_RG8;
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
                return GL_RGB8;
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
                return GL_RGBA8;
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
                return GL_DEPTH_COMPONENT32;
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
