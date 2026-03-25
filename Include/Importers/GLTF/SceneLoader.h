#pragma once
#include <filesystem>

#include "Image/Image.h"
#include "Math/RMath.h"
#include "Modeling/Mesh.h"
#include "Rendering/IndexBuffer.h"
#include "Rendering/MeshObject.h"
#include "Rendering/Textures.h"
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
        // todo clearcoat
        
        using Color = Math::Vector4f;
        using Texture = size_t;

        Color color =           {1,1,1,1};
        Color emissive;
        float metallic =        0.0f;
        float roughness =       0.8f;
        float specular =        0.0f;
        Color specularColor;
        float transmission =    0.0f;
        float ior =             0.0f;
        float thickness =       0.0f;
        float attenuation =     0.0f;
        Color attenuationColor;
        bool twoSided = false;

        Texture colorTexture = UINT64_MAX;
        Texture emissiveTexture = UINT64_MAX;
        Texture metallicRoughnessTexture = UINT64_MAX;
        Texture occlusionTexture = UINT64_MAX;
        Texture normalTexture = UINT64_MAX;
        Texture specularTexture = UINT64_MAX;
        Texture specularColorTexture = UINT64_MAX;
        Texture transmissionTexture = UINT64_MAX;
        Texture thicknessTexture = UINT64_MAX;
    };
    
    struct MeshInstance
    {
        size_t mesh;
        size_t material;
        size_t transform;
        uint8_t vertexGroup;
    };
    
    struct CPUScene
    {        
        std::vector<Image> textures;
        
        std::vector<Mesh> meshes;

        std::vector<Material> materials;

        std::vector<Math::WorldTransformF> transforms;

        std::vector<MeshInstance> instances;
    };
    
    bool LoadCPUScene(const std::filesystem::path& path, CPUScene& scene);
    
    struct GPUScene
    {        
        std::vector<Texture2D> textures;
        
        std::vector<MeshObject> meshes;

        std::vector<Material> materials;

        std::vector<Math::WorldTransformF> transforms;

        std::vector<MeshInstance> instances;
    };
    
    bool LoadGPUScene(const std::filesystem::path& path, GPUScene& scene);
}
