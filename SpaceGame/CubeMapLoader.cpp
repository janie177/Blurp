#include "CubeMapLoader.h"
#include <iostream>
#include <stb_image.h>


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
	for (auto& ptr : ptrs)
	{
		stbi_image_free(ptr);
	}

	return tex;

}