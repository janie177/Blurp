#include "MaterialLoader.h"
#include "stb_image.h"

std::shared_ptr<blurp::Material> LoadMaterial(blurp::RenderResourceManager& a_Manager, const MaterialData& a_Data)
{
	using namespace blurp;

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

	//DIFFUSE
	if(!a_Data.diffuseTextureName.empty())
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* image = stbi_load((a_Data.path + a_Data.diffuseTextureName).c_str(),
			&width,
			&height,
			&channels,
			STBI_rgb);
		texSettings.texture2D.data = image;
		texSettings.dimensions = { width, height, 1 };
		std::shared_ptr<Texture> texture = a_Manager.CreateTexture(texSettings);
		matSettings.EnableAttribute(MaterialAttribute::DIFFUSE_TEXTURE);
		matSettings.SetDiffuseTexture(texture);
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
		texSettings.texture2D.data = image;
		texSettings.dimensions = { width, height, 1 };
		std::shared_ptr<Texture> texture = a_Manager.CreateTexture(texSettings);
		matSettings.EnableAttribute(MaterialAttribute::NORMAL_TEXTURE);
		matSettings.SetNormalTexture(texture);
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
		texSettings.texture2D.data = image;
		texSettings.dimensions = { width, height, 1 };
		std::shared_ptr<Texture> texture = a_Manager.CreateTexture(texSettings);
		matSettings.EnableAttribute(MaterialAttribute::EMISSIVE_TEXTURE);
		matSettings.SetEmissiveTexture(texture);
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
			matSettings.EnableAttribute(MaterialAttribute::METALLIC_TEXTURE);
			assert(w == 0 || w == width);
			assert(h == 0 || h == width);
			w = width;
			h = height;
		}
		if (!a_Data.roughnessTextureName.empty())
		{
			int width, height, channels;
		    rough = stbi_load((a_Data.path + a_Data.roughnessTextureName).c_str(), &width, &height, &channels, STBI_grey);
			matSettings.EnableAttribute(MaterialAttribute::ROUGHNESS_TEXTURE);
			assert(w == 0 || w == width);
			assert(h == 0 || h == width);
			w = width;
			h = height;
		}
		if (!a_Data.alphaTextureName.empty())
		{
			int width, height, channels;
		    alpha = stbi_load((a_Data.path + a_Data.alphaTextureName).c_str(), &width, &height, &channels, STBI_grey);
			matSettings.EnableAttribute(MaterialAttribute::ALPHA_TEXTURE);
			assert(w == 0 || w == width);
			assert(h == 0 || h == width);
			w = width;
			h = height;
		}

		const auto size = w * h * 3;		std::vector<unsigned char> mraData;
		mraData.resize(size);

		for (unsigned i = 0; i < w * h; ++i)
		{
			if (metal != nullptr) mraData[3 * i + 0] = metal[i];
			if (rough != nullptr) mraData[3 * i + 1] = rough[i];
			if (alpha != nullptr) mraData[3 * i + 2] = alpha[i];
		}


	    texSettings.texture2D.data = &mraData[0];
		texSettings.dimensions = { w, h, 1 };
		std::shared_ptr<Texture> texture = a_Manager.CreateTexture(texSettings);
		matSettings.SetMRATexture(texture);

		if(metal)
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
			matSettings.EnableAttribute(MaterialAttribute::OCCLUSION_TEXTURE);
			assert(w == 0 || w == width);
			assert(h == 0 || h == width);
			w = width;
			h = height;
		}
		if (!a_Data.heightTextureName.empty())
		{
			int width, height, channels;
			heightMap = stbi_load((a_Data.path + a_Data.heightTextureName).c_str(), &width, &height, &channels, STBI_grey);
			matSettings.EnableAttribute(MaterialAttribute::HEIGHT_TEXTURE);
			assert(w == 0 || w == width);
			assert(h == 0 || h == width);
			w = width;
			h = height;
		}

		const auto size = w * h * 3;
		std::vector<unsigned char> ohData;
		ohData.resize(size);

		for (unsigned i = 0; i < w * h; ++i)
		{
			if (ao != nullptr) ohData[3 * i + 0] = ao[i];

			if (heightMap != nullptr)
			{
				short raw = heightMap[i];
				unsigned char half1 = raw & 0xff;
				unsigned char half2 = raw >> 8;
				ohData[3 * i + 1] = half1;
				ohData[3 * i + 2] = half2;
			}
		}


		texSettings.texture2D.data = &ohData[0];
		texSettings.dimensions = { w, h, 1 };
		std::shared_ptr<Texture> texture = a_Manager.CreateTexture(texSettings);
		matSettings.SetOHTexture(texture);

		if (ao)
		{
			stbi_image_free(ao);
		}
		if (heightMap)
		{
			stbi_image_free(heightMap);
		}
	}

	return a_Manager.CreateMaterial(matSettings);
}
