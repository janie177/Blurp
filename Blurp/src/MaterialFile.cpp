#include "MaterialFile.h"
#include "Material.h"
#include "api/stb_image.h"
#include "Data.h"
#include "Settings.h"
#include <RenderResourceManager.h>

#include <fstream>
#include <iostream>


#include "lz4.h"
#include "lz4hc.h"

bool blurp::CreateMaterialFile(const MaterialInfo& a_MaterialInfo, const std::string& a_FileName)
{
	//Header part of the file, containing most information.
	MaterialHeader header;

	//Trailing data buffer.
	std::vector<char> data;

	//Resize to fit header.
	data.resize(sizeof(MaterialHeader));

	//Copy over the bitmask.
	header.mask = a_MaterialInfo.mask.GetMask();

	//Copy over the static data which is always there regardless of being used or not.
	header.diffuse.constantData = a_MaterialInfo.diffuse.constant;
	header.metalRoughnessAlpha.constantData.x = a_MaterialInfo.metallic.constant;
	header.metalRoughnessAlpha.constantData.y = a_MaterialInfo.roughness.constant;
	header.metalRoughnessAlpha.constantData.z = a_MaterialInfo.alpha.constant;
	header.emissive.constantData = a_MaterialInfo.emissive.constant;

	//DIFFUSE
	if (!a_MaterialInfo.diffuse.textureName.empty() && a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::DIFFUSE_TEXTURE))
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* image = stbi_load((a_MaterialInfo.path + a_MaterialInfo.diffuse.textureName).c_str(),
			&width,
			&height,
			&channels,
			3);
		
		if(image == nullptr)
		{
			std::cout << "Could not find texture provided." << std::endl;
			return false;
		}

		//Amount of bytes in this image.
		const long size = width * height * 3;

		header.diffuse.width = width;
		header.diffuse.height = height;
		header.diffuse.size = size;
		header.diffuse.start = data.size();

		data.insert(data.end(), image, image + size);

		stbi_image_free(image);
	}

	//NORMAL
	if (!a_MaterialInfo.normal.textureName.empty() && a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::NORMAL_TEXTURE))
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* image = stbi_load((a_MaterialInfo.path + a_MaterialInfo.normal.textureName).c_str(),
			&width,
			&height,
			&channels,
			3);

		if (image == nullptr)
		{
			std::cout << "Could not find texture provided." << std::endl;
			return false;
		}

		//Amount of bytes in this image.
		const long size = width * height * 3;

		header.normal.width = width;
		header.normal.height = height;
		header.normal.size = size;
		header.normal.start = data.size();

		data.insert(data.end(), image, image + size);

		stbi_image_free(image);
	}

	//EMISSIVE
	if (!a_MaterialInfo.emissive.textureName.empty() && a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::EMISSIVE_TEXTURE))
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* image = stbi_load((a_MaterialInfo.path + a_MaterialInfo.emissive.textureName).c_str(),
			&width,
			&height,
			&channels,
			3);

		if (image == nullptr)
		{
			std::cout << "Could not find texture provided." << std::endl;
			return false;
		}

		//Amount of bytes in this image.
		const long size = width * height * 3;

		header.emissive.width = width;
		header.emissive.height = height;
		header.emissive.size = size;
		header.emissive.start = data.size();

		data.insert(data.end(), image, image + size);

		stbi_image_free(image);
	}

	//METAL/ROUGHNESS/ALPHA
	if ((!a_MaterialInfo.metallic.textureName.empty() && a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::METALLIC_TEXTURE)) || (!a_MaterialInfo.roughness.textureName.empty() && a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::ROUGHNESS_TEXTURE)) || (!a_MaterialInfo.alpha.textureName.empty() && a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::ALPHA_TEXTURE)))
	{
		int w = 0, h = 0;
		stbi_set_flip_vertically_on_load(true);

		unsigned char* metal = nullptr;
		unsigned char* rough = nullptr;
		unsigned char* alpha = nullptr;

		//NOTE all textures need to be the same dimensions.

		if (!a_MaterialInfo.metallic.textureName.empty() && a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::METALLIC_TEXTURE))
		{
			int width, height, channels;
			metal = stbi_load((a_MaterialInfo.path + a_MaterialInfo.metallic.textureName).c_str(), &width, &height, &channels, STBI_grey);

			//Width and height have to be the same for each one. 
			if (metal == nullptr || (w != 0 && w != width) || (h != 0 && h != height))
			{
				std::cout << "Malformed texture found passed to material compilation." << std::endl;
				return false;
			}

			w = width;
			h = height;
		}
		if (!a_MaterialInfo.roughness.textureName.empty() && a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::ROUGHNESS_TEXTURE))
		{
			int width, height, channels;
			rough = stbi_load((a_MaterialInfo.path + a_MaterialInfo.roughness.textureName).c_str(), &width, &height, &channels, STBI_grey);

			//Width and height have to be the same for each one. 
			if (rough == nullptr || (w != 0 && w != width) || (h != 0 && h != height))
			{
				std::cout << "Malformed texture found passed to material compilation." << std::endl;
				return false;
			}

			w = width;
			h = height;
		}
		if (!a_MaterialInfo.alpha.textureName.empty() && a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::ALPHA_TEXTURE))
		{
			int width, height, channels;
			alpha = stbi_load((a_MaterialInfo.path + a_MaterialInfo.alpha.textureName).c_str(), &width, &height, &channels, STBI_grey);

			//Width and height have to be the same for each one. 
			if (alpha == nullptr || (w != 0 && w != width) || (h != 0 && h != height))
			{
				std::cout << "Malformed texture found passed to material compilation." << std::endl;
				return false;
			}

			w = width;
			h = height;
		}

		const auto size = w * h * 3;
	    std::vector<unsigned char> mraData;
		mraData.resize(size);

		//Interleave the metal/rough/alpha channels.
		for (unsigned i = 0; i < static_cast<unsigned>(w) * static_cast<unsigned>(h); ++i)
		{
			if (metal != nullptr) mraData[i * 3 + 0] = metal[i];
			if (rough != nullptr) mraData[i * 3 + 1] = rough[i];
			if (alpha != nullptr) mraData[i * 3 + 2] = alpha[i];
		}

		header.metalRoughnessAlpha.width = w;
		header.metalRoughnessAlpha.height = h;
		header.metalRoughnessAlpha.size = size;
		header.metalRoughnessAlpha.start = data.size();

		data.insert(data.end(), mraData.begin(), mraData.end());


		if (metal)
		{
			stbi_image_free(metal);
		}
		if (rough)
		{
			stbi_image_free(rough);
		}
		if (alpha)
		{
			stbi_image_free(alpha);
		}
	}

	//AMBIENT_OCCLUSION/HEIGHT
	if ((!a_MaterialInfo.height.textureName.empty() && a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::HEIGHT_TEXTURE)) || (!a_MaterialInfo.ao.textureName.empty() && a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::OCCLUSION_TEXTURE)))
	{
		int w = 0, h = 0;
		stbi_set_flip_vertically_on_load(true);

		unsigned char* ao = nullptr;
		unsigned char* heightMap = nullptr;

		//NOTE all textures need to be the same dimensions.

		if (!a_MaterialInfo.ao.textureName.empty())
		{
			int width, height, channels;
			ao = stbi_load((a_MaterialInfo.path + a_MaterialInfo.ao.textureName).c_str(), &width, &height, &channels, STBI_grey);

			//Width and height have to be the same for each one. 
			if (ao == nullptr || (w != 0 && w != width) || (h != 0 && h != height))
			{
				std::cout << "Malformed texture found passed to material compilation." << std::endl;
				return false;
			}

			w = width;
			h = height;
		}
		if (!a_MaterialInfo.height.textureName.empty())
		{
			int width, height, channels;
			heightMap = stbi_load((a_MaterialInfo.path + a_MaterialInfo.height.textureName).c_str(), &width, &height, &channels, STBI_grey);

			//Width and height have to be the same for each one. 
			if (heightMap == nullptr || (w != 0 && w != width) || (h != 0 && h != height))
			{
				std::cout << "Malformed texture found passed to material compilation." << std::endl;
				return false;
			}

			w = width;
			h = height;
		}

		const long long size = w * h * 3;
		std::vector<unsigned char> ohData;
		ohData.resize(size);

		//Interleave with the third bit empty.
		for (unsigned i = 0; i < static_cast<unsigned>(w) * static_cast<unsigned>(h); ++i)
		{
			if (ao != nullptr)
			{
				ohData[i * 3 + 0] = ao[i];
			}

			if (heightMap != nullptr)
			{
				ohData[i * 3 + 1] = heightMap[i];
			}
		}

		header.aoHeight.width = w;
		header.aoHeight.height = h;
		header.aoHeight.size = size;
		header.aoHeight.start = data.size();

		data.insert(data.end(), ohData.begin(), ohData.end());

		if (ao)
		{
			stbi_image_free(ao);
		}
		if (heightMap)
		{
			stbi_image_free(heightMap);
		}
	}

	//Copy header into buffer (already has enough memory allocated for it at the start).
	*reinterpret_cast<MaterialHeader*>(&data[0]) = header;	

	/*
	 * Compression using LZ4.
	 */
	const int src_size = static_cast<int>(data.size());
	const int max_dst_size = LZ4_compressBound(src_size);
	char* compressed_data = static_cast<char*>(malloc(static_cast<size_t>(max_dst_size)));
	if (compressed_data == NULL)
	{
		throw std::exception("Could not allocate compression memory!");
	}
	const int compressed_data_size = LZ4_compress_HC(&data[0], compressed_data, src_size, max_dst_size, LZ4HC_CLEVEL_MAX);
	if (compressed_data_size <= 0)
	{
		throw std::exception("A 0 or negative result from LZ4_compress_default() indicates a failure trying to compress the data. ");
	}

	//Write to file.
	std::string finalName = a_FileName + MATERIAL_FILE_EXTENSION;
	std::ofstream file(finalName, std::ios::out | std::ios::binary);

	//Write some info needed for decompression.
	CompressionHeader compressionInfo;
	compressionInfo.compressedSize = compressed_data_size;
	compressionInfo.originalSize = src_size;


	//Write the compressed data.
	file.write(reinterpret_cast<char*>(&compressionInfo), sizeof(CompressionHeader));
	file.write(compressed_data, compressed_data_size);
	file.close();

	//Free memory
	free(compressed_data);

	return true;
}

std::shared_ptr<blurp::Material> blurp::LoadMaterial(blurp::RenderResourceManager& a_Manager, const std::string& a_FileName)
{
	std::ifstream file(a_FileName + MATERIAL_FILE_EXTENSION, std::ios::in | std::ios::binary);
	std::vector<char> data;

	CompressionHeader header;

	if (!file.eof() && !file.fail())
	{
		file.seekg(0, std::ios_base::end);
		auto fileSize = file.tellg();
		data.resize(fileSize);

		file.seekg(0, std::ios_base::beg);
		file.read(&data[0], fileSize);
	}
	else
	{
		throw std::exception("Could not load material file!");
	}

	header = *reinterpret_cast<CompressionHeader*>(&data[0]);

	char* const regen_buffer = static_cast<char*>(malloc(header.originalSize));
	if (regen_buffer == NULL)
	{
		throw std::exception("Failed to allocate memory for *regen_buffer.");
	}

	char* originStart = reinterpret_cast<char*>(&data[0]) + sizeof(CompressionHeader);

	const int decompressed_size = LZ4_decompress_safe(originStart, regen_buffer, static_cast<int>(header.compressedSize), static_cast<int>(header.originalSize));

	if (decompressed_size < 0)
	{
		throw std::exception("A negative result from LZ4_decompress_safe indicates a failure trying to decompress the data.  See exit code (echo $?) for value returned.");
	}

	if (decompressed_size != header.originalSize)
	{
		throw std::exception("Decompressed data is different from original!");
	}


	MaterialHeader* materialHeader = reinterpret_cast<MaterialHeader*>(regen_buffer);

	MaterialSettings matSettings;
	TextureSettings texSettings;
	texSettings.wrapMode = WrapMode::REPEAT;
	texSettings.minFilter = MinFilterType::MIPMAP_LINEAR;
	texSettings.magFilter = MagFilterType::LINEAR;
	texSettings.generateMipMaps = true;

	texSettings.textureType = TextureType::TEXTURE_2D;
	texSettings.memoryUsage = MemoryUsage::GPU;
	texSettings.memoryAccess = AccessMode::READ_ONLY;
	texSettings.pixelFormat = PixelFormat::RGB;
	texSettings.dataType = DataType::UBYTE;

	//Copy settings over
	matSettings.SetMask(materialHeader->mask);

	//Set constant data always.
	matSettings.SetDiffuseConstant(materialHeader->diffuse.constantData);
	matSettings.SetEmissiveConstant(materialHeader->emissive.constantData);
	matSettings.SetMetallicConstant(materialHeader->metalRoughnessAlpha.constantData.x);
	matSettings.SetRoughnessConstant(materialHeader->metalRoughnessAlpha.constantData.y);
	matSettings.SetAlphaConstant(materialHeader->metalRoughnessAlpha.constantData.z);

	/*
	 * Next up, look for textures and load if present (size > 0).
	 */

	//DIFFUSE
	if (materialHeader->diffuse.size > 0)
	{
		texSettings.texture2D.data = regen_buffer + materialHeader->diffuse.start;
		texSettings.dimensions = { materialHeader->diffuse.width, materialHeader->diffuse.height, 1 };
		std::shared_ptr<Texture> texture = a_Manager.CreateTexture(texSettings);
		matSettings.SetDiffuseTexture(texture);
	}

	//NORMAL
	if (materialHeader->normal.size > 0)
	{
		texSettings.texture2D.data = regen_buffer + materialHeader->normal.start;
		texSettings.dimensions = { materialHeader->normal.width, materialHeader->normal.height, 1 };
		std::shared_ptr<Texture> texture = a_Manager.CreateTexture(texSettings);
		matSettings.SetNormalTexture(texture);
	}

	//EMISSIVE
	if (materialHeader->emissive.size > 0)
	{
		texSettings.texture2D.data = regen_buffer + materialHeader->emissive.start;
		texSettings.dimensions = { materialHeader->emissive.width, materialHeader->emissive.height, 1 };
		std::shared_ptr<Texture> texture = a_Manager.CreateTexture(texSettings);
		matSettings.SetEmissiveTexture(texture);
	}

	//METAL/ROUGHNESS/ALPHA
	if (materialHeader->metalRoughnessAlpha.size > 0)
	{
		texSettings.texture2D.data = regen_buffer + materialHeader->metalRoughnessAlpha.start;
		texSettings.dimensions = { materialHeader->metalRoughnessAlpha.width, materialHeader->metalRoughnessAlpha.height, 1 };
		std::shared_ptr<Texture> texture = a_Manager.CreateTexture(texSettings);
		matSettings.SetMRATexture(texture);
	}

	//AMBIENT_OCCLUSION/HEIGHT
	if (materialHeader->aoHeight.size > 0)
	{
		texSettings.texture2D.data = regen_buffer + materialHeader->aoHeight.start;
		texSettings.dimensions = { materialHeader->aoHeight.width, materialHeader->aoHeight.height, 1 };
		std::shared_ptr<Texture> texture = a_Manager.CreateTexture(texSettings);
		matSettings.SetOHTexture(texture);
	}

	//Make the material before freeing the memory.
	auto mat = a_Manager.CreateMaterial(matSettings);

	//Free memory that is no longer used.
	free(regen_buffer);

	return mat;
}

bool blurp::CreateMaterialBatchFile(const MaterialBatchInfo& a_MaterialInfo, const std::string& a_FileName)
{
	//Ensure that positive dimensions are specified.
	if(a_MaterialInfo.dimensions.numMaterials <= 0 || a_MaterialInfo.dimensions.width <= 0 || a_MaterialInfo.dimensions.height <= 0)
	{
		std::cout << "Cannot create an empty material batch! Width, height and depth need to be atleast 1." << std::endl;
		return false;
	}

	//Header part of the file, containing most information.
	MaterialBatchHeader header;

	//Trailing data buffer.
	std::vector<char> data;

	//Resize to fit header.
	data.resize(sizeof(MaterialHeader));

	//Copy over the bitmask.
	header.batchData.mask = a_MaterialInfo.mask.GetMask();

	//Set dimensions
	header.batchData.materialCount = a_MaterialInfo.dimensions.numMaterials;
	header.batchData.width = a_MaterialInfo.dimensions.width;
	header.batchData.height = a_MaterialInfo.dimensions.height;

	//Add constant data to the buffer and store the offset.

	if(a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::DIFFUSE_CONSTANT_VALUE))
	{
	    if(a_MaterialInfo.diffuse.constantData.size() != a_MaterialInfo.dimensions.numMaterials)
	    {
			std::cout << "When constant values are enabled, there needs to be one provided for every material in the batch!" << std::endl;
			return false;
	    }
		//Set the headers contents.
		header.diffuseConstantData.start = data.size();

		//Interpret the array as char array, and then append to the dataset.
		const auto size = a_MaterialInfo.diffuse.constantData.size() * 3 * sizeof(float);
		char* start = (char*)(&a_MaterialInfo.diffuse.constantData[0]);
		data.insert(data.end(), start, start + size);
	}

	if (a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::EMISSIVE_CONSTANT_VALUE))
	{
		if (a_MaterialInfo.emissive.constantData.size() != a_MaterialInfo.dimensions.numMaterials)
		{
			std::cout << "When constant values are enabled, there needs to be one provided for every material in the batch!" << std::endl;
			return false;
		}
		//Set the headers contents.
		header.emissiveConstantData.start = data.size();

		//Interpret the array as char array, and then append to the dataset.
		const auto size = a_MaterialInfo.emissive.constantData.size() * 3 * sizeof(float);
		char* start = (char*)(&a_MaterialInfo.emissive.constantData[0]);
		data.insert(data.end(), start, start + size);
	}

	if (a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::METALLIC_CONSTANT_VALUE))
	{
		if (a_MaterialInfo.metallic.constantData.size() != a_MaterialInfo.dimensions.numMaterials)
		{
			std::cout << "When constant values are enabled, there needs to be one provided for every material in the batch!" << std::endl;
			return false;
		}
		//Set the headers contents.
		header.metallicConstantData.start = data.size();

		//Interpret the array as char array, and then append to the dataset.
		const auto size = a_MaterialInfo.metallic.constantData.size() * sizeof(float);
		char* start = (char*)(&a_MaterialInfo.metallic.constantData[0]);
		data.insert(data.end(), start, start + size);
	}

	if (a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::ROUGHNESS_CONSTANT_VALUE))
	{
		if (a_MaterialInfo.roughness.constantData.size() != a_MaterialInfo.dimensions.numMaterials)
		{
			std::cout << "When constant values are enabled, there needs to be one provided for every material in the batch!" << std::endl;
			return false;
		}
		//Set the headers contents.
		header.roughnessConstantData.start = data.size();

		//Interpret the array as char array, and then append to the dataset.
		const auto size = a_MaterialInfo.roughness.constantData.size() * sizeof(float);
		char* start = (char*)(&a_MaterialInfo.roughness.constantData[0]);
		data.insert(data.end(), start, start + size);
	}

	if (a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::ALPHA_CONSTANT_VALUE))
	{
		if (a_MaterialInfo.alpha.constantData.size() != a_MaterialInfo.dimensions.numMaterials)
		{
			std::cout << "When constant values are enabled, there needs to be one provided for every material in the batch!" << std::endl;
			return false;
		}
		//Set the headers contents.
		header.alphaConstantData.start = data.size();

		//Interpret the array as char array, and then append to the dataset.
		const auto size = a_MaterialInfo.alpha.constantData.size() * sizeof(float);
		char* start = (char*)(&a_MaterialInfo.alpha.constantData[0]);
		data.insert(data.end(), start, start + size);
	}


	/*
	 * Load textures and interleave their data.
	 */
	const size_t textureStart = data.size();
	header.textures.start = textureStart;

	const bool metalEnabled = a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::METALLIC_TEXTURE);
	const bool roughnessEnabled = a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::ROUGHNESS_TEXTURE);
	const bool alphaEnabled = a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::ALPHA_TEXTURE);

	const bool aoEnabled = a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::OCCLUSION_TEXTURE);
	const bool heightEnabled = a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::HEIGHT_TEXTURE);


	//Calculate how many textures are enabled per material. This is super ugly but it's offline I guess so yea.
	int numTextures = 0;
	if(metalEnabled || roughnessEnabled || alphaEnabled)
	{
		++numTextures;
	}
	if(aoEnabled || heightEnabled)
	{
		++numTextures;
	}
	if(a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::DIFFUSE_TEXTURE))
	{
		++numTextures;
	}
	if (a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::EMISSIVE_TEXTURE))
	{
		++numTextures;
	}
	if (a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::NORMAL_TEXTURE))
	{
		++numTextures;
	}

	//Store how many textures per material. This info is required for array texture creation.
	header.batchData.numTextures = numTextures;

	//Iterate over each depth and load each texture if enabled.
	for(int depth = 0; depth < a_MaterialInfo.dimensions.numMaterials; ++depth)
	{
		//RGB texture needs to be enabled even if just one is enabled.
		if (aoEnabled || heightEnabled)
		{
			unsigned char* ao = nullptr;
			unsigned char* heigtMap = nullptr;

			if(aoEnabled)
			{
				if (a_MaterialInfo.ao.textureNames.size() != a_MaterialInfo.dimensions.numMaterials)
				{
					std::cout << "Not enough textures specified! One needed for each depth of material batch." << std::endl;
					return false;
				}

				int width, height, channels;
				ao = stbi_load((a_MaterialInfo.path + a_MaterialInfo.ao.textureNames[depth]).c_str(), &width, &height, &channels, 3);

				if (width != a_MaterialInfo.dimensions.width || height != a_MaterialInfo.dimensions.height)
				{
					std::cout << "Dimension mismatch with texture in material batch!" << std::endl;
					return false;
				}

				if (ao == nullptr)
				{
					std::cout << "Could not find texture specified in material batch!" << std::endl;
					return false;
				}
			}

			if (heightEnabled)
			{
				if (a_MaterialInfo.height.textureNames.size() != a_MaterialInfo.dimensions.numMaterials)
				{
					std::cout << "Not enough textures specified! One needed for each depth of material batch." << std::endl;
					return false;
				}

				int width, height, channels;
				heigtMap = stbi_load((a_MaterialInfo.path + a_MaterialInfo.height.textureNames[depth]).c_str(), &width, &height, &channels, 3);

				if (width != a_MaterialInfo.dimensions.width || height != a_MaterialInfo.dimensions.height)
				{
					std::cout << "Dimension mismatch with texture in material batch!" << std::endl;
					return false;
				}

				if (heigtMap == nullptr)
				{
					std::cout << "Could not find texture specified in material batch!" << std::endl;
					return false;
				}
			}

			//Interleave the textures
			const size_t size = a_MaterialInfo.dimensions.width * a_MaterialInfo.dimensions.height;
			for(size_t i = 0; i < size; ++i)
			{
				data.push_back(ao == nullptr ? 0 : ao[3 * i + 0]);
				data.push_back(heigtMap == nullptr ? 0 : heigtMap[3 * i + 1]);
				data.push_back(0);
			}

			//Free stb memory.
			if(ao != nullptr) stbi_image_free(ao);
			if(heigtMap != nullptr) stbi_image_free(heigtMap);

		}

	    if(a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::DIFFUSE_TEXTURE))
	    {
	        if(a_MaterialInfo.diffuse.textureNames.size() != a_MaterialInfo.dimensions.numMaterials)
	        {
				std::cout << "Not enough textures specified! One needed for each depth of material batch." << std::endl;
				return false;
	        }

			int width, height, channels;
			const auto image = stbi_load((a_MaterialInfo.path + a_MaterialInfo.diffuse.textureNames[depth]).c_str(), &width, &height, &channels, 3);

			if(width != a_MaterialInfo.dimensions.width || height != a_MaterialInfo.dimensions.height)
			{
				std::cout << "Dimension mismatch with texture in material batch!" << std::endl;
				return false;
			}

			if(image == nullptr)
			{
				std::cout << "Could not find texture specified in material batch!" << std::endl;
				return false;
			}

			//Append to buffer.
			const size_t size = width * height * 3;
			data.insert(data.end(), image, image + size);
			stbi_image_free(image);
	    }

		if (a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::NORMAL_TEXTURE))
		{
			if (a_MaterialInfo.normal.textureNames.size() != a_MaterialInfo.dimensions.numMaterials)
			{
				std::cout << "Not enough textures specified! One needed for each depth of material batch." << std::endl;
				return false;
			}

			int width, height, channels;
			const auto image = stbi_load((a_MaterialInfo.path + a_MaterialInfo.normal.textureNames[depth]).c_str(), &width, &height, &channels, 3);

			if (width != a_MaterialInfo.dimensions.width || height != a_MaterialInfo.dimensions.height)
			{
				std::cout << "Dimension mismatch with texture in material batch!" << std::endl;
				return false;
			}

			if (image == nullptr)
			{
				std::cout << "Could not find texture specified in material batch!" << std::endl;
				return false;
			}

			//Append to buffer.
			const size_t size = width * height * 3;
			data.insert(data.end(), image, image + size);
			stbi_image_free(image);
		}

		if (a_MaterialInfo.mask.IsAttributeEnabled(MaterialAttribute::EMISSIVE_TEXTURE))
		{
			if (a_MaterialInfo.emissive.textureNames.size() != a_MaterialInfo.dimensions.numMaterials)
			{
				std::cout << "Not enough textures specified! One needed for each depth of material batch." << std::endl;
				return false;
			}

			int width, height, channels;
			const auto image = stbi_load((a_MaterialInfo.path + a_MaterialInfo.emissive.textureNames[depth]).c_str(), &width, &height, &channels, 3);

			if (width != a_MaterialInfo.dimensions.width || height != a_MaterialInfo.dimensions.height)
			{
				std::cout << "Dimension mismatch with texture in material batch!" << std::endl;
				return false;
			}

			if (image == nullptr)
			{
				std::cout << "Could not find texture specified in material batch!" << std::endl;
				return false;
			}

			//Append to buffer.
			const size_t size = width * height * 3;
			data.insert(data.end(), image, image + size);
			stbi_image_free(image);
		}

		//RGB needs to be added even if only one is enabled.
		if (metalEnabled || roughnessEnabled || alphaEnabled)
		{
			unsigned char* metal = nullptr;
			unsigned char* roughness = nullptr;
			unsigned char* alpha = nullptr;

			if (metalEnabled)
			{
				if (a_MaterialInfo.metallic.textureNames.size() != a_MaterialInfo.dimensions.numMaterials)
				{
					std::cout << "Not enough textures specified! One needed for each depth of material batch." << std::endl;
					return false;
				}

				int width, height, channels;
				metal = stbi_load((a_MaterialInfo.path + a_MaterialInfo.metallic.textureNames[depth]).c_str(), &width, &height, &channels, 3);

				if (width != a_MaterialInfo.dimensions.width || height != a_MaterialInfo.dimensions.height)
				{
					std::cout << "Dimension mismatch with texture in material batch!" << std::endl;
					return false;
				}

				if (metal == nullptr)
				{
					std::cout << "Could not find texture specified in material batch!" << std::endl;
					return false;
				}
			}

			if (roughnessEnabled)
			{
				if (a_MaterialInfo.roughness.textureNames.size() != a_MaterialInfo.dimensions.numMaterials)
				{
					std::cout << "Not enough textures specified! One needed for each depth of material batch." << std::endl;
					return false;
				}

				int width, height, channels;
				roughness = stbi_load((a_MaterialInfo.path + a_MaterialInfo.roughness.textureNames[depth]).c_str(), &width, &height, &channels, 3);

				if (width != a_MaterialInfo.dimensions.width || height != a_MaterialInfo.dimensions.height)
				{
					std::cout << "Dimension mismatch with texture in material batch!" << std::endl;
					return false;
				}

				if (roughness == nullptr)
				{
					std::cout << "Could not find texture specified in material batch!" << std::endl;
					return false;
				}
			}

			if (alphaEnabled)
			{
				if (a_MaterialInfo.alpha.textureNames.size() != a_MaterialInfo.dimensions.numMaterials)
				{
					std::cout << "Not enough textures specified! One needed for each depth of material batch." << std::endl;
					return false;
				}

				int width, height, channels;
				alpha = stbi_load((a_MaterialInfo.path + a_MaterialInfo.alpha.textureNames[depth]).c_str(), &width, &height, &channels, 3);

				if (width != a_MaterialInfo.dimensions.width || height != a_MaterialInfo.dimensions.height)
				{
					std::cout << "Dimension mismatch with texture in material batch!" << std::endl;
					return false;
				}

				if (alpha == nullptr)
				{
					std::cout << "Could not find texture specified in material batch!" << std::endl;
					return false;
				}
			}


			//Interleave the textures
			const size_t size = a_MaterialInfo.dimensions.width * a_MaterialInfo.dimensions.height;
			for (size_t i = 0; i < size; ++i)
			{
				data.push_back(metal == nullptr ? 0 : metal[3 * i + 0]);
				data.push_back(roughness == nullptr ? 0 : roughness[3 * i + 1]);
				data.push_back(alpha == nullptr ? 0 : alpha[3 * i + 1]);
			}

			//Free stb memory.
			if (metal != nullptr) stbi_image_free(metal);
			if (roughness != nullptr) stbi_image_free(roughness);
			if (alpha != nullptr) stbi_image_free(alpha);
		}


	}


	//Copy header into buffer (already has enough memory allocated for it at the start).
	*reinterpret_cast<MaterialBatchHeader*>(&data[0]) = header;

	/*
	 * Compression using LZ4.
	 */
	const int src_size = static_cast<int>(data.size());
	const int max_dst_size = LZ4_compressBound(src_size);
	char* compressed_data = static_cast<char*>(malloc(static_cast<size_t>(max_dst_size)));
	if (compressed_data == NULL)
	{
		throw std::exception("Could not allocate compression memory!");
	}
	const int compressed_data_size = LZ4_compress_HC(&data[0], compressed_data, src_size, max_dst_size, LZ4HC_CLEVEL_MAX);
	if (compressed_data_size <= 0)
	{
		throw std::exception("A 0 or negative result from LZ4_compress_default() indicates a failure trying to compress the data. ");
	}

	//Write to file.
	std::string finalName = a_FileName + MATERIAL_BATCH_FILE_EXTENSION;
	std::ofstream file(finalName, std::ios::out | std::ios::binary);

	//Write some info needed for decompression.
	CompressionHeader compressionInfo;
	compressionInfo.compressedSize = compressed_data_size;
	compressionInfo.originalSize = src_size;


	//Write the compressed data.
	file.write(reinterpret_cast<char*>(&compressionInfo), sizeof(CompressionHeader));
	file.write(compressed_data, compressed_data_size);
	file.close();

	//Free memory
	free(compressed_data);

	return true;
}

std::shared_ptr<blurp::MaterialBatch> blurp::LoadMaterialBatch(blurp::RenderResourceManager& a_Manager, const std::string& a_FileName)
{
	std::ifstream file(a_FileName + MATERIAL_BATCH_FILE_EXTENSION, std::ios::in | std::ios::binary);
	std::vector<char> data;

	CompressionHeader header;

	if (!file.eof() && !file.fail())
	{
		file.seekg(0, std::ios_base::end);
		auto fileSize = file.tellg();
		data.resize(fileSize);

		file.seekg(0, std::ios_base::beg);
		file.read(&data[0], fileSize);
	}
	else
	{
		throw std::exception("Could not load material file!");
	}

	header = *reinterpret_cast<CompressionHeader*>(&data[0]);

	char* const regen_buffer = static_cast<char*>(malloc(header.originalSize));
	if (regen_buffer == NULL)
	{
		throw std::exception("Failed to allocate memory for *regen_buffer.");
	}

	char* originStart = reinterpret_cast<char*>(&data[0]) + sizeof(CompressionHeader);

	const int decompressed_size = LZ4_decompress_safe(originStart, regen_buffer, static_cast<int>(header.compressedSize), static_cast<int>(header.originalSize));

	if (decompressed_size < 0)
	{
		throw std::exception("A negative result from LZ4_decompress_safe indicates a failure trying to decompress the data.  See exit code (echo $?) for value returned.");
	}

	if (decompressed_size != header.originalSize)
	{
		throw std::exception("Decompressed data is different from original!");
	}


	MaterialBatchHeader* materialHeader = reinterpret_cast<MaterialBatchHeader*>(regen_buffer);

	MaterialBatchSettings batchSettings;
	batchSettings.textureCount = materialHeader->batchData.numTextures;
	batchSettings.SetMask(materialHeader->batchData.mask);

	batchSettings.textureData = regen_buffer + materialHeader->textures.start;

	batchSettings.constantData.emissiveConstantData = reinterpret_cast<float*>(regen_buffer + materialHeader->emissiveConstantData.start);
	batchSettings.constantData.diffuseConstantData = reinterpret_cast<float*>(regen_buffer + materialHeader->diffuseConstantData.start);
	batchSettings.constantData.metallicConstantData = reinterpret_cast<float*>(regen_buffer + materialHeader->metallicConstantData.start);
	batchSettings.constantData.roughnessConstantData = reinterpret_cast<float*>(regen_buffer + materialHeader->roughnessConstantData.start);
	batchSettings.constantData.alphaConstantData = reinterpret_cast<float*>(regen_buffer + materialHeader->alphaConstantData.start);

	batchSettings.textureSettings.dataType = DataType::UBYTE;
	batchSettings.textureSettings.dimensions = {materialHeader->batchData.width , materialHeader->batchData.height };
	batchSettings.materialCount = materialHeader->batchData.materialCount;
	batchSettings.textureSettings.wrapMode = WrapMode::REPEAT;

	return a_Manager.CreateMaterialBatch(batchSettings);
}
