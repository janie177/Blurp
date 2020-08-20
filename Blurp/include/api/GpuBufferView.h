#pragma once
#include <cinttypes>

namespace blurp
{
    /*
     * Struct containing information about a GPU Buffer.
     * The offsets into the buffer allow data to be read from the GPU.
     */
    struct GpuBufferView
    {
        GpuBufferView()
        {
            start = 0;
            size = 0;
            stride = 0;
            end = 0;
        }

        GpuBufferView(std::uintptr_t a_Start, std::uintptr_t a_Size, std::uintptr_t a_Stride, std::uintptr_t a_End)
            : start(a_Start), size(a_Size), stride(a_Stride), end(a_End)
        {
        }

        //Pointer to the start of the data added in the GPU buffer.
        //This is measured as an offset from the start of the buffer.
        std::uintptr_t start;

        //The total size in bytes of the data between start and end.
        std::uintptr_t size;

        //Stride in bytes between each element on the GPU with padding.
        std::uintptr_t stride;

        //Pointer to the end of the data added in the GPU buffer.
        //This is measured as the offset from the start of the buffer.
        std::uintptr_t end;
    };
}