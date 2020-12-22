#include "GLTFUtil.h"
#include <nlohmann/json.hpp>
#include <fx/gltf.h>


BufferInfo GLTFUtil::ReadBufferData(const fx::gltf::Document& file, std::int32_t a_AttribIndex)
{
	assert(a_AttribIndex >= 0 && "Invalid attribute index!");
	const auto& accessor = file.accessors[a_AttribIndex];
	fx::gltf::BufferView const& bufferView = file.bufferViews[accessor.bufferView];
	fx::gltf::Buffer const& buffer = file.buffers[bufferView.buffer];
	const uint32_t dataTypeSize = CalculateDataTypeSize(accessor);
	auto emptySpace = bufferView.byteStride == 0 ? 0 : bufferView.byteStride - dataTypeSize;
	return BufferInfo(&accessor, &buffer.data[static_cast<uint64_t>(bufferView.byteOffset) + accessor.byteOffset], dataTypeSize, accessor.count* dataTypeSize, emptySpace, accessor.count);
}

std::uint32_t GLTFUtil::CalculateDataTypeSize(fx::gltf::Accessor const& accessor) noexcept
{
	uint32_t elementSize = 0;
	switch (accessor.componentType)
	{
	case fx::gltf::Accessor::ComponentType::Byte:
	case fx::gltf::Accessor::ComponentType::UnsignedByte:
		elementSize = 1;
		break;
	case fx::gltf::Accessor::ComponentType::Short:
	case fx::gltf::Accessor::ComponentType::UnsignedShort:
		elementSize = 2;
		break;
	case fx::gltf::Accessor::ComponentType::Float:
	case fx::gltf::Accessor::ComponentType::UnsignedInt:
		elementSize = 4;
		break;
	}

	switch (accessor.type)
	{
	case fx::gltf::Accessor::Type::Mat2:
		return 4 * elementSize;
		break;
	case fx::gltf::Accessor::Type::Mat3:
		return 9 * elementSize;
		break;
	case fx::gltf::Accessor::Type::Mat4:
		return 16 * elementSize;
		break;
	case fx::gltf::Accessor::Type::Scalar:
		return elementSize;
		break;
	case fx::gltf::Accessor::Type::Vec2:
		return 2 * elementSize;
		break;
	case fx::gltf::Accessor::Type::Vec3:
		return 3 * elementSize;
		break;
	case fx::gltf::Accessor::Type::Vec4:
		return 4 * elementSize;
		break;
	}

	return 0;
}