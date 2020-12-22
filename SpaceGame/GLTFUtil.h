#pragma once
#include <cinttypes>
#include <cassert>

namespace fx
{
    namespace gltf
    {
        struct Document;
        struct Accessor;
        struct Attribute;
    }
}

/*
 * Struct containing information about a buffer.
 */
struct BufferInfo
{
    BufferInfo() : accessor(nullptr), data(nullptr), dataSize(0), totalSize(0), emptySpace(0), numElements(0) {}

    BufferInfo(fx::gltf::Accessor const* accessor, uint8_t const* data, uint32_t dataSize, uint32_t totalSize, uint32_t emptySpace, uint32_t numElements) :
        accessor(accessor),
        data(data),
        dataSize(dataSize),
        totalSize(totalSize),
        emptySpace(emptySpace),
        numElements(numElements)
    {}

    fx::gltf::Accessor const* accessor;

    //Start of the data.
    uint8_t const* data;

    //The size of each data element in bytes.
    uint32_t dataSize;

    //The total size of all elements together in bytes.
    uint32_t totalSize;

    //Empty space between objects. Measured from the end of one object to the start of the next.
    uint32_t emptySpace;

    //The amount of elements.
    uint32_t numElements;

    bool HasData() const noexcept
    {
        return data != nullptr;
    }

    /*
     * Get the element at the given index.
     */
    template<typename T>
    T const* GetElement(uint32_t index)
    {
        //Make sure there is data, and that this type is the correct type.
        //Also check for out of bounds.
        assert(HasData() && "Error! No data in this BufferInfo object.");
        assert(index < numElements && "Error! Index out of bounds for BufferInfo.");

        //Calculate the position of the object required and return it.
        return reinterpret_cast<T const*>(reinterpret_cast<size_t>(data) + (static_cast<size_t>(index)* static_cast<size_t>(emptySpace)) + (static_cast<size_t>(index)* static_cast<size_t>(dataSize)));
    };
};

class GLTFUtil
{
public:

    static BufferInfo ReadBufferData(const fx::gltf::Document& file, std::int32_t a_AttribIndex);

    /*
     * Calculate the size of an accessor.
     */
    static std::uint32_t CalculateDataTypeSize(fx::gltf::Accessor const& accessor) noexcept;
};
