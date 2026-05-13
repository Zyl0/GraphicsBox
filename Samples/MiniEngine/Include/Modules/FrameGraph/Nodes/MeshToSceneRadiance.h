#pragma once

#include "Modules/FrameGraph/Commands.h"

namespace FrameGraph
{
    class MeshToSceneRadiance : public ICommand
    {
    public:
        MeshToSceneRadiance(CommandContext& Resources): 
            ICommand(Resources),
            VDirectionalLightDirection(Resources.GetLocation<Math::Vector3f>("Light Direction")),
            VDirectionalLightColor(Resources.GetLocation<Math::Vector3f>("Light Color")),
            VDirectionalLightIntensity(Resources.GetLocation<Float>("Light Intensity")),
            VIndirectLightSamples(Resources.AddVariable<UInt>("Indirect Sample Count", 32)),
            VSkylightMethod(Resources.GetLocation<UInt>("Skylight Method")),
            VUseFrustumCulling(Resources.AddVariable("UseFrustumCulling", true)),
            VUseMSAA(Resources.GetLocation<Bool>("Use MSAA")),
            VMSAASampleCount(Resources.GetLocation<UInt>("MSAA Sample Count")),
            Cubemap(Resources.GetLocation<TextureCube>("Cubemap Skylight")),
            HDRi(Resources.GetLocation<Texture2D>("HDRi Skylight")),
            FBDepthAttachment(Resources.Get<Texture2D>("Scene Depth")),
            FBDepthAttachmentMSAA(Resources.Get<Texture2D>("Scene Depth MSAA")),
            SceneRadianceFB(FrameBuffer::Attachment(Resources.Get<Texture2D>("Scene Radiance"), FrameBuffer::ClearColor(0.0)), &FBDepthAttachment),
            SceneRadianceMSAAFB(FrameBuffer::Attachment(Resources.Get<Texture2D>("Scene Radiance MSAA"), FrameBuffer::ClearColor(0.0)), &FBDepthAttachmentMSAA),
            CubemapPipeline(PipelineFromFile("Skylight To Radiance Cubemap", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "Nodes/MeshToRadiance.glsl", PipelineCubemapDefines)),
            HDRiPipeline(PipelineFromFile("Skylight To Radiance HDRi", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "Nodes/MeshToRadiance.glsl", PipelineHDRIDefines)),
            Sampler(Sampler::Params{})
        {
        }

        ~MeshToSceneRadiance() override = default;

    protected:
        void OnReloadShaders(CommandContext& Resources) override
        {
            PipelineUpdateFromFile(CubemapPipeline, "Nodes/MeshToRadiance.glsl", PipelineCubemapDefines);
            PipelineUpdateFromFile(HDRiPipeline, "Nodes/MeshToRadiance.glsl", PipelineHDRIDefines);
        }
        
        void OnUpdate(CommandContext& Resources, double DeltaTime) override
        {
            if (Resources.HasChanged<Size2D>("Scene Radiance"))
            {
                Size2D SceneRadianceSize = Resources.GetValue<Size2D>("Scene Radiance");
                SceneRadianceFB.Resize(SceneRadianceSize.x, SceneRadianceSize.y);
                SceneRadianceMSAAFB.Resize(SceneRadianceSize.x, SceneRadianceSize.y);
            }

            // if (Resources.HasChanged<Bool>(VUseMSAA)) // || Resources.HasChanged<UInt>(VMSAASampleCount)
            // {
            //     Bool UseMSAA = Resources.GetValue<Bool>(VUseMSAA);
            //     // UInt SampleCount = Resources.GetValue<UInt>(VMSAASampleCount);
            // 
            //     // SampleCount = UseMSAA ? SampleCount : 0;
            //     
            //     FBDepthAttachmentMSAA = FrameBuffer::DepthAttachment(Resources.Get<Texture2D>("Scene Depth MSAA"));
            //     if (UseMSAA)
            //     {
            //         SceneRadianceMSAAFB.Retarget(FrameBuffer::RetargetAttachment(Resources.Get<Texture2D>("Scene Radiance MSAA")), &FBDepthAttachmentMSAA);
            //         SceneRadianceFB.Retarget(FrameBuffer::RetargetAttachment(Resources.Get<Texture2D>("Scene Radiance")), nullptr);
            //     }
            //     else
            //     {
            //         SceneRadianceFB.Retarget(FrameBuffer::RetargetAttachment(Resources.Get<Texture2D>("Scene Radiance")), &FBDepthAttachment);
            //     }
            // }
        }
        
        void OnExecute(const CommandContext& Resources) override
        {
            Bool UseMSAA = Resources.GetValue<Bool>(VUseMSAA);
            if (UseMSAA)
            {
                Bind(SceneRadianceMSAAFB);
            }
            else
            {
                Bind(SceneRadianceFB);
            }
            
            const Pipeline* pipeline = nullptr;
            
            switch (Resources.GetValue<UInt>(VSkylightMethod))
            {
            case 0: // Cubemap Sampling
                Bind(CubemapPipeline);
                
                SetUniform(CubemapPipeline, "SkyLightCubeMap", 0, Resources.Get<TextureCube>(Cubemap), Sampler);
                SetUniform(CubemapPipeline, "SkyLightMipCount", Resources.Get<TextureCube>(Cubemap).MipCount());
                
                pipeline = &CubemapPipeline;
                break;
            
            case 1: // HDRI Sampling
                Bind(HDRiPipeline);
                
                SetUniform(HDRiPipeline, "SkyLightHDRi", 0, Resources.Get<Texture2D>(HDRi), Sampler);
                SetUniform(HDRiPipeline, "SkyLightMipCount", Resources.Get<Texture2D>(HDRi).MipCount());
                
                pipeline = &HDRiPipeline;
                break;
                        
            default:
                return;
            }
            
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_DEPTH_BUFFER_BIT); // Color clear is done when drawing the skylight
            
            // States settings
            bool UseFrustumCulling = Resources.GetValue<Bool>(VUseFrustumCulling);
            
            // Light properties
            SetUniform(*pipeline, "LightDirection", Resources.GetValue<Math::Vector3f>(VDirectionalLightDirection));
            SetUniform(*pipeline, "LightColor", Resources.GetValue<Math::Vector3f>(VDirectionalLightColor));
            SetUniform(*pipeline, "LightIntensity", Resources.GetValue<Float>(VDirectionalLightIntensity));
            SetUniform(*pipeline, "IndirectLightingSampleCount", Resources.GetValue<UInt>(VIndirectLightSamples));
            
            // Uniform Data
            GLint GLTFBaseColor = GetUniformLocation(*pipeline, "BaseColor");
            GLint GLTFRoughness = GetUniformLocation(*pipeline, "Roughness");
            GLint GLTFMetalness = GetUniformLocation(*pipeline, "Metalness");
            GLint GLTFUseColorTexture = GetUniformLocation(*pipeline, "UseColorTexture");
            GLint GLTFUseNormalTexture = GetUniformLocation(*pipeline, "UseNormalTexture");
            GLint GLTFUseMRTexture = GetUniformLocation(*pipeline, "UseMRTexture");
            GLint GLTFUseAOTexture = GetUniformLocation(*pipeline, "UseAOTexture");
            GLint GLTFTexColor = GetUniformLocation(*pipeline, "texColor");
            GLint GLTFTexNormal = GetUniformLocation(*pipeline, "texNormal");
            GLint GLTFTexMR = GetUniformLocation(*pipeline, "texMR");
            GLint GLTFTexAO = GetUniformLocation(*pipeline, "texAO");
            GLint GLTFModelMatrix = GetUniformLocation(*pipeline, "Model");
            
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
            
            
            UnBind(*pipeline);

            if (UseMSAA)
            {
                Size2D SceneRadianceSize = Resources.GetValue<Size2D>("Scene Radiance");

                Bind(SceneRadianceFB, SceneRadianceMSAAFB);
                    
                glBlitFramebuffer(
                    0, 0, SceneRadianceSize.x, SceneRadianceSize.y,
                    0, 0, SceneRadianceSize.x, SceneRadianceSize.y,
                    GL_COLOR_BUFFER_BIT, GL_NEAREST );
            }

            UnBind(SceneRadianceFB);
        }

    private:
        Location VDirectionalLightDirection;
        Location VDirectionalLightColor;
        Location VDirectionalLightIntensity;
        Location VIndirectLightSamples;
        Location VSkylightMethod;
        Location VUseFrustumCulling;
        Location VUseMSAA;
        Location VMSAASampleCount;
        Location Cubemap;
        Location HDRi;
        FrameBuffer::DepthAttachment FBDepthAttachment;
        FrameBuffer::DepthAttachment FBDepthAttachmentMSAA;
        FrameBuffer SceneRadianceFB;
        FrameBuffer SceneRadianceMSAAFB;
        Shader::DefineArray<1> PipelineCubemapDefines = {Shader::Define("USE_CUBEMAP_SKYLIGHT", "")};
        Shader::DefineArray<1> PipelineHDRIDefines = {Shader::Define("USE_HDRI_SKYLIGHT", "")};
        Pipeline CubemapPipeline;
        Pipeline HDRiPipeline;
        Sampler Sampler;
    };
}