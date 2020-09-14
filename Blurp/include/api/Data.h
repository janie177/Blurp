#pragma once
#include <cinttypes>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <memory>

#include "GpuBufferView.h"

/*
 * Data structures in this header are used to keep track of data on the CPU and GPU.
 */

#define NUM_VERTEX_ATRRIBS 12
#define NUM_MATERIAL_ATRRIBS 13
#define NUM_DRAW_ATTRIBS 5

namespace blurp
{
    //Forward declarations.
    class Texture;
    class Mesh;
    class Material;
    class MaterialBatch;
    class GpuBuffer;
    class Light;
    class SpotLight;
    class DirectionalLight;
    class PointLight;

    enum class ProjectionMode
    {
        ORTHOGRAPHIC,
        PERSPECTIVE
    };

    enum class Direction : std::uint8_t
    {
        RIGHT,
        LEFT,
        UP,
        DOWN,
        FORWARD,
        BACKWARD
    };

    enum class RenderPassType
    {
        RP_HELLOTRIANGLE,
        RP_CLEAR,
        RP_FORWARD,
        RP_DEFERRED,
        RP_SHADOWMAP,
        RP_CUBEMAP,
        RP_SKYBOX,
        RP_DOF,
        RP_2D,
        RP_BLOOM,
        RP_BLUR,
        RP_ANIMATION
    };

    enum class WindowType
    {
        WINDOW_WIN32,
        NONE
    };

    enum class LightType
    {
        LIGHT_AMBIENT,
        LIGHT_SPOT,
        LIGHT_DIRECTIONAL,
        LIGHT_POINT
    };

    enum class GraphicsAPI
    {
        OPENGL,
        OPENGLES,
        VULKAN,
        DIRECTX12
    };

    enum class TextureType
    {
        TEXTURE_1D,
        TEXTURE_2D,
        TEXTURE_3D,
        TEXTURE_CUBEMAP,
        TEXTURE_2D_ARRAY,
        TEXTURE_CUBEMAP_ARRAY
    };

    enum class WrapMode
    {
        CLAMP_TO_EDGE,
        MIRRORED_REPEAT,
        REPEAT
    };


    /*
     * Enumeration that can be used to specify how memory will be accessed.
     * CPU_R means that the memory should be optimized for CPU reading. GPU access is not optimal. Used for reading back compute output.
     * CPU_W means that the memory should be optimized for CPU writing. GPU access is not optimal. Used for frequently changing data like transforms.
     * GPU means that the memory should be optimized for GPU access. CPU access will be slow. Used for data that does not change often.
     *
     * Note: This does not prevent the CPU or GPU from accessing the memory. It just allows for the memory to be faster when used in this scenario.
     */
    enum class MemoryUsage
    {
        CPU_R,
        CPU_W,
        GPU
    };

    /*
     * Enumeration that can be used to specify whether a resource is mutable or not.
     * READ_ONLY means a resource will never be written to once it is uploaded.
     * READ_WRITE means that a resource may change, and requires locking and state tracking.
     */
    enum class AccessMode
    {
        READ_ONLY,
        READ_WRITE
    };

    enum class MagFilterType
    {
        NEAREST,
        LINEAR
    };

    enum class MinFilterType
    {
        NEAREST,
        LINEAR,
        MIPMAP_NEAREST,
        MIPMAP_LINEAR
    };

    enum class ShaderType
    {
        COMPUTE,
        GRAPHICS
    };

    enum class PixelFormat
    {
        R,
        RG,
        RGB,
        RGBA,
        DEPTH,
        DEPTH_STENCIL
    };

    enum class DataType
    {
        FLOAT,
        INT,
        BYTE,
        SHORT,
        UINT,
        UBYTE,
        USHORT
    };

    /*
     * Helper function to see if a DataType is signed or unsigned.
     */
    bool IsSigned(DataType a_Type);

    enum class WindowFlags : std::uint16_t
    {
        OPEN_FULLSCREEN = 1 << 0,
        HIDE_CURSOR = 1 << 1,
        CAPTURE_CURSOR = 1 << 2,
        NONE = 1 << 3
    };

    /*
     * Bitwise OR operator.
     * Combine WindowFlags into a single new value.
     */
    inline WindowFlags operator|(WindowFlags a_Lhs, WindowFlags a_Rhs)
    {
        return static_cast<WindowFlags>(static_cast<int>(a_Lhs) | static_cast<int>(a_Rhs));
    }

    /*
     * Bitwise AND operator.
     * Compare if the flags match.
     */
    inline bool operator&(WindowFlags a_Lhs, WindowFlags a_Rhs)
    {
        return (static_cast<int>(a_Lhs) & static_cast<int>(a_Rhs)) != 0;
    }

    enum class VertexAttribute : std::uint16_t
    {
        //X, Y and Z position.
        POSITION_3D = 1 << 0,

        //UV coordinates for a texture.
        //Multiple UV coords can be enabled for different material
        UV_COORDS = 1 << 1,

        //X, Y and Z direction of the surface.
        NORMAL = 1 << 2,

        //R, G and B color channels.
        COLOR = 1 << 3,

        //X, Y and Z indicating a curved direction.
        TANGENT = 1 << 4,

        //X, Y and Z indicating a curved surface. Used in combination with Tangent.
        BI_TANGENT = 1 << 5,

        //Vec3 of the bone indices affecting vertex.
        BONE_INDEX = 1 << 6,

        //Vec3 of the weights of each bone (to be used with BONE_INDEX).
        BONE_WEIGHT = 1 << 7,

        //The Integer material ID to use. Used in combination with Material Batches.
        MATERIAL_ID = 1 << 8,

        //Float representing the index of the UV modifier in the UVModifierBuffer.
        UV_MODIFIER_ID = 1 << 9,

        //Matrices for instancing.
        MATRIX = 1 << 10,

        //Transpose of the inverse instance matrix. Used with MATRIX to allow for uneven scaling with correct lighting normals.
        ITMATRIX = 1 << 11,
    };

    //All vertex attributes in an iterable format.
    const static VertexAttribute VERTEX_ATTRIBUTES[NUM_VERTEX_ATRRIBS]{
        VertexAttribute::POSITION_3D,
        VertexAttribute::UV_COORDS,
        VertexAttribute::NORMAL,
        VertexAttribute::COLOR,
        VertexAttribute::TANGENT,
        VertexAttribute::BI_TANGENT,
        VertexAttribute::BONE_INDEX,
        VertexAttribute::BONE_WEIGHT,
        VertexAttribute::MATERIAL_ID,
        VertexAttribute::UV_MODIFIER_ID,
        VertexAttribute::MATRIX,
        VertexAttribute::ITMATRIX,
    };

    inline VertexAttribute operator|(VertexAttribute a_Lhs, VertexAttribute a_Rhs)
    {
        return static_cast<VertexAttribute>(static_cast<std::uint16_t>(a_Lhs) | static_cast<std::uint16_t>(a_Rhs));
    }

    inline VertexAttribute operator^(VertexAttribute a_Lhs, VertexAttribute a_Rhs)
    {
        return static_cast<VertexAttribute>(static_cast<std::uint16_t>(a_Lhs) ^ static_cast<std::uint16_t>(a_Rhs));
    }

    inline VertexAttribute operator~(VertexAttribute a_Lhs)
    {
        return static_cast<VertexAttribute>(~static_cast<std::uint16_t>(a_Lhs));
    }

    inline VertexAttribute operator&(VertexAttribute a_Lhs, VertexAttribute a_Rhs)
    {
        return static_cast<VertexAttribute>(static_cast<std::uint16_t>(a_Lhs) & static_cast<std::uint16_t>(a_Rhs));
    }

    inline bool operator==(VertexAttribute a_Lhs, VertexAttribute a_Rhs)
    {
        return static_cast<std::uint16_t>(a_Lhs) == static_cast<std::uint16_t>(a_Rhs);
    }

    /*
     * Enumeration containing the bitmasks for various material attributes.
     */
    enum class MaterialAttribute : std::uint16_t
    {
        //Occlusion texture containing a single byte value per pixel indicating how much it should be darkened.
        //Stored in the R channel of an Occlusion/Height texture.
        OCCLUSION_TEXTURE = 1 << 0,

        //Height offset from the surface. This moves vertices along their normal direction.
        //Stored as the G channel of the Occlusion/Height texture.
        HEIGHT_TEXTURE = 1 << 1,

        //Diffuse texture used for coloring this material.
        //Enabling this disables DIFFUSE_CONSTANT_VALUE
        //RGB format.
        DIFFUSE_TEXTURE = 1 << 2,

        //Normal texture containing the direction of each pixel in XYZ format. Can only be used with meshes that contain normal information.
        //RGB format.
        NORMAL_TEXTURE = 1 << 3,

        //Emissive texture indicating the brightness in R, G and B channels. Added on top of calculated light value to appear glowing.
        //Enabling this disables EMISSIVE_CONSTANT_VALUE.
        //RGB format.
        EMISSIVE_TEXTURE = 1 << 4,

        //Metallicness of the surface stored as a single byte value.
        //Enabling this disables the METALLIC_CONSTANT_VALUE setting.
        //Stored in the R channel of an Metallic/Roughness/Alpha texture.
        METALLIC_TEXTURE = 1 << 5,

        //Roughness of the surface.
        //Enabling this disables this ROUGHNESS_CONSTANT_VALUE setting.
        //Stored in the G channel of an Metallic/Roughness/Alpha texture.
        ROUGHNESS_TEXTURE = 1 << 6,

        //Alpha of the surface. Stored as an RG texture.
        //Enabling this disables the ALPHA_CONSTANT_VALUE setting.
        //Stored in the B channel of an Metallic/Roughness/Alpha texture.
        ALPHA_TEXTURE = 1 << 7,

        //Allows using a single color for the diffuse texture of this material.
        //Enabling this disables DIFFUSE_TEXTURE texture.
        //Colors scaled between 0 and 1.
        DIFFUSE_CONSTANT_VALUE = 1 << 8,

        //Allows the setting of a single color for the emissive texture of this material.
        //Enabling this disables EMISSIVE_TEXTURE texture.
        //Emissive color specified as RGB normalized between 0 and 1.
        EMISSIVE_CONSTANT_VALUE = 1 << 9,

        //Allows the setting of the metallicness of the entire surface.
        //Enabling this disables METALLIC_TEXTURE texture.
        //Float normalized between 0 and 1.
        METALLIC_CONSTANT_VALUE = 1 << 10,

        //Allows the setting of a single float value to measure the roughness of the entire material.
        //Enabling this disables the ROUGHNESS_TEXTURE texture.
        //Normalized between 0 and 1.
        ROUGHNESS_CONSTANT_VALUE = 1 << 11,

        //Allows the setting of a single float value representing the opacity of the entire material.
        //This disables the ALPHA_TEXTURE texture.
        //Normalized between 0 (translucent) and 1 (opaque).
        ALPHA_CONSTANT_VALUE = 1 << 12,
    };

    /*
     * All material attributes sorted in order of the bitmask.
     */
    const static MaterialAttribute MATERIAL_ATTRIBUTES[NUM_MATERIAL_ATRRIBS]{
        MaterialAttribute::OCCLUSION_TEXTURE,
        MaterialAttribute::HEIGHT_TEXTURE,
        MaterialAttribute::DIFFUSE_TEXTURE,
        MaterialAttribute::NORMAL_TEXTURE,
        MaterialAttribute::EMISSIVE_TEXTURE,
        MaterialAttribute::METALLIC_TEXTURE,
        MaterialAttribute::ROUGHNESS_TEXTURE,
        MaterialAttribute::ALPHA_TEXTURE,

        MaterialAttribute::DIFFUSE_CONSTANT_VALUE,
        MaterialAttribute::EMISSIVE_CONSTANT_VALUE,
        MaterialAttribute::METALLIC_CONSTANT_VALUE,
        MaterialAttribute::ROUGHNESS_CONSTANT_VALUE,
        MaterialAttribute::ALPHA_CONSTANT_VALUE,
    };


    /*
     * Array of all texture material attributes.
     */
    const static MaterialAttribute TEXTURE_MATERIAL_ATTRIBUTES[]
    {
        MaterialAttribute::OCCLUSION_TEXTURE,
        MaterialAttribute::HEIGHT_TEXTURE,
        MaterialAttribute::DIFFUSE_TEXTURE,
        MaterialAttribute::NORMAL_TEXTURE,
        MaterialAttribute::EMISSIVE_TEXTURE,
        MaterialAttribute::METALLIC_TEXTURE,
        MaterialAttribute::ROUGHNESS_TEXTURE,
        MaterialAttribute::ALPHA_TEXTURE,
    };

    /*
     * Array of all constant value material attributes.
     */
    const static MaterialAttribute CONSTANT_MATERIAL_ATTRIBUTES[]
    {
        MaterialAttribute::DIFFUSE_CONSTANT_VALUE,
        MaterialAttribute::EMISSIVE_CONSTANT_VALUE,
        MaterialAttribute::METALLIC_CONSTANT_VALUE,
        MaterialAttribute::ROUGHNESS_CONSTANT_VALUE,
        MaterialAttribute::ALPHA_CONSTANT_VALUE,
    };

    struct MaterialAttributeInfo
    {
        //Preprocessor definition name of this attribute.
        std::string defineName;
        std::uint8_t numElements;
    };

    /*
     * Unordered map containing information about each material attribute.
     */
    static const std::unordered_map<MaterialAttribute, MaterialAttributeInfo> MATERIAL_ATTRIBUTE_INFO =
    {
        {MaterialAttribute::OCCLUSION_TEXTURE, {"MAT_OCCLUSION_TEXTURE_DEFINE", 1}},
        {MaterialAttribute::HEIGHT_TEXTURE, {"MAT_HEIGHT_TEXTURE_DEFINE", 1}},
        {MaterialAttribute::DIFFUSE_TEXTURE, {"MAT_DIFFUSE_TEXTURE_DEFINE", 3}},
        {MaterialAttribute::NORMAL_TEXTURE, {"MAT_NORMAL_TEXTURE_DEFINE", 3}},
        {MaterialAttribute::EMISSIVE_TEXTURE, {"MAT_EMISSIVE_TEXTURE_DEFINE", 3}},
        {MaterialAttribute::METALLIC_TEXTURE, {"MAT_METALLIC_TEXTURE_DEFINE", 1}},
        {MaterialAttribute::ROUGHNESS_TEXTURE, {"MAT_ROUGHNESS_TEXTURE_DEFINE", 1}},
        {MaterialAttribute::ALPHA_TEXTURE, {"MAT_ALPHA_TEXTURE_DEFINE", 1}},

        {MaterialAttribute::DIFFUSE_CONSTANT_VALUE, {"MAT_DIFFUSE_CONSTANT_DEFINE", 3}},
        {MaterialAttribute::EMISSIVE_CONSTANT_VALUE, {"MAT_EMISSIVE_CONSTANT_DEFINE", 3}},
        {MaterialAttribute::METALLIC_CONSTANT_VALUE, {"MAT_METALLIC_CONSTANT_DEFINE", 1}},
        {MaterialAttribute::ROUGHNESS_CONSTANT_VALUE, {"MAT_ROUGHNESS_CONSTANT_DEFINE", 1}},
        {MaterialAttribute::ALPHA_CONSTANT_VALUE, {"MAT_ALPHA_CONSTANT_DEFINE", 1}},
    };


    enum class DrawAttribute
    {
        /*
         * Transformation matrix to move the mesh to world space.
         */
        TRANSFORMATION_MATRIX = 1 << 0,

        /*
        * Inverse transpose of the above transformation matrix to transform the mesh normals to world space when uneven scaling is used.
        */
        NORMAL_MATRIX = 1 << 1,

        /*
        * One or more vec4's representing UV multiplier and offset.
        */
        UV_MODIFIER = 1 << 2,

        /*
        * A single material is used.
        */
        MATERIAL_SINGLE = 1 << 3,

        /*
        * A material batch is used.
        */
        MATERIAL_BATCH = 1 << 4,
    };

    const static DrawAttribute DRAW_ATTRIBUTES[NUM_DRAW_ATTRIBS]
    {
        DrawAttribute::TRANSFORMATION_MATRIX,
        DrawAttribute::NORMAL_MATRIX,
        DrawAttribute::UV_MODIFIER,
        DrawAttribute::MATERIAL_SINGLE,
        DrawAttribute::MATERIAL_BATCH
    };

    /*
     * How will the DrawAttribute be used while rendering?
     */
    enum class DrawAttributeUsageType
    {
        PER_INSTANCE,       //DrawAtrribute is uploaded per instance.
        GLOBAL_SINGLE,      //DrawAttribute is uploaded once globally for all instances to use. Only one of the DrawAttributes is required per draw call.
        GLOBAL_ARRAY        //DrawAttribute is uploaded once globally for all instances to use. The total data size is a multiple of the size per element, as it has an unbound size.
    };

    struct DrawAttributeInfo
    {
        //Preprocessor definition name of this attribute.
        std::string defineName;

        //How is the data of the draw attribute used?
        DrawAttributeUsageType usage;

        //The size of the draw attribute data in bytes per element. Multiple elements can be uploaded.
        std::uint32_t size;
    };

    static const std::unordered_map<DrawAttribute, DrawAttributeInfo> DRAW_ATTRIBUTE_INFO =
    {
        {DrawAttribute::TRANSFORMATION_MATRIX, {"DYNAMIC_TRANSFORMMATRIX_DEFINE", DrawAttributeUsageType::PER_INSTANCE, sizeof(glm::mat4)}},
        {DrawAttribute::NORMAL_MATRIX, {"DYNAMIC_NORMALMATRIX_DEFINE", DrawAttributeUsageType::PER_INSTANCE, sizeof(glm::mat4)}},
        {DrawAttribute::UV_MODIFIER, {"DYNAMIC_UVMODIFIER_DEFINE", DrawAttributeUsageType::GLOBAL_ARRAY, sizeof(glm::vec4)}},
        {DrawAttribute::MATERIAL_SINGLE, {"MAT_SINGLE_DEFINE", DrawAttributeUsageType::GLOBAL_SINGLE, 0}},
        {DrawAttribute::MATERIAL_BATCH, {"MAT_BATCH_DEFINE", DrawAttributeUsageType::GLOBAL_SINGLE, 0}},
    };


    struct DrawAttributeMask
    {
    public:
        DrawAttributeMask()
        {
            m_Mask = static_cast<DrawAttribute>(0u);
        }

    public:
        /*
         * Enable the given attribute.
         */
        DrawAttributeMask& EnableAttribute(DrawAttribute a_Attribute);

        /*
         * Disable the given attribute.
         */
        DrawAttributeMask& DisableAttribute(DrawAttribute a_Attribute);

        /*
         * Returns true if the given attribute is enabled.
         */
        bool IsAttributeEnabled(DrawAttribute a_Attribute) const;

        /*
         * Get the current dynamic attribute mask.
         */
        std::uint32_t GetMask() const;

    private:
        DrawAttribute m_Mask;
    };

    /*
     * Datastruct used to describe a draw call for a mesh.
     * References to the mesh, instance count and material used are all stored inside.
     */
    struct DrawData
    {
        DrawData()
        {
            instanceCount = 1;
        }

        /*
         * Shared pointer to the mesh resource to draw.
         */
        std::shared_ptr<Mesh> mesh;

        /*
         * The amount of instances to draw of this mesh.
         * This has to be at least 1, and correspond to the amount of transforms in the dynamic data.
         */
        std::uint32_t instanceCount;

        /*
         * The dynamic attribute object used to enable and disable certain data.
         * This is used as a mask in the shader to determine which data can be expected in the below buffers.
         */
        DrawAttributeMask attributes;

        //Materials.
        struct
        {
            /*
             * The material to use for drawing.
             * Either enable this, batch or none.
             */
            std::shared_ptr<Material> material;

            /*
             * The material batch to use for drawing.
             * Either enable this, single material or none.
             */
            std::shared_ptr<MaterialBatch> materialBatch;

        } materialData;

        //Transforms.
        struct
        {
            /*
             * GpuBuffer that contains the transform information.
             */
            std::shared_ptr<GpuBuffer> dataBuffer;

            /*
             * Pointer into the GpuBuffer provided where the transformation matrices are stored.
             */
            GpuBufferView dataRange;

        } transformData;

        //Uv modifiers.
        struct
        {
            /*
             * GpuBuffer that contains the uv modifier information.
             */
            std::shared_ptr<GpuBuffer> dataBuffer;

            /*
             * Pointer into the GpuBuffer provided where the uv modifiers are stored.
             */
            GpuBufferView dataRange;

        } uvModifierData;
    };

    /*
     * A combination of a pointer to a DrawData array and the amount of DrawData elements in that array.
     * Passed to render passes to then perform drawing.
     */
    struct DrawDataSet
    {
        DrawDataSet()
        {
            drawDataPtr = nullptr;
            drawDataCount = 0u;
        }

        DrawDataSet(DrawData* a_DrawData, std::uint32_t a_Count) : drawDataPtr(a_DrawData), drawDataCount(a_Count) {}

        /*
         * Pointer to the draw data start.
         */
        DrawData* drawDataPtr;

        /*
         * The amount of drawData objects in the dataset.
         */
        std::uint32_t drawDataCount;
    };

    /*
     * Information about light data that has been uploaded to the GPU.
     */
    struct LightDataInfo
    {
        LightDataInfo()
        {
            count = 0;
        }

        LightDataInfo(std::uint32_t a_Count, const GpuBufferView& a_DataRange, const std::shared_ptr<GpuBuffer>& a_DataBuffer) : count(a_Count), dataRange(a_DataRange), dataBuffer(a_DataBuffer)
        {
            
        }

        //The amount of lights in the dataRange.
        std::uint32_t count;
        std::uint32_t shadowCount;

        //The memory offsets into the dataBuffer where the lights are stored.
        GpuBufferView dataRange;

        //The databuffer in which the lights are stored.
        std::shared_ptr<GpuBuffer> dataBuffer;
    };

    /*
     * Struct containing information about the lights in a scene.
     * Lights have to be uploaded to the GPU by the user to minimize uploading overhead.
     */
    struct LightData
    {
        /*
         * Information regarding point lights without shadows.
         */
        LightDataInfo pointLights;

        /*
         * Information regarding spot lights without shadows.
         */
        LightDataInfo spotLights;

        /*
         * Information regarding directional lights without shadows.
         */
        LightDataInfo directionalLights;

        /*
         * The amount of ambient light in the scene.
         */
        glm::vec3 ambient;
    };

    /*
     * Structure containing information about the shadow casting used in the scene.
     */
    struct ShadowData
    {
        ShadowData()
        {
            directional.numCascades = 1;
        }

        //Shadowmaps for point and spot lights.
        struct
        {
            //Texture containing the shadow maps. Has to be Texture_Cube_Array.
            std::shared_ptr<Texture> shadowMaps;
        } positional;

        //Shadowmaps for directional lights.
        struct
        {
            std::shared_ptr<Texture> shadowMaps;            //The shadow map texture. Has to be a Texture_2D_Array.
            std::shared_ptr<GpuBuffer> dataBuffer;           //The buffer containing the transformation matrices for the lights and all the 
            std::shared_ptr<GpuBufferView> dataRange;       //The view into above buffer to where the matrices are stored.
            std::uint32_t numCascades;                      //The amount of cascades used with directional shadows.
        } directional;
    };

    /*
     * Struct used to upload light data to the GPU.
     */
    struct LightUploadData
    {
        LightUploadData()
        {
            lightData = nullptr;
            point.count = 0;
            point.lights = nullptr;
            spot.count = 0;
            spot.lights = nullptr;
            directional.count = 0;
            directional.lights = nullptr;
        }

        /*
         * Pointer to the LightData object that is to be filled with the references to the information residing on the GPU.
         */
        LightData* lightData;

        //Pointlights in the scene.
        struct
        {
            std::shared_ptr<PointLight>* lights;
            std::uint32_t count;
        } point;

        //Spotlights in the scene.
        struct
        {
            std::shared_ptr<SpotLight>* lights;
            std::uint32_t count;
        } spot;

        //Directional lights in the scene.
        struct
        {
            std::shared_ptr<DirectionalLight>* lights;
            std::uint32_t count;
        } directional;
    };

    /*
     * Structure that can be passed to a GpuBuffer to upload per instance data for a DrawData object to the GPU.
     * The DrawData object is then correctly modified to reference the right buffer and
     */
    struct PerInstanceUploadData
    {
        PerInstanceUploadData() : drawData(nullptr), transforms(nullptr), normalMatrices(nullptr) {}
        PerInstanceUploadData(DrawData* a_DrawData) : drawData(a_DrawData), transforms(nullptr), normalMatrices(nullptr) {}
        PerInstanceUploadData(DrawData& a_DrawData) : drawData(&a_DrawData), transforms(nullptr), normalMatrices(nullptr) {}
        PerInstanceUploadData(DrawData* a_DrawData, glm::mat4* a_Transforms) : drawData(a_DrawData), transforms(a_Transforms), normalMatrices(nullptr) {}
        PerInstanceUploadData(DrawData& a_DrawData, glm::mat4& a_Transforms) : drawData(&a_DrawData), transforms(&a_Transforms), normalMatrices(nullptr) {}
        PerInstanceUploadData(DrawData* a_DrawData, glm::mat4* a_Transforms, glm::mat4* a_NormalMatrices) : drawData(a_DrawData), transforms(a_Transforms), normalMatrices(a_NormalMatrices) {}
        PerInstanceUploadData(DrawData& a_DrawData, glm::mat4& a_Transforms, glm::mat4& a_NormalMatrices) : drawData(&a_DrawData), transforms(&a_Transforms), normalMatrices(&a_NormalMatrices) {}

        /*
         * Pointer to the draw data to upload the instance data for.
         * Always required.
         */
        DrawData* drawData;

        /*
         * Pointer to the start of the transform matrices array to upload.
         * Size of the array has to be equal to the amount of instances in the DrawData.
         *
         * Only required if the transform draw data attribute is enabled.
         */
        glm::mat4* transforms;

        /*
         * Pointer to the start of the normal matrices array to upload.
         * Size has to correspond to the instance count in the drawData.
         * Only required if the normal matrix draw attribute is enabled for this draw data.
         */
        glm::mat4* normalMatrices;
    };

    /*
     * Structure that can be passed to a GpuBuffer to upload global information for a DrawData object.
     */
    struct GlobalUploadData
    {
        GlobalUploadData() : drawData(nullptr), uvModifiers(nullptr) {}
        GlobalUploadData(DrawData* a_DrawData) : drawData(a_DrawData), uvModifiers(nullptr) {}
        GlobalUploadData(DrawData& a_DrawData) : drawData(&a_DrawData), uvModifiers(nullptr) {}
        GlobalUploadData(DrawData* a_DrawData, glm::vec4* a_UvModifiers) : drawData(a_DrawData), uvModifiers(a_UvModifiers) {}
        GlobalUploadData(DrawData& a_DrawData, glm::vec4& a_UvModifiers) : drawData(&a_DrawData), uvModifiers(&a_UvModifiers) {}

        /*
         * The draw data to upload the globally accessible data for.
         */
        DrawData* drawData;

        /*
         * The start of the array of UVModifier float4's.
         * The UV Mod draw attribute has to be enabled for this.
         */
        glm::vec4* uvModifiers;
    };

    /*
     * Struct detailing a region in a texture to be cleared.
     * Used with RenderPass_Clear.
     */
    struct ClearData
    {
        ClearData()
        {
            offset = glm::vec3(0.f);
            size = glm::vec3(1.f);
            clearValue.floats = glm::vec4(1.f);
        }

        //Offset where X, Y and Z start for the region to be cleared.
        glm::vec3 offset;

        //X, Y and Z sizes to be cleared.
        glm::vec3 size;

        //The value to clear the data with. Only the first n values apply for a buffer with n channels. For example, X is used when only depth is enabled.
        //Use the correct data type to clear depending on the pixel format and data type used in the buffer that is being cleared.
        union
        {
            glm::vec4 floats;
            glm::vec<4, unsigned char> uChars;
            glm::vec<4, char> chars;
            glm::vec<4, int> ints;
            glm::vec<4, unsigned int> uints;
            glm::vec<4, short> shorts;
            glm::vec<4, unsigned short> ushorts;

        } clearValue;
    };

    /*
     * Information about a vertex attribute.
     */
    struct VertexAttributeData
    {
        VertexAttributeData()
        {
            byteOffset = 0;
            byteStride = 0;
            normalize = false;
            instanceDivisor = 0;
        }

        //The offset from the start of the buffer to the first vertex attribute of this type.
        std::uint32_t byteOffset;

        //The stride in bytes between the data elements. When 0, they are tightly packed.
        std::uint32_t byteStride;

        //Normalize or not (between 0 and 1).
        bool normalize;

        //The instance divisor of this attribute. If 0, no instancing is used.
        //Any other value indicates after how many full instance draws the attribute updates in the shader.
        std::uint16_t instanceDivisor;
    };

    /*
     * Information about each vertex attribute.
     * This is statically stored in a lookup table to be queried at runtime.
     */
    struct VertexAttributeInfo
    {
        //The amount of elements for this vertex attribute.
        std::uint32_t numElements;

        //The data type of each element.
        DataType dataType;

        //The unique name of this vertex attribute
        std::string name;

        //The define name used in preprocessor definitions in shaders.
        std::string defineName;

        //The define used to determine the position of this vertex attribute.
        std::string locationDefine;
    };

}
