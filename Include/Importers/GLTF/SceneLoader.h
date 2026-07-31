#pragma once
#include <filesystem>

#include "Image/Image.h"
#include "Math/RMath.h"
#include "Modeling/Mesh.h"
#include "Rendering/IndexBuffer.h"
#include "Rendering/MeshObject.h"
#include "Rendering/Textures.h"
#include "Rendering/UniformBuffer.h"
#include "Rendering/VertexArrayObject.h"
#include "Rendering/VertexBuffer.h"

namespace GLTF
{
    enum Properties : uint8_t
    {
        None =          0,
        
        CastShadows =   1 << 0,
        
        IsOpaque =      0 << 1,
        IsMasked =      1 << 1,
        IsTransparent = 2 << 1,
        
        IsTwoSided =    1 << 3,
    };
    
    /**
     * Description of a PBR Material (using Principled BRDF)
     *
     * using the following extensions:
     *      - KHR_materials_ior: https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_ior
     *      - KHR_materials_specular: https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_specular
     *      - KHR_materials_transmission: https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_transmission
     *      - KHR_materials_clearcoat: https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_clearcoat
     */
    struct Material
    {
        using Color = Math::Vector4f;
        using Texture = size_t;

        Color color =           {1,1,1,1};
        Color specularColor =   {1,1,1,1};
        Color emissive =        {0,0,0,1};
        Color attenuationColor;
        
        float metallic =        0.0f;
        float roughness =       0.8f;
        float specular =        0.0f;
        float transmission =    0.0f;
        
        float ior =             0.0f;
        float thickness =       0.0f;
        float attenuation =     0.0f;
        enum EFlags : uint32_t
        {
            None =                  0,
            TwoSided =              1 << 0,
            Masked =                1 << 1,
            Transparent =           1 << 2,
            NoShadows =             1 << 3,
            UseSpecularExt =        1 << 4,
            UseTransmissionExt =    1 << 5,
            UseClearCoat =          1 << 6,  // todo clearcoat
        } flags = None;

        Texture colorTexture = UINT64_MAX;
        Texture emissiveTexture = UINT64_MAX;
        Texture metallicRoughnessTexture = UINT64_MAX;
        Texture occlusionTexture = UINT64_MAX;
        Texture normalTexture = UINT64_MAX;
        Texture specularTexture = UINT64_MAX;
        Texture specularColorTexture = UINT64_MAX;
        Texture transmissionTexture = UINT64_MAX;
        Texture thicknessTexture = UINT64_MAX;
        
        Texture EXT_colorTextureBin = UINT64_MAX;
        Texture EXT_emissiveTextureBin = UINT64_MAX;
        Texture EXT_metallicRoughnessTextureBin = UINT64_MAX;
        Texture EXT_occlusionTextureBin = UINT64_MAX;
        Texture EXT_normalTextureBin = UINT64_MAX;
        Texture EXT_specularTextureBin = UINT64_MAX;
        Texture EXT_specularColorTextureBin = UINT64_MAX;
        Texture EXT_transmissionTextureBin = UINT64_MAX;
        Texture EXT_thicknessTextureBin = UINT64_MAX;
        
        uint32_t EXT_colorTextureBinIndex = UINT32_MAX;
        uint32_t EXT_emissiveTextureBinIndex = UINT32_MAX;
        uint32_t EXT_metallicRoughnessTextureBinIndex = UINT32_MAX;
        uint32_t EXT_occlusionTextureBinIndex = UINT32_MAX;
        uint32_t EXT_normalTextureBinIndex = UINT32_MAX;
        uint32_t EXT_specularTextureBinIndex = UINT32_MAX;
        uint32_t EXT_specularColorTextureBinIndex = UINT32_MAX;
        uint32_t EXT_transmissionTextureBinIndex = UINT32_MAX;
        uint32_t EXT_thicknessTextureBinIndex = UINT32_MAX;
    };
    
    INLINE Material::EFlags operator|(Material::EFlags a, Material::EFlags b) {return static_cast<Material::EFlags>(static_cast<int>(a) | static_cast<int>(b)); }
    INLINE Material::EFlags operator&(Material::EFlags a, Material::EFlags b) {return static_cast<Material::EFlags>(static_cast<int>(a) & static_cast<int>(b)); }
    
    struct MeshInstance
    {
        size_t mesh;
        size_t material;
        size_t transform;
        uint8_t vertexGroup;
    };
    
    struct Transform
    {
        enum TransformType
        {
            Properties, Matrix
        } Type;
        union TransformData
        {
            Math::WorldTransformF asProperties;
            Math::Transform4f asMatrix;
        } Value;
        
        Transform() : Type(Properties), Value(Math::WorldTransformF{}) {}

        Transform(const Transform& Other);

        Transform(Transform&& Other) noexcept;

        Transform& operator=(const Transform& Other);

        Transform& operator=(Transform&& Other) noexcept;
    };
    
    // TODO move out of namespace
    struct CPUScene
    {        
        std::vector<Image> textures;
        
        std::vector<Mesh> meshes;

        std::vector<Material> materials;

        std::vector<Transform> transforms;

        std::vector<MeshInstance> instances;
    };

    // TODO move out of namespace
    struct GPUScene
    {
        enum Extensions : uint32_t
        {
            ExNone =                    0,
            MaterialsAsBuffers =        1 << 0,
            MaterialsAsUnifiedBuffer =  1 << 1,
            TexturesAsBindlessArrays =  1 << 2,
        } Extension = ExNone;
        
        std::vector<Texture2D> textures;
        
        std::vector<MeshObject> meshes;

        std::vector<Material> materials;
        
        std::vector<UniformBuffer> materialUniformBuffers;
        
        std::optional<UniformBuffer> unifiedMaterialBuffer;
        std::vector<size_t> unifiedMaterialOffsets;

        std::vector<Transform> transforms;

        std::vector<MeshInstance> instances;
        
        std::vector<Texture2DArray> texturesArrays;
        
        void Clear()
        {
            Extension = GLTF::GPUScene::ExNone;
            textures.clear();
            meshes.clear();
            materials.clear();
            materialUniformBuffers.clear();
            unifiedMaterialBuffer.reset();
            unifiedMaterialOffsets.clear();
            transforms.clear();
            instances.clear();
            texturesArrays.clear();
        }
    };
    
    bool LoadCPUScene(const std::filesystem::path& path, CPUScene& scene);
    
    bool LoadGPUScene(const std::filesystem::path& path, GPUScene& scene);
}

void EnableMaterialAsBuffers(GLTF::GPUScene& scene);
void EnableMaterialAsUnifiedBuffer(GLTF::GPUScene& scene);
void EnableTexturesAsBindlessArrays(GLTF::GPUScene& scene);

// Graphics Box Scene is a baked representation of the gltf format stripped of its modularity and arrange in a way that already matches GPU Objects
// This format is intended for cooked content and not versioned transfer data.
namespace GBS
{
    bool LoadGPUScene(const std::filesystem::path& path, GLTF::GPUScene& scene);
    
    struct ExportSettings
    {
        enum Flags : uint32_t
        {
            None =                  0,
            ImageCompression =      1 << 0,
            ImageCompressionJPG =   1 << 1,
            ImageCompressionEXR =   1 << 2,
        } flags;
    };
    bool SaveGPUScene(const std::filesystem::path& path, const GLTF::GPUScene& scene, ExportSettings settings);
}
