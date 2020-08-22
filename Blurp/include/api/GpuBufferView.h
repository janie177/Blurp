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
            totalSize = 0;
            elementSize = 0;
            end = 0;
        }

        GpuBufferView(std::uintptr_t a_Start, std::uintptr_t a_Size, std::uintptr_t a_ElementSize)
            : start(a_Start), totalSize(a_Size), elementSize(a_ElementSize), end(a_Start + a_Size)
        {
        }

        /*
         * Create a GpuBufferView spanning from the first to last element index provided.
         * Both elements are inclusive.
         *
         * An exception is thrown if the elements are not within bounds of the original GpuBufferView.
         */
        GpuBufferView CreateSubView(std::uint32_t a_FirstElement, std::uint32_t a_LastElement) const;

        //Pointer to the start of the data added in the GPU buffer.
        //This is measured as an offset from the start of the buffer.
        std::uintptr_t start;

        //The total size in bytes of the data in this view.
        std::uintptr_t totalSize;

        //Stride in bytes between each element on the GPU with padding.
        //This is equal to the padded size of each element.
        std::uintptr_t elementSize;

        //End of the data. Automatically calculated based on start and size.
        std::uintptr_t end;
    };

    inline GpuBufferView GpuBufferView::CreateSubView(std::uint32_t a_FirstElement, std::uint32_t a_LastElement) const
    {
        assert(a_FirstElement <= a_LastElement && "First element has to be smaller than or equal to the last one!");

        const std::uintptr_t newStart = start + (a_FirstElement * elementSize);
        const auto numElements = (a_LastElement - a_FirstElement) + 1;
        const std::uintptr_t newSize = numElements * elementSize;

        //Ensure within bounds.
        assert(newStart + newSize <= start + totalSize && "Data range exceeded for GpuBufferView! Last element out of range?");

        return GpuBufferView(newStart, newSize, elementSize);
    }
}
