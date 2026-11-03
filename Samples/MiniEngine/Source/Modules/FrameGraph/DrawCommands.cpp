#include "Modules/FrameGraph/Helpers/DrawCommands.h"

#include "Modules/Rendering/Tools/FrustumCulling.h"
#include "Rendering/Uniforms.h"

namespace FrameGraph
{
    void DrawGLTFScene(const CommandContext& Resources, const Pipeline& Pipeline, FrameGraph::Location Camera, const DrawGLTFSettings& Settings)
    {
        // Scene storage buffers
        if (Settings.CurrentCameraBufferBinding >= 0) SetUniform(Settings.CurrentCameraBufferBinding, Resources.GetCameraBuffer());
        if (Settings.PreviousCameraBufferBinding >= 0 && Resources.GetPreviousCamerasBuffer())
        {
            SetUniform(Settings.PreviousCameraBufferBinding, *Resources.GetPreviousCamerasBuffer());
        }
        
        GLint GLTFBaseColor, GLTFRoughness, GLTFMetalness, GLTFEmissiveColor, GLTFSpecularColor, GLTFMaterialFlags;
        GLint  GLTFUseColorTexture, GLTFUseNormalTexture, GLTFUseMRTexture, GLTFUseAOTexture, GLTFUseSpecularTexture, GLTFUseEmissiveTexture;
        GLint GLTFTexColor, GLTFTexNormal, GLTFTexMR, GLTFTexAO, GLTFTexSpecular, GLTFTexEmissive;
        GLint GLTFParamBuffer = -1;
        if (Settings.BindMaterial)
        {
            if (Resources.Scene().Extension & GLTF::GPUScene::MaterialsAsBuffers)
            {
                GLTFParamBuffer = GetUniformLocation(Pipeline, "ParamBuffer");
            }
            else if (Resources.Scene().Extension & GLTF::GPUScene::MaterialsAsUnifiedBuffer)
            {
                AssertOrError(false, "Feature not yet supported")
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
        
        for (const GLTF::MeshInstance& Instance : Resources.Scene().instances)
        {
            const MeshObject& Mesh = Resources.Scene().meshes[Instance.mesh];
            const Mesh::VertexGroup& Group = Mesh.GetGroups()[Instance.vertexGroup];
            const GLTF::Transform& Transform = Resources.Scene().transforms[Instance.transform];
            const GLTF::Material& Material = Resources.Scene().materials[Instance.material];
            const UniformBuffer* MaterialBuffer = Resources.Scene().Extension & GLTF::GPUScene::MaterialsAsBuffers ?
                &Resources.Scene().materialUniformBuffers[Instance.material] :
                nullptr;
            
            // if ((Settings.MaterialFlagsRequireFilter & GLTF::Material::Transparent) && ((Material.flags & GLTF::Material::Transparent) != GLTF::Material::None)) {EngineRuntimeBREAKPOINT}
            
            // Filter by material
            if ((Settings.MaterialFlagsExcludeFilter & Material.flags) != GLTF::Material::None) continue; 
            if ((Settings.MaterialFlagsRequireFilter & Material.flags) != Settings.MaterialFlagsRequireFilter) continue;
            
            switch (Transform.Type)
            {
            case GLTF::Transform::Properties:
                {
                    Math::Transform4f TransformMatrix = Transform.Value.asProperties.GetTransform();
                
                    if (Settings.FrustumCulling && !Rendering::frustumCullingTest(Resources.GetCamera(Camera).Camera_WorldToProj(), TransformMatrix, Group.BoundsMin, Group.BoundsMax)) continue;
                
                    SetUniform(GLTFModelMatrix, TransformMatrix);
                }
            break;
                    
            case GLTF::Transform::Matrix:
                {
                    if (Settings.FrustumCulling && !Rendering::frustumCullingTest(Resources.GetCamera(Camera).Camera_WorldToProj(), Transform.Value.asMatrix, Group.BoundsMin, Group.BoundsMax)) continue;
                
                    SetUniform(GLTFModelMatrix, Transform.Value.asMatrix);
                }
            break;
                    
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
            }
            
            // Material
            if (Settings.BindMaterial)
            {
                if (Resources.Scene().Extension & GLTF::GPUScene::MaterialsAsBuffers)
                {
                    SetUniform(GLTFParamBuffer, MaterialBuffer);
                }
                else if (Resources.Scene().Extension & GLTF::GPUScene::MaterialsAsUnifiedBuffer)
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

            
            Bind(Mesh.GetVAO());
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
            
            UnBind(Mesh.GetVAO());
        }
    }
}
