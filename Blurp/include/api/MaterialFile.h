#pragma once
#include <memory>
#include <glm/vec3.hpp>
#include <string>
#include <cinttypes>
#include "Settings.h"


#define MATERIAL_FILE_EXTENSION ".blurpmat"
#define MATERIAL_BATCH_FILE_EXTENSION ".blurpmatx"

namespace blurp
{
	class Material;
	struct MaterialSettings;
	class RenderResourceManager;


	/*
	 * Structure with information about a material on disk.
	 * Contains paths to each specific texture and/or value.
	 *
	 * This data is used to construct a material file.
	 * Enable attributes in the mask and specify data for those attributes.
	 *
	 * Instead of file names, raw data can also be provided.
	 *
	 * A TextureSettings object is provided for each texture.
	 */
	struct MaterialInfo
	{
		MaterialInfo()
		{
			TextureSettings s;
			s.textureType = TextureType::TEXTURE_2D;
			s.pixelFormat = PixelFormat::RGB;
			s.dataType = DataType::UBYTE;

			settings = { s, s, s, s, s };
		}

		//Path where the files are found.
		std::string path;

		//The mask that determines the enabled material attributes.
		MaterialMask mask;

		//Diffuse texture or constant.
		struct
		{
			std::string textureName;
			glm::vec3 constant{0.f};
			void* data = nullptr;
		} diffuse;

		struct
		{
			std::string textureName;
			glm::vec3 constant{ 0.f };
			void* data = nullptr;
		} emissive;

		struct
		{
			std::string textureName;
			void* data = nullptr;
		} normal;

		struct
		{
			std::string textureName;
			void* data = nullptr;
		} ao;

		struct
		{
			std::string textureName;
			void* data = nullptr;
		} height;

		struct
		{
			std::string textureName;
			float constant = 0.f;
			void* data = nullptr;
		} metallic;
		
		struct
		{
			std::string textureName;
			float constant = 0.f;
			void* data = nullptr;
		} roughness;

		struct
		{
			std::string textureName;
			float constant = 0.f;
			void* data = nullptr;

		} alpha;

		/*
		 * Texture settings for each texture type.
		 */
		struct
		{
			TextureSettings metalRoughAlpha;
			TextureSettings ambientOcclusionHeight;
			TextureSettings diffuse;
			TextureSettings normal;
			TextureSettings emissive;
		} settings;
	};

    /*
	 * Information about start and size of data inside a material file.
	 */
	struct MaterialFileAttribute
	{
		MaterialFileAttribute() : start(0), size(0), constantData(0.f)
		{
		    
		}

		long long start;
		long long size;
		glm::vec3 constantData;
		TextureSettings settings;
	};

	/*
	 * Material files raw data start with this struct.
	 */
	struct MaterialHeader
	{
		MaterialHeader() : version(1), mask(0) {}

		std::uint16_t version;
		std::uint16_t mask;

		MaterialFileAttribute diffuse;
		MaterialFileAttribute emissive;
		MaterialFileAttribute normal;
		MaterialFileAttribute aoHeight;
		MaterialFileAttribute metalRoughnessAlpha;
	};

	/*
	 * Header containing information for decompression.
	 */
	struct CompressionHeader
	{
		CompressionHeader() : originalSize(0), compressedSize(0)
		{
		    
		}

		size_t originalSize;
		size_t compressedSize;
	};

	struct MaterialBatchHeader
	{
		MaterialBatchHeader() : version(1), batchData({0, 0, 0})
		{
		    
		}

		std::uint16_t version;

		//General info that affects the entire batch.
		struct
		{
			std::int16_t materialCount;
			std::int16_t numTextures;

			std::uint16_t mask;
			MaterialBatchTextureSettings settings;
		} batchData;

		//Raw texture data.
		struct
		{
			long long start;
			long long size;
		} textures;

		//Constant data offset.

		struct
		{
			long long start;
		} diffuseConstantData;

		struct
		{
			long long start;
		} emissiveConstantData;

		struct
		{
			long long start;
		} alphaConstantData;

		struct
		{
			long long start;
		} metallicConstantData;

		struct
		{
			long long start;
		} roughnessConstantData;
	};


	/*
	 * Enable attributes and specify data for multiple attributes.
	 * The amount of elements in the specified data has to be the same as the number of elements specified!
	 * All textures have to be the same dimensions.
	 */
	struct MaterialBatchInfo
	{
		//Path where the files are found.
		std::string path;

		//The mask that determines the enabled material attributes.
		MaterialMask mask;

		//The amount of materials in this batch.
		std::int32_t materialCount;

		//The settings that will apply to this material batch. The settings apply to every texture in the batch.
		MaterialBatchTextureSettings textureSettings;

		struct
		{
			std::vector<std::string> textureNames;
			std::vector<glm::vec3> constantData;
		} diffuse;

		struct
		{
			std::vector<std::string> textureNames;
			std::vector<glm::vec3> constantData;
		} emissive;

		struct
		{
			std::vector<std::string> textureNames;
		} normal;

		struct
		{
			std::vector<std::string> textureNames;
		} ao;

		struct
		{
			std::vector<std::string> textureNames;
		} height;

		struct
		{
			std::vector<std::string> textureNames;
			std::vector<float> constantData;
		} metallic;

		struct
		{
			std::vector<std::string> textureNames;
			std::vector<float> constantData;
		} roughness;

		struct
		{
			std::vector<std::string> textureNames;
			std::vector<float> constantData;
		} alpha;
	};


	/*
	 * Create a material file from the given material settings.
	 * This will save the material file with the given file name and path.
	 */
	bool CreateMaterialFile(const MaterialInfo& a_MaterialInfo, const std::string& a_Path, const std::string& a_FileName);

	/*
	 *
	 */
	bool CreateMaterialFile(const MaterialInfo& a_MaterialInfo, const std::string& a_Path, const std::string& a_FileName);

	/*
	 * Load a material from the given file name.
	 */
	std::shared_ptr<Material> LoadMaterial(blurp::RenderResourceManager& a_Manager, const std::string& a_FileName);

	/*
	 * Create a material batch file from the given material settings.
	 * This will save the material file with the given file name and path.
	 */
	bool CreateMaterialBatchFile(const MaterialBatchInfo& a_MaterialInfo, const std::string& a_Path, const std::string& a_FileName);

	/*
	 * Load a material batch from the given file.
	 */
	std::shared_ptr<MaterialBatch> LoadMaterialBatch(blurp::RenderResourceManager& a_Manager, const std::string& a_FileName);


}
