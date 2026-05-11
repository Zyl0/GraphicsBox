#pragma once

#include "Importers/GLTF/SceneLoader.h"
#include "Modules/Rendering/Tools/Commands.h"
#include "Modules/Rendering/Tools/FrustumCulling.h"
#include "Rendering/FrameBuffers.h"
#include "Rendering/Pipelines.h"
#include "Rendering/Sampler.h"
#include "Rendering/Uniforms.h"

namespace Rendering::Graph
{
    class MeshToGBuffer : public Command
    {
    public:
        MeshToGBuffer(CommandContext& Resources): 
            Command(Resources),
            GBufferSize(Resources.GetValue<Size2D>("Scene Radiance")),
            GBufferAlbedo(Resources.Add<Texture2D>("GBufferAlbedo", GBufferSize.x, GBufferSize.y, Texture::Type::Packed_R11F_G11F_B10F, Texture::Layout::RGB)),
            GBufferNormal(Resources.Add<Texture2D>("GBufferNormal", GBufferSize.x, GBufferSize.y, Texture::Type::Half, Texture::Layout::RGB)),
            GBufferProperties(Resources.Add<Texture2D>("GBufferProperties", GBufferSize.x, GBufferSize.y, Texture::Type::Packed_R11F_G11F_B10F, Texture::Layout::RGB)),
            VUseFrustumCulling(Resources.AddVariable("UseFrustumCulling", true)),
            FBDepthAttachment(Resources.Get<Texture2D>("Scene Depth")),
            FrameBuffer(std::array{
                FrameBuffer::Attachment(Resources.Get<Texture2D>(GBufferAlbedo), FrameBuffer::ClearColor(0.0)),
                FrameBuffer::Attachment(Resources.Get<Texture2D>(GBufferNormal), FrameBuffer::ClearColor(0.0)),
                FrameBuffer::Attachment(Resources.Get<Texture2D>(GBufferProperties), FrameBuffer::ClearColor(0.0))
            }, &FBDepthAttachment),
            Pipeline(PipelineFromFile("GLTF Mesh to GBuffer", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "Nodes/MeshToGBuffer.glsl")),
            Sampler(Sampler::Params{})
        {
        }

    protected:
        void OnReloadShaders(CommandContext& Resources) override
        {
            PipelineUpdateFromFile(Pipeline, "Nodes/MeshToGBuffer.glsl");
        }
        
        void OnUpdate(CommandContext& Resources, double DeltaTime) override
        {
            if (Resources.HasChanged<Size2D>("Scene Radiance"))
            {
                GBufferSize = Resources.GetValue<Size2D>("Scene Radiance");
                Resources.Get<Texture2D>(GBufferAlbedo).Data(GBufferSize.x, GBufferSize.y);
                Resources.Get<Texture2D>(GBufferNormal).Data(GBufferSize.x, GBufferSize.y);
                Resources.Get<Texture2D>(GBufferProperties).Data(GBufferSize.x, GBufferSize.y);
                FrameBuffer.Resize(GBufferSize.x, GBufferSize.y);
            }
        }
        
        void OnExecute(const CommandContext& Resources) override
        {
            Bind(FrameBuffer);
            
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_DEPTH_BUFFER_BIT); // Color clear is done when drawing the skylight
            
            Bind(Pipeline);
            
            // States settings
            bool UseFrustumCulling = Resources.GetValue<Bool>(VUseFrustumCulling);
            
            // Uniform Data
            GLint GLTFBaseColor = GetUniformLocation(Pipeline, "BaseColor");
            GLint GLTFRoughness = GetUniformLocation(Pipeline, "Roughness");
            GLint GLTFMetalness = GetUniformLocation(Pipeline, "Metalness");
            GLint GLTFUseColorTexture = GetUniformLocation(Pipeline, "UseColorTexture");
            GLint GLTFUseNormalTexture = GetUniformLocation(Pipeline, "UseNormalTexture");
            GLint GLTFUseMRTexture = GetUniformLocation(Pipeline, "UseMRTexture");
            GLint GLTFUseAOTexture = GetUniformLocation(Pipeline, "UseAOTexture");
            GLint GLTFTexColor = GetUniformLocation(Pipeline, "texColor");
            GLint GLTFTexNormal = GetUniformLocation(Pipeline, "texNormal");
            GLint GLTFTexMR = GetUniformLocation(Pipeline, "texMR");
            GLint GLTFTexAO = GetUniformLocation(Pipeline, "texAO");
            GLint GLTFModelMatrix = GetUniformLocation(Pipeline, "Model");
            
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
                
                // Transform
                switch (Transform.Type)
                {
                case GLTF::Transform::Properties:
                    {
                        Math::Transform4f TransformMatrix = Transform.Value.asProperties.GetTransform();
                
                        if (UseFrustumCulling && !frustumCullingTest(Resources.GetMainCameraData().Camera_WorldToProj(), TransformMatrix, Group.BoundsMin, Group.BoundsMax)) continue;
                
                        SetUniform(GLTFModelMatrix, TransformMatrix);
                    }
                    break;
                    
                case GLTF::Transform::Matrix:
                    {
                        if (UseFrustumCulling && !frustumCullingTest(Resources.GetMainCameraData().Camera_WorldToProj(), Transform.Value.asMatrix, Group.BoundsMin, Group.BoundsMax)) continue;
                
                        SetUniform(GLTFModelMatrix, Transform.Value.asMatrix);
                    }
                    break;
                    
                SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
                }

                
                // Material
                SetUniform(GLTFBaseColor, Material.color.XYZ());
                SetUniform(GLTFRoughness, Material.roughness);
                SetUniform(GLTFMetalness, Material.metallic);
                SetUniform(GLTFUseColorTexture, Material.colorTexture != UINT64_MAX);
                SetUniform(GLTFUseNormalTexture, Material.normalTexture != UINT64_MAX);
                SetUniform(GLTFUseMRTexture, Material.metallicRoughnessTexture != UINT64_MAX);
                SetUniform(GLTFUseAOTexture, Material.occlusionTexture != UINT64_MAX);
                
                if (Material.colorTexture != UINT64_MAX)                SetUniform(GLTFTexColor, 2, Resources.Scene().textures[Material.colorTexture], Sampler);
                if (Material.normalTexture != UINT64_MAX)               SetUniform(GLTFTexNormal, 3, Resources.Scene().textures[Material.normalTexture], Sampler);
                if (Material.metallicRoughnessTexture != UINT64_MAX)    SetUniform(GLTFTexMR, 4, Resources.Scene().textures[Material.metallicRoughnessTexture], Sampler);
                if (Material.occlusionTexture != UINT64_MAX)            SetUniform(GLTFTexAO, 5, Resources.Scene().textures[Material.occlusionTexture], Sampler);
                
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
        Size2D GBufferSize;
        Location GBufferAlbedo;
        Location GBufferNormal;
        Location GBufferProperties;
        Location VUseFrustumCulling;
        FrameBuffer::DepthAttachment FBDepthAttachment;
        FrameBuffer FrameBuffer;
        Pipeline Pipeline;
        Sampler Sampler;
    };
}
