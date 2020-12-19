#include "MaterialLoader.h"
#include <iostream>
#include <stb_image.h>

std::shared_ptr<blurp::MaterialBatch> LoadMaterialBatch(blurp::RenderResourceManager& a_Manager)
{
	using namespace blurp;

	std::vector<unsigned char> textureData;
	MaterialBatchSettings batchSettings;

	MaterialData baseData;
	baseData.diffuseTextureName = "diffuse.jpg";
	baseData.aoTextureName = "ao.jpg";
	batchSettings.textureCount = 2;

	MaterialData d1 = baseData;
	d1.path = "materials/testmat1/";
	MaterialData d2 = baseData;
	d2.path = "materials/testmat2/";
	MaterialData d3 = baseData;
	d3.path = "materials/testmat3/";

	std::vector<glm::vec3> emissiveValues
	{
		{ 0.0f, 0.0f, 0.0f },
	    { 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f }
	};

	std::uint32_t w, h;

	auto d1Data = LoadTextureData(d1, w, h);
	std::cout << "Loaded texture" << d1.path << " with width: " << w << " and height: " << h << "." << std::endl;
	textureData.insert(textureData.end(), d1Data.begin(), d1Data.end());

	auto d2Data = LoadTextureData(d2, w, h);
	std::cout << "Loaded texture" << d2.path << " with width: " << w << " and height: " << h << "." << std::endl;
	textureData.insert(textureData.end(), d2Data.begin(), d2Data.end());

	auto d3Data = LoadTextureData(d3, w, h);
	std::cout << "Loaded texture" << d3.path << " with width: " << w << " and height: " << h << "." << std::endl;
	textureData.insert(textureData.end(), d3Data.begin(), d3Data.end());


	batchSettings.EnableAttribute(MaterialAttribute::DIFFUSE_TEXTURE);
	batchSettings.EnableAttribute(MaterialAttribute::OCCLUSION_TEXTURE);
	batchSettings.EnableAttribute(MaterialAttribute::EMISSIVE_CONSTANT_VALUE);

	batchSettings.textureData = &textureData[0];
	batchSettings.constantData.emissiveConstantData = reinterpret_cast<float*>(&emissiveValues[0]);
	batchSettings.textureSettings.dataType = DataType::UBYTE;
	batchSettings.textureSettings.dimensions = {w, h};
	batchSettings.materialCount = 3;
	batchSettings.textureSettings.wrapMode = WrapMode::REPEAT;

	return a_Manager.CreateMaterialBatch(batchSettings);
}

std::shared_ptr<blurp::Texture> LoadCubeMap(blurp::RenderResourceManager& a_Manager, const CubeMapSettings& a_Settings)
{
	using namespace blurp;

	assert(!a_Settings.front.empty());
	assert(!a_Settings.back.empty());
	assert(!a_Settings.left.empty());
	assert(!a_Settings.right.empty());
	assert(!a_Settings.up.empty());
	assert(!a_Settings.down.empty());

	int w = 0, h = 0;
	TextureSettings tS;

	std::vector<void*> ptrs;

	{
		int width, height, channels;
		//stbi_set_flip_vertically_on_load(true);
		unsigned char* image = stbi_load((a_Settings.path + a_Settings.right).c_str(), &width, &height, &channels, STBI_rgb);
		assert(w == 0 || w == width);
		assert(h == 0 || h == width);
		tS.textureCubeMap.data[0] = image;
		ptrs.push_back(image);
		w = width;
		h = height;
	}

	{
		int width, height, channels;
		//stbi_set_flip_vertically_on_load(true);
		unsigned char* image = stbi_load((a_Settings.path + a_Settings.left).c_str(), &width, &height, &channels, STBI_rgb);
		assert(w == 0 || w == width);
		assert(h == 0 || h == width);
		tS.textureCubeMap.data[1] = image;
		ptrs.push_back(image);
		w = width;
		h = height;
	}

	{
		int width, height, channels;
		//stbi_set_flip_vertically_on_load(true);
		unsigned char* image = stbi_load((a_Settings.path + a_Settings.up).c_str(), &width, &height, &channels, STBI_rgb);
		assert(w == 0 || w == width);
		assert(h == 0 || h == width);
		tS.textureCubeMap.data[2] = image;
		ptrs.push_back(image);
		w = width;
		h = height;
	}

	{
		int width, height, channels;
		//stbi_set_flip_vertically_on_load(true);
		unsigned char* image = stbi_load((a_Settings.path + a_Settings.down).c_str(), &width, &height, &channels, STBI_rgb);
		assert(w == 0 || w == width);
		assert(h == 0 || h == width);
		tS.textureCubeMap.data[3] = image;
		ptrs.push_back(image);
		w = width;
		h = height;
	}

	{
		int width, height, channels;
		//stbi_set_flip_vertically_on_load(true);
		unsigned char* image = stbi_load((a_Settings.path + a_Settings.front).c_str(), &width, &height, &channels, STBI_rgb);
		assert(w == 0 || w == width);
		assert(h == 0 || h == width);
		tS.textureCubeMap.data[4] = image;
		ptrs.push_back(image);
		w = width;
		h = height;
	}

	{
		int width, height, channels;
		//stbi_set_flip_vertically_on_load(true);
		unsigned char* image = stbi_load((a_Settings.path + a_Settings.back).c_str(), &width, &height, &channels, STBI_rgb);
		assert(w == 0 || w == width);
		assert(h == 0 || h == width);
		tS.textureCubeMap.data[5] = image;
		ptrs.push_back(image);
		w = width;
		h = height;
	}


	tS.dataType = DataType::UBYTE;
	tS.pixelFormat = PixelFormat::RGB;
	tS.wrapMode = WrapMode::CLAMP_TO_EDGE;
	tS.dimensions = glm::vec3(w, h, 1);
	tS.generateMipMaps = false;
	tS.textureType = TextureType::TEXTURE_CUBEMAP;

	auto tex = a_Manager.CreateTexture(tS);

	//Free STB reserved memory.
	for(auto& ptr : ptrs)
	{
		stbi_image_free(ptr);
	}

	return tex;
    
}

std::vector<unsigned char> LoadTextureData(const MaterialData& a_Data, std::uint32_t& a_Width, std::uint32_t& a_Height)
{
	using namespace blurp;

	std::vector<unsigned char> data;


	//AMBIENT_OCCLUSION/HEIGHT
	if (!a_Data.aoTextureName.empty() || !a_Data.heightTextureName.empty())
	{
		int w = 0, h = 0;
		stbi_set_flip_vertically_on_load(true);

		unsigned char* ao = nullptr;
		unsigned char* heightMap = nullptr;

		//NOTE all textures need to be the same dimensions.

		if (!a_Data.aoTextureName.empty())
		{
			int width, height, channels;
			ao = stbi_load((a_Data.path + a_Data.aoTextureName).c_str(), &width, &height, &channels, STBI_grey);
			assert(w == 0 || w == width);
			assert(h == 0 || h == width);
			w = width;
			h = height;
			a_Width = width;
			a_Height = height;
		}
		if (!a_Data.heightTextureName.empty())
		{
			int width, height, channels;
			heightMap = stbi_load((a_Data.path + a_Data.heightTextureName).c_str(), &width, &height, &channels, STBI_grey);
			assert(w == 0 || w == width);
			assert(h == 0 || h == width);
			w = width;
			h = height;
			a_Width = width;
			a_Height = height;
		}

		for (int i = 0; i < w * h; ++i)
		{
			if (ao != nullptr) data.push_back(ao[i]);
			else data.push_back(255);

			if (heightMap != nullptr) data.push_back(heightMap[i]);
			else data.push_back(255);

			data.push_back(255);
		}

		if (ao)
		{
			stbi_image_free(ao);
		}
		if (heightMap)
		{
			stbi_image_free(heightMap);
		}
	}

	//DIFFUSE
	if (!a_Data.diffuseTextureName.empty())
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* image = stbi_load((a_Data.path + a_Data.diffuseTextureName).c_str(),
			&width,
			&height,
			&channels,
			STBI_rgb);
		for(int i = 0; i < width * height * channels; ++i)
		{
			data.push_back(image[i]);
		}
		a_Width = width;
		a_Height = height;
		stbi_image_free(image);
	}

	//NORMAL
	if (!a_Data.normalTextureName.empty())
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* image = stbi_load((a_Data.path + a_Data.normalTextureName).c_str(),
			&width,
			&height,
			&channels,
			STBI_rgb);
		for (int i = 0; i < width * height * channels; ++i)
		{
			data.push_back(image[i]);
		}
		a_Width = width;
		a_Height = height;
		stbi_image_free(image);
	}

	//EMISSIVE
	if (!a_Data.emissiveTextureName.empty())
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* image = stbi_load((a_Data.path + a_Data.emissiveTextureName).c_str(),
			&width,
			&height,
			&channels,
			STBI_rgb);
		for (int i = 0; i < width * height * channels; ++i)
		{
			data.push_back(image[i]);
		}
		a_Width = width;
		a_Height = height;
		stbi_image_free(image);
	}

	//METAL/ROUGHNESS/ALPHA
	if (!a_Data.metallicTextureName.empty() || !a_Data.roughnessTextureName.empty() || !a_Data.alphaTextureName.empty())
	{
		int w = 0, h = 0;
		stbi_set_flip_vertically_on_load(true);

		unsigned char* metal = nullptr;
		unsigned char* rough = nullptr;
		unsigned char* alpha = nullptr;

		//NOTE all textures need to be the same dimensions.

		if (!a_Data.metallicTextureName.empty())
		{
			int width, height, channels;
			metal = stbi_load((a_Data.path + a_Data.metallicTextureName).c_str(), &width, &height, &channels, STBI_grey);
			assert(w == 0 || w == width);
			assert(h == 0 || h == width);
			w = width;
			h = height;
			a_Width = width;
			a_Height = height;
		}
		if (!a_Data.roughnessTextureName.empty())
		{
			int width, height, channels;
			rough = stbi_load((a_Data.path + a_Data.roughnessTextureName).c_str(), &width, &height, &channels, STBI_grey);
			assert(w == 0 || w == width);
			assert(h == 0 || h == width);
			w = width;
			h = height;
			a_Width = width;
			a_Height = height;
		}
		if (!a_Data.alphaTextureName.empty())
		{
			int width, height, channels;
			alpha = stbi_load((a_Data.path + a_Data.alphaTextureName).c_str(), &width, &height, &channels, STBI_grey);
			assert(w == 0 || w == width);
			assert(h == 0 || h == width);
			w = width;
			h = height;
			a_Width = width;
			a_Height = height;
		}

		for (int i = 0; i < w * h; ++i)
		{
			if (metal != nullptr) data.push_back(metal[i]);
			else data.push_back(0);
			if (rough != nullptr) data.push_back(rough[i]);
			else data.push_back(0);
			if (alpha != nullptr) data.push_back(alpha[i]);
			else data.push_back(0);
		}

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

	return data;
}
