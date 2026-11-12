#include "Modules/FrameGraph/Helpers/DrawCommands.h"

#include "Modules/Rendering/Tools/FrustumCulling.h"
#include "Rendering/Debug.h"
#include "Rendering/Uniforms.h"

namespace FrameGraph
{
    void DrawGLTFScene(const CommandContext& Resources, const Pipeline& Pipeline, FrameGraph::Location Camera, const DrawGLTFSettings& Settings)
    {
        size_t PreviousMesh = std::numeric_limits<size_t>::max(), PreviousMaterial = std::numeric_limits<size_t>::max();
        
        // Scene storage buffers
        if (Settings.CurrentCameraBufferBinding >= 0) SetUniform(Settings.CurrentCameraBufferBinding, Resources.GetCameraBuffer());
        if (Settings.PreviousCameraBufferBinding >= 0 && Resources.GetPreviousCamerasBuffer())
        {
            SetUniform(Settings.PreviousCameraBufferBinding, *Resources.GetPreviousCamerasBuffer());
        }
        
        GLint GLTFBaseColor, GLTFRoughness, GLTFMetalness, GLTFEmissiveColor, GLTFSpecularColor, GLTFMaterialFlags;
        GLint  GLTFUseColorTexture, GLTFUseNormalTexture, GLTFUseMRTexture, GLTFUseAOTexture, GLTFUseSpecularTexture, GLTFUseEmissiveTexture;
        GLint GLTFTexColor, GLTFTexNormal, GLTFTexMR, GLTFTexAO, GLTFTexSpecular, GLTFTexEmissive;
        if (Settings.BindMaterial)
        {
            if (Resources.Scene().Extension & Scene::GPUScene::MaterialsAsBuffers)
            {
            }
            else if (Resources.Scene().Extension & Scene::GPUScene::MaterialsAsUnifiedBuffer)
            {
            }
            else
            {
                GLTFBaseColor = GetUniformLocation(Pipeline, "BaseColor");
                GLTFEmissiveColor = GetUniformLocation(Pipeline, "EmissiveColor");
                GLTFSpecularColor = GetUniformLocation(Pipeline, "SpecularColor");
                GLTFRoughness = GetUniformLocation(Pipeline, "Roughness");
                GLTFMetalness = GetUniformLocation(Pipeline, "Metalness");
                GLTFMaterialFlags = GetUniformLocation(Pipeline, "MaterialFlags");
            
                GLTFUseColorTexture = GetUniformLocation(Pipeline, "UseColorTexture");
                GLTFUseNormalTexture = GetUniformLocation(Pipeline, "UseNormalTexture");
                GLTFUseMRTexture = GetUniformLocation(Pipeline, "UseMRTexture");
                GLTFUseAOTexture = GetUniformLocation(Pipeline, "UseAOTexture");
                GLTFUseSpecularTexture = GetUniformLocation(Pipeline, "UseSpecularTexture");
                GLTFUseEmissiveTexture = GetUniformLocation(Pipeline, "UseEmissiveTexture");
            }
            GLTFTexColor = GetUniformLocation(Pipeline, "texColor");
            GLTFTexNormal = GetUniformLocation(Pipeline, "texNormal");
            GLTFTexMR = GetUniformLocation(Pipeline, "texMR");
            GLTFTexAO = GetUniformLocation(Pipeline, "texAO");
            GLTFTexSpecular = GetUniformLocation(Pipeline, "texSpecular");
            GLTFTexEmissive = GetUniformLocation(Pipeline, "texEmissive");
        }
        
        GLint GLTFModelMatrix = GetUniformLocation(Pipeline, "Model");
        
        bool UseDrawCallList = !Settings.ElementListToDraw.empty();
        for (size_t i = 0, iend = UseDrawCallList ? Settings.ElementListToDraw.size() : Resources.Scene().instances.size(); i < iend; i++)
        {
            if (UseDrawCallList)
            {
                if (Settings.ElementListToDraw[i] == std::numeric_limits<uint32_t>::max()) break;
            }
            
            size_t instIndex = UseDrawCallList ? Settings.ElementListToDraw[i] : i;
            const Scene::MeshInstance& Instance = Resources.Scene().instances[instIndex];
            
            const MeshObject& Mesh = Resources.Scene().meshes[Instance.mesh];
            const Mesh::VertexGroup& Group = Mesh.GetGroups()[Instance.vertexGroup];
            const Scene::Transform& Transform = Resources.Scene().transforms[Instance.transform];
            const GLTF::Material& Material = Resources.Scene().materials[Instance.material];
            const UniformBuffer* MaterialBuffer = Resources.Scene().Extension & Scene::GPUScene::MaterialsAsBuffers ?
                &Resources.Scene().materialUniformBuffers[Instance.material] :
                nullptr;
            
            // if ((Settings.MaterialFlagsRequireFilter & GLTF::Material::Transparent) && ((Material.flags & GLTF::Material::Transparent) != GLTF::Material::None)) {EngineRuntimeBREAKPOINT}
            
            // Filter by material
            if ((Settings.Filters.MaterialFlagsExclude & Material.flags) != GLTF::Material::None) continue; 
            if ((Settings.Filters.MaterialFlagsRequire & Material.flags) != Settings.Filters.MaterialFlagsRequire) continue;
            
            switch (Transform.Type)
            {
            case Scene::Transform::Properties:
                {
                    Math::Transform4f TransformMatrix = Transform.Value.asProperties.GetTransform();
                
                    if (Settings.Filters.FrustumCulling && !Rendering::frustumCullingTest(Resources.GetCamera(Camera).Camera_WorldToProj(), TransformMatrix, Group.BoundsMin, Group.BoundsMax)) continue;
                
                    SetUniform(GLTFModelMatrix, TransformMatrix);
                }
            break;
                    
            case Scene::Transform::Matrix:
                {
                    if (Settings.Filters.FrustumCulling && !Rendering::frustumCullingTest(Resources.GetCamera(Camera).Camera_WorldToProj(), Transform.Value.asMatrix, Group.BoundsMin, Group.BoundsMax)) continue;
                
                    SetUniform(GLTFModelMatrix, Transform.Value.asMatrix);
                }
            break;
                    
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
            }
            
            if (Resources.Scene().Extension & Scene::GPUScene::NamedResources)
            {
                if (PreviousMaterial != std::numeric_limits<size_t>::max() && PreviousMaterial != Instance.material) PopDebugMarker(); // Mesh
                if (Settings.BindMaterial && PreviousMesh != std::numeric_limits<size_t>::max() && PreviousMesh != Instance.mesh) PopDebugMarker(); // Material
            }
            
            // Material
            if (Settings.BindMaterial)
            {
                if (PreviousMaterial == std::numeric_limits<size_t>::max() || PreviousMaterial != Instance.material)
                {
                    if (Resources.Scene().Extension & Scene::GPUScene::NamedResources)
                    {
                        PushDebugMarker(Resources.Scene().materialNames[Instance.material]);
                    }
                    
                    if (Resources.Scene().Extension & Scene::GPUScene::MaterialsAsBuffers)
                    {
                        SetUniform(Settings.GLTFMaterialBufferBinding, *MaterialBuffer);
                    }
                    else if (Resources.Scene().Extension & Scene::GPUScene::MaterialsAsUnifiedBuffer)
                    {
                        AssertOrError(false, "Feature not yet supported")
                    }
                    else
                    {
                        SetUniform(GLTFBaseColor, Material.color);
                        SetUniform(GLTFSpecularColor, Material.specularColor.xyz());
                        SetUniform(GLTFEmissiveColor, Material.emissive.xyz());
                        SetUniform(GLTFRoughness, Material.roughness);
                        SetUniform(GLTFMetalness, Material.metallic);
                        SetUniform(GLTFMaterialFlags, (uint32_t)Material.flags);
                    
                        SetUniform(GLTFUseColorTexture, Material.colorTexture != UINT64_MAX);
                        SetUniform(GLTFUseNormalTexture, Material.normalTexture != UINT64_MAX);
                        SetUniform(GLTFUseMRTexture, Material.metallicRoughnessTexture != UINT64_MAX);
                        SetUniform(GLTFUseAOTexture, Material.occlusionTexture != UINT64_MAX);
                        SetUniform(GLTFUseSpecularTexture, Material.specularTexture != UINT64_MAX);
                        SetUniform(GLTFUseEmissiveTexture, Material.emissiveTexture != UINT64_MAX);
                    }
                
                    if (Settings.MaterialSampler)
                    {
                        if (Material.colorTexture != UINT64_MAX)                SetUniform(GLTFTexColor, Settings.TextureUnitOffset + 0, Resources.Scene().textures[Material.colorTexture], *Settings.MaterialSampler);
                        if (Material.normalTexture != UINT64_MAX)               SetUniform(GLTFTexNormal, Settings.TextureUnitOffset + 1, Resources.Scene().textures[Material.normalTexture], *Settings.MaterialSampler);
                        if (Material.metallicRoughnessTexture != UINT64_MAX)    SetUniform(GLTFTexMR, Settings.TextureUnitOffset + 2, Resources.Scene().textures[Material.metallicRoughnessTexture], *Settings.MaterialSampler);
                        if (Material.occlusionTexture != UINT64_MAX)            SetUniform(GLTFTexAO, Settings.TextureUnitOffset + 3, Resources.Scene().textures[Material.occlusionTexture], *Settings.MaterialSampler);
                        if (Material.specularTexture != UINT64_MAX)             SetUniform(GLTFTexSpecular, Settings.TextureUnitOffset + 4, Resources.Scene().textures[Material.specularTexture], *Settings.MaterialSampler);
                        if (Material.emissiveTexture != UINT64_MAX)             SetUniform(GLTFTexEmissive, Settings.TextureUnitOffset + 5, Resources.Scene().textures[Material.emissiveTexture], *Settings.MaterialSampler);
                    }
                    else
                    {
                        if (Material.colorTexture != UINT64_MAX)                SetUniform(GLTFTexColor, Settings.TextureUnitOffset + 0, Resources.Scene().textures[Material.colorTexture]);
                        if (Material.normalTexture != UINT64_MAX)               SetUniform(GLTFTexNormal, Settings.TextureUnitOffset + 1, Resources.Scene().textures[Material.normalTexture]);
                        if (Material.metallicRoughnessTexture != UINT64_MAX)    SetUniform(GLTFTexMR, Settings.TextureUnitOffset + 2, Resources.Scene().textures[Material.metallicRoughnessTexture]);
                        if (Material.occlusionTexture != UINT64_MAX)            SetUniform(GLTFTexAO, Settings.TextureUnitOffset + 3, Resources.Scene().textures[Material.occlusionTexture]);
                        if (Material.specularTexture != UINT64_MAX)             SetUniform(GLTFTexSpecular, Settings.TextureUnitOffset + 4, Resources.Scene().textures[Material.specularTexture]);
                        if (Material.emissiveTexture != UINT64_MAX)             SetUniform(GLTFTexEmissive, Settings.TextureUnitOffset + 5, Resources.Scene().textures[Material.emissiveTexture]);
                    }
                }
            }
            
            if (PreviousMesh == std::numeric_limits<size_t>::max() || PreviousMesh != Instance.mesh)
            {
                if (Resources.Scene().Extension & Scene::GPUScene::NamedResources)
                {
                    PushDebugMarker(Resources.Scene().meshNames[Instance.mesh]);
                }
                
                Bind(Mesh.GetVAO());
            }
            if (Mesh.GetIndexBuffer().has_value())
            {
                const IndexBuffer& indexBuffer = Mesh.GetIndexBuffer().value();
                Bind(indexBuffer);
            
                glDrawElements(ToGLGeometryType(Mesh.GetVertexType()), Group.VertexCount, ToGLIndexType(indexBuffer.GetIndexType()), (void*)(Group.FirstVertex * ToGLIndexSize(indexBuffer.GetIndexType())));
            
                UnBind(indexBuffer);
            }
            else
            {
                glDrawArrays(ToGLGeometryType(Mesh.GetVertexType()), Group.FirstVertex, Group.VertexCount);
            }
            
            PreviousMesh = Instance.mesh;
            PreviousMaterial = Instance.material;
            
            if (i + 1 >= iend)
            {
                UnBind(Mesh.GetVAO());
                
                if (Resources.Scene().Extension & Scene::GPUScene::NamedResources)
                {
                    PopDebugMarker(); // Mesh
                    if (Settings.BindMaterial) PopDebugMarker(); // Material
                }
            }
        }
    }

    size_t DrawGLTFElementListSize(const CommandContext& Resources, FrameGraph::Location Camera, const DrawGLTFFilters& Filters)
    {
        size_t count = 0;
        for (size_t i = 0, iend = Resources.Scene().instances.size(); i < iend; i++)
        {
            const Scene::MeshInstance& Instance = Resources.Scene().instances[i];
            
            const MeshObject& Mesh = Resources.Scene().meshes[Instance.mesh];
            const Mesh::VertexGroup& Group = Mesh.GetGroups()[Instance.vertexGroup];
            const Scene::Transform& Transform = Resources.Scene().transforms[Instance.transform];
            const GLTF::Material& Material = Resources.Scene().materials[Instance.material];
            
            // Filter by material
            if ((Filters.MaterialFlagsExclude & Material.flags) != GLTF::Material::None) continue; 
            if ((Filters.MaterialFlagsRequire & Material.flags) != Filters.MaterialFlagsRequire) continue;
            
            switch (Transform.Type)
            {
            case Scene::Transform::Properties:
                {
                    Math::Transform4f TransformMatrix = Transform.Value.asProperties.GetTransform();
                
                    if (Filters.FrustumCulling && !Rendering::frustumCullingTest(Resources.GetCamera(Camera).Camera_WorldToProj(), TransformMatrix, Group.BoundsMin, Group.BoundsMax)) continue;
                }
            break;
                    
            case Scene::Transform::Matrix:
                {
                    if (Filters.FrustumCulling && !Rendering::frustumCullingTest(Resources.GetCamera(Camera).Camera_WorldToProj(), Transform.Value.asMatrix, Group.BoundsMin, Group.BoundsMax)) continue;
                }
            break;
                    
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
            }
            
            count++;
        }
        
        return count;
    }

    void DrawGLTFElementListCollect(const CommandContext& Resources, FrameGraph::Location Camera, std::span<uint32_t> ElementList, const DrawGLTFFilters& Filters)
    {
        size_t CurrentElement = 0;
        for (size_t i = 0, iend = Resources.Scene().instances.size(); i < iend; i++)
        {
            const Scene::MeshInstance& Instance = Resources.Scene().instances[i];
            
            const MeshObject& Mesh = Resources.Scene().meshes[Instance.mesh];
            const Mesh::VertexGroup& Group = Mesh.GetGroups()[Instance.vertexGroup];
            const Scene::Transform& Transform = Resources.Scene().transforms[Instance.transform];
            const GLTF::Material& Material = Resources.Scene().materials[Instance.material];
            
            // Filter by material
            if ((Filters.MaterialFlagsExclude & Material.flags) != GLTF::Material::None) continue; 
            if ((Filters.MaterialFlagsRequire & Material.flags) != Filters.MaterialFlagsRequire) continue;
            
            switch (Transform.Type)
            {
            case Scene::Transform::Properties:
                {
                    Math::Transform4f TransformMatrix = Transform.Value.asProperties.GetTransform();
                
                    if (Filters.FrustumCulling && !Rendering::frustumCullingTest(Resources.GetCamera(Camera).Camera_WorldToProj(), TransformMatrix, Group.BoundsMin, Group.BoundsMax)) continue;
                }
            break;
                    
            case Scene::Transform::Matrix:
                {
                    if (Filters.FrustumCulling && !Rendering::frustumCullingTest(Resources.GetCamera(Camera).Camera_WorldToProj(), Transform.Value.asMatrix, Group.BoundsMin, Group.BoundsMax)) continue;
                }
            break;
                    
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
            }
            
            ElementList[CurrentElement++] = i;
        }
        
        // Mark the end of the drawcall list if bigger than required
        if (CurrentElement < ElementList.size()) ElementList[CurrentElement] = std::numeric_limits<uint32_t>::max();
    }

    void DrawGLTFElementListSort(const CommandContext& Resources, FrameGraph::Location Camera, std::span<uint32_t> ElementList, DrawGLTFSortingProfile SortProfile)
    {
        size_t ElementCount = 0;
        for (ElementCount = 0; ElementCount < ElementList.size(); ElementCount++)
        {
            if (ElementList[ElementCount] == std::numeric_limits<uint32_t>::max()) break;
        }
        
        switch (SortProfile)
        {
        case SP_SortForRenderingShadedElements:
            std::sort(ElementList.begin(), ElementList.begin() + ElementCount, [&Resources](uint32_t a, uint32_t b) -> bool
            {
                // return true if is less so false is greater or equal
                
                const Scene::MeshInstance& InstanceA = Resources.Scene().instances[a];
                const MeshObject& MeshA = Resources.Scene().meshes[InstanceA.mesh];
                // const Mesh::VertexGroup& GroupA = MeshA.GetGroups()[InstanceA.vertexGroup];
                // const Scene::Transform& TransformA = Resources.Scene().transforms[InstanceA.transform];
                // const GLTF::Material& MaterialA = Resources.Scene().materials[InstanceA.material];
                
                const Scene::MeshInstance& InstanceB = Resources.Scene().instances[b];
                const MeshObject& MeshB = Resources.Scene().meshes[InstanceB.mesh];
                // const Mesh::VertexGroup& GroupB = MeshB.GetGroups()[InstanceB.vertexGroup];
                // const Scene::Transform& TransformB = Resources.Scene().transforms[InstanceB.transform];
                // const GLTF::Material& MaterialB = Resources.Scene().materials[InstanceB.material];
                
                if (InstanceA.material != InstanceB.material)
                {
                    return InstanceA.material < InstanceB.material;
                }
                
                return InstanceA.mesh < InstanceB.mesh;
            });
            break;
            
        case SP_SortForRenderingDepthBuffer:
            {                
                std::sort(ElementList.begin(), ElementList.begin() + ElementCount, [&Resources, Camera](uint32_t a, uint32_t b) -> bool
                {
                    // return true if is less so false is greater or equal
                             
                    const Scene::MeshInstance& InstanceA = Resources.Scene().instances[a];
                    const MeshObject& MeshA = Resources.Scene().meshes[InstanceA.mesh];
                    const Mesh::VertexGroup& GroupA = MeshA.GetGroups()[InstanceA.vertexGroup];
                    const Scene::Transform& TransformA = Resources.Scene().transforms[InstanceA.transform];
                    // const GLTF::Material& MaterialA = Resources.Scene().materials[InstanceA.material];
                             
                    const Scene::MeshInstance& InstanceB = Resources.Scene().instances[b];
                    const MeshObject& MeshB = Resources.Scene().meshes[InstanceB.mesh];
                    const Mesh::VertexGroup& GroupB = MeshB.GetGroups()[InstanceB.vertexGroup];
                    const Scene::Transform& TransformB = Resources.Scene().transforms[InstanceB.transform];
                    // const GLTF::Material& MaterialB = Resources.Scene().materials[InstanceB.material];
                 
                    Math::Vector4f CenterA = Math::Vector4f(GroupA.BoundsMax - GroupA.BoundsMin, 1.f);
                    switch (TransformA.Type)
                    {
                    case Scene::Transform::Properties:
                        {
                            CenterA = Resources.GetCamera(Camera).Camera_WorldToProj.matrix * (TransformA.Value.asProperties.GetTransform() * CenterA);
                        }
                    break;
                                     
                    case Scene::Transform::Matrix:
                        {
                            CenterA = Resources.GetCamera(Camera).Camera_WorldToProj.matrix * (TransformA.Value.asMatrix * CenterA);
                        }
                    break;
                                     
                    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
                    }
                    CenterA.xyz() = CenterA.xyz() / CenterA.w;
                     
                    Math::Vector4f CenterB = Math::Vector4f(GroupB.BoundsMax - GroupB.BoundsMin, 1.f);
                    switch (TransformA.Type)
                    {
                    case Scene::Transform::Properties:
                        {
                            CenterB = Resources.GetCamera(Camera).Camera_WorldToProj.matrix * (TransformB.Value.asProperties.GetTransform() * CenterB);
                        }
                    break;
                                     
                    case Scene::Transform::Matrix:
                        {
                            CenterB = Resources.GetCamera(Camera).Camera_WorldToProj.matrix * (TransformB.Value.asMatrix * CenterB);
                        }
                    break;
                                     
                    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
                    }
                    CenterB.xyz() = CenterB.xyz() / CenterB.w;
                 
                    return CenterA.z < CenterB.z;
                });
            }
            break;
            
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported draw call list sorting profile")
        }
    }
}
