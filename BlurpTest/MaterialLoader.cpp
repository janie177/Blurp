#include "MaterialLoader.h"
#include "stb_image.h"

std::shared_ptr<blurp::Material> LoadMaterial(blurp::RenderResourceManager& a_Manager, const MaterialData& a_Data)
{
	using namespace blurp;

	MaterialSettings matSettings;

	//Diffuse is present, load.
	if(!a_Data.diffuseTextureName.empty())
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* image = stbi_load((a_Data.path + a_Data.diffuseTextureName).c_str(),
			&width,
			&height,
			&channels,
			STBI_rgb);

		//Convert uchar to char.
		const auto size = width * height * channels;
		std::vector<char> signedImage;
		signedImage.resize(size);

		TextureSettings texSettings;
		texSettings.dimensions = {width, height, 1};
		texSettings.wrapMode = WrapMode::REPEAT;
		texSettings.minFilter = MinFilterType::MIPMAP_LINEAR;
		texSettings.magFilter = MagFilterType::LINEAR;
		texSettings.generateMipMaps = true;

		texSettings.textureType = TextureType::TEXTURE_2D;
		texSettings.memoryUsage = MemoryUsage::GPU;
		texSettings.memoryAccess = AccessMode::READ_ONLY;
		texSettings.pixelFormat = PixelFormat::RGB;
		texSettings.dataType = DataType::UBYTE;

		texSettings.texture2D.data = image;

		std::shared_ptr<Texture> texture = a_Manager.CreateTexture(texSettings);

		matSettings.EnableAttribute(MaterialAttribute::DIFFUSE_TEXTURE);
		matSettings.SetDiffuseTexture(texture);


		stbi_image_free(image);

	}

	return a_Manager.CreateMaterial(matSettings);
}
