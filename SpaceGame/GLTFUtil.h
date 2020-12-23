#pragma once
#include <cinttypes>
#include <cassert>
#include <string>
#include <fx/gltf.h>

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

class ImageData
{
public:
    struct ImageInfo
    {
        std::string FileName{};

        uint32_t BinarySize{};
        uint8_t const* BinaryData{};

        bool IsBinary() const noexcept
        {
            return BinaryData != nullptr;
        }
    };

    explicit ImageData(std::string const& texture)
    {
        m_info.FileName = texture;
    }

    ImageData(fx::gltf::Document const& doc, std::size_t textureIndex, std::string const& modelPath)
    {
        fx::gltf::Image const& image = doc.images[doc.textures[textureIndex].source];

        const bool isEmbedded = image.IsEmbeddedResource();
        if (!image.uri.empty() && !isEmbedded)
        {
            m_info.FileName = fx::gltf::detail::GetDocumentRootPath(modelPath) + "/" + image.uri;
        }
        else
        {
            if (isEmbedded)
            {
                image.MaterializeData(m_embeddedData);
                m_info.BinaryData = &m_embeddedData[0];
                m_info.BinarySize = static_cast<uint32_t>(m_embeddedData.size());
            }
            else
            {
                fx::gltf::BufferView const& bufferView = doc.bufferViews[image.bufferView];
                fx::gltf::Buffer const& buffer = doc.buffers[bufferView.buffer];

                m_info.BinaryData = &buffer.data[bufferView.byteOffset];
                m_info.BinarySize = bufferView.byteLength;
            }
        }
    }

    ImageInfo const& Info() const noexcept
    {
        return m_info;
    }

private:
    ImageInfo m_info{};

    std::vector<uint8_t> m_embeddedData{};
};