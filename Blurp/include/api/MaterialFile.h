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
	 */
	struct MaterialInfo
	{
		//Path where the files are found.
		std::string path;

		//The mask that determines the enabled material attributes.
		MaterialMask mask;

		//Diffuse texture or constant.
		struct
		{
			std::string textureName;
			glm::vec3 constant{0.f};
		} diffuse;

		struct
		{
			std::string textureName;
			glm::vec3 constant{ 0.f };
		} emissive;

		struct
		{
			std::string textureName;
		} normal;

		struct
		{
			std::string textureName;
		} ao;

		struct
		{
			std::string textureName;
		} height;

		struct
		{
			std::string textureName;
			float constant = 0.f;
		} metallic;
		
		struct
		{
			std::string textureName;
			float constant = 0.f;
		} roughness;

		struct
		{
			std::string textureName;
			float constant = 0.f;
		} alpha;
	};

    /*
	 * Information about start and size of data inside a material file.
	 */
	struct MaterialFileAttribute
	{
		MaterialFileAttribute() : start(0), size(0), width(0), height(0), constantData(0.f)
		{
		    
		}

		long long start;
		long long size;
		int width;
		int height;
		glm::vec3 constantData;
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

	///*
	// * Material batch files start with this header.
	// */
	//struct MaterialBatchHeader
	//{
	//	std::uint16_t version;
	//	std::uint16_t mask;

	//	MaterialFileAttribute textures;
	//	MaterialFileAttribute diffuse;
	//	MaterialFileAttribute emissive;
	//	MaterialFileAttribute metal;
	//	MaterialFileAttribute rough;
	//	MaterialFileAttribute alpha;
	//};


	/*
	 * Create a material file from the given material settings.
	 * This will save the material file with the given file name and path.
	 */
	bool CreateMaterialFile(const MaterialInfo& a_MaterialInfo, const std::string& a_Output);

	/*
	 * Load a material from the given file name.
	 */
	std::shared_ptr<Material> LoadMaterial(blurp::RenderResourceManager& a_Manager, const std::string& a_FileName);


}
