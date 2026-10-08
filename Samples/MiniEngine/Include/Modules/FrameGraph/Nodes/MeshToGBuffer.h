#pragma once

#include "Modules/FrameGraph/Commands.h"
#include "Importers/GLTF/SceneLoader.h"
#include "Modules/Rendering/Tools/FrustumCulling.h"
#include "Rendering/FrameBuffers.h"
#include "Rendering/Pipelines.h"
#include "Rendering/Sampler.h"
#include "Rendering/Uniforms.h"

namespace FrameGraph
{
    class MeshToGBuffer : public ICommand
    {
    public:
        MeshToGBuffer(CommandContext& Resources): 
            ICommand(Resources),
            GBufferSize(Resources.GetValue<Size2D>("Scene Radiance")),
            GBufferAlbedo(Resources.Add<Texture2D>("GBufferAlbedo", GBufferSize.x, GBufferSize.y, Texture::Type::Packed_R11F_G11F_B10F, Texture::Layout::RGB)),
            GBufferNormal(Resources.Add<Texture2D>("GBufferNormal", GBufferSize.x, GBufferSize.y, Texture::Type::Half, Texture::Layout::RGBA)),
            GBufferTangent(Resources.Add<Texture2D>("GBufferTangent", GBufferSize.x, GBufferSize.y, Texture::Type::Half, Texture::Layout::RGBA)),
            GBufferProperties(Resources.Add<Texture2D>("GBufferProperties", GBufferSize.x, GBufferSize.y, Texture::Type::Packed_R11F_G11F_B10F, Texture::Layout::RGB)),
            VUseFrustumCulling(Resources.AddVariable("UseFrustumCulling", true)),
            FBDepthAttachment(Resources.Get<Texture2D>("Scene Depth")),
            FrameBuffer(std::array{
                FrameBuffer::Attachment(Resources.Get<Texture2D>(GBufferAlbedo), FrameBuffer::ClearColor(0.0)),
                FrameBuffer::Attachment(Resources.Get<Texture2D>(GBufferNormal), FrameBuffer::ClearColor(0.0)),
                FrameBuffer::Attachment(Resources.Get<Texture2D>(GBufferTangent), FrameBuffer::ClearColor(0.0)),
                FrameBuffer::Attachment(Resources.Get<Texture2D>(GBufferProperties), FrameBuffer::ClearColor(0.0))
            }, &FBDepthAttachment),
            PipelineVariant(GLTFPipelineOptions),
            Pipelines(PipelineMatrixFromFile("GLTF Mesh to GBuffer", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "Nodes/MeshToGBuffer.glsl", GLTFPipelineOptions)),
            Sampler(Sampler::Params{})
        {
        }

    protected:
        void OnReloadShaders(CommandContext& Resources) override
        {
            PipelineMatrixUpdateFromFile(Pipelines, "Nodes/MeshToGBuffer.glsl");
        }
        
        void OnUpdate(CommandContext& Resources, double DeltaTime) override
        {
            if (Resources.HasChanged<Size2D>("Scene Radiance"))
            {
                GBufferSize = Resources.GetValue<Size2D>("Scene Radiance");
                Resources.Get<Texture2D>(GBufferAlbedo).Data(GBufferSize.x, GBufferSize.y);
                Resources.Get<Texture2D>(GBufferNormal).Data(GBufferSize.x, GBufferSize.y);
                Resources.Get<Texture2D>(GBufferTangent).Data(GBufferSize.x, GBufferSize.y);
                Resources.Get<Texture2D>(GBufferProperties).Data(GBufferSize.x, GBufferSize.y);
                FrameBuffer.Resize(GBufferSize.x, GBufferSize.y);
            }
        }
        
        void OnExecute(const CommandContext& Resources) override
        {
            Bind(FrameBuffer);
            FrameBuffer.Clear();
            
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_DEPTH_BUFFER_BIT);

            PipelineVariant.SetParameter(kParamMaterialParamMode, Resources.Scene().Extension & GLTF::GPUScene::MaterialsAsBuffers ? 1 : 0);
            const Pipeline& Pipeline = Pipelines.GetVariant(PipelineVariant);
            
            Bind(Pipeline);
            
            // States settings
            bool UseFrustumCulling = Resources.GetValue<Bool>(VUseFrustumCulling);
            
            // Uniform Data
            GLuint GLTFBaseColor, GLTFRoughness, GLTFMetalness, GLTFEmissiveColor, GLTFSpecularColor, GLTFMaterialFlags;
            GLuint  GLTFUseColorTexture, GLTFUseNormalTexture, GLTFUseMRTexture, GLTFUseAOTexture, GLTFUseSpecularTexture, GLTFUseEmissiveTexture;
            GLuint GLTFParamBuffer = -1;
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
            GLuint GLTFTexColor = GetUniformLocation(Pipeline, "texColor");
            GLuint GLTFTexNormal = GetUniformLocation(Pipeline, "texNormal");
            GLuint GLTFTexMR = GetUniformLocation(Pipeline, "texMR");
            GLuint GLTFTexAO = GetUniformLocation(Pipeline, "texAO");
            GLuint GLTFTexSpecular = GetUniformLocation(Pipeline, "texSpecular");
            GLuint GLTFTexEmissive = GetUniformLocation(Pipeline, "texEmissive");
            GLuint GLTFModelMatrix = GetUniformLocation(Pipeline, "Model");
            
            // Scene storage buffers
            SetUniform(0, Resources.GetCameraBuffer());
            
            // For now we only support GLTF materials
            // TODO generify material system and migrate to a scene mesh type
            for (const GLTF::MeshInstance& Instance : Resources.Scene().instances)
            {
                const MeshObject& Mesh = Resources.Scene().meshes[Instance.mesh];
                const Mesh::VertexGroup& Group = Mesh.GetGroups()[Instance.vertexGroup];
                const GLTF::Transform& Transform = Resources.Scene().transforms[Instance.transform];
                const GLTF::Material& Material = Resources.Scene().materials[Instance.material];
                const UniformBuffer* MaterialBuffer = Resources.Scene().Extension & GLTF::GPUScene::MaterialsAsBuffers ?
                    &Resources.Scene().materialUniformBuffers[Instance.material] :
                    nullptr;
                
                // Transform
                switch (Transform.Type)
                {
                case GLTF::Transform::Properties:
                    {
                        Math::Transform4f TransformMatrix = Transform.Value.asProperties.GetTransform();
                
                        if (UseFrustumCulling && !Rendering::frustumCullingTest(Resources.GetMainCameraData().Camera_WorldToProj(), TransformMatrix, Group.BoundsMin, Group.BoundsMax)) continue;
                
                        SetUniform(GLTFModelMatrix, TransformMatrix);
                    }
                    break;
                    
                case GLTF::Transform::Matrix:
                    {
                        if (UseFrustumCulling && !Rendering::frustumCullingTest(Resources.GetMainCameraData().Camera_WorldToProj(), Transform.Value.asMatrix, Group.BoundsMin, Group.BoundsMax)) continue;
                
                        SetUniform(GLTFModelMatrix, Transform.Value.asMatrix);
                    }
                    break;
                    
                SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
                }

                
                // Material
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
                    SetUniform(GLTFBaseColor, Material.color.xyz());
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
                
                if (Material.colorTexture != UINT64_MAX)                SetUniform(GLTFTexColor, 4, Resources.Scene().textures[Material.colorTexture], Sampler);
                if (Material.normalTexture != UINT64_MAX)               SetUniform(GLTFTexNormal, 5, Resources.Scene().textures[Material.normalTexture], Sampler);
                if (Material.metallicRoughnessTexture != UINT64_MAX)    SetUniform(GLTFTexMR, 6, Resources.Scene().textures[Material.metallicRoughnessTexture], Sampler);
                if (Material.occlusionTexture != UINT64_MAX)            SetUniform(GLTFTexAO, 7, Resources.Scene().textures[Material.occlusionTexture], Sampler);
                if (Material.specularTexture != UINT64_MAX)             SetUniform(GLTFTexSpecular, 7, Resources.Scene().textures[Material.specularTexture], Sampler);
                if (Material.emissiveTexture != UINT64_MAX)             SetUniform(GLTFTexEmissive, 7, Resources.Scene().textures[Material.emissiveTexture], Sampler);
                
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
            
            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
            
            UnBind(Pipeline);
            UnBind(FrameBuffer);
        }
    
    private:
        PipelineMatrix::MatrixDesc GLTFPipelineOptions = PipelineMatrix::MatrixDesc::Literal
        {
            {"GLTF_MATERIAL_PARAMS_MODE", {"GLTF_MATERIAL_PARAMS_UNIFORMS", "GLTF_MATERIAL_PARAMS_UNIFORM_BUFFER"}}
        };
        static constexpr PipelineMatrix::Name kParamMaterialParamMode = 0;
        
        Size2D GBufferSize;
        Location GBufferAlbedo;
        Location GBufferNormal;
        Location GBufferTangent;
        Location GBufferProperties;
        Location VUseFrustumCulling;
        FrameBuffer::DepthAttachment FBDepthAttachment;
        FrameBuffer FrameBuffer;
        PipelineMatrix::VariantDesc PipelineVariant;
        PipelineMatrix Pipelines;
        Sampler Sampler;
    };
}
