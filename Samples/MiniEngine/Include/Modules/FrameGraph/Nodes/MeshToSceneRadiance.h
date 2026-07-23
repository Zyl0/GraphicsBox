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
            VUseScreenSpaceReflections(Resources.AddVariable<Bool>("Use Screen Space Reflections", false)),
            VUsePreviousRadiance(Resources.GetLocation<UInt>("Use Previous Radiance")),
            PrevSceneRadiance(Resources.GetLocation<Texture2D>("Previous Scene Radiance")),
            PrevSceneDepth(Resources.GetLocation<Texture2D>("Previous Scene Depth")),
            VUseMotionVectors(Resources.GetLocation<UInt>("Use Motion Vectors")),
            VUsePreviousMotionVectors(Resources.GetLocation<UInt>("Use Previous Motion Vectors")),
            VUseSSAO(Resources.AddVariable<Bool>("Use Screen Space AO", false)),
            Cubemap(Resources.GetLocation<TextureCube>("Cubemap Skylight")),
            HDRi(Resources.GetLocation<Texture2D>("HDRi Skylight")),
            FBDepthAttachment(Resources.Get<Texture2D>("Scene Depth")),
            FBDepthAttachmentMSAA(Resources.Get<Texture2D>("Scene Depth MSAA")),
            SceneRadianceFB(std::array{
                FrameBuffer::Attachment(Resources.Get<Texture2D>("Scene Radiance"), FrameBuffer::ClearColor(0.0)),
                FrameBuffer::Attachment(Resources.Get<Texture2D>("Motion Vectors"), FrameBuffer::ClearColor(0.0))
            }, &FBDepthAttachment),
            SceneRadianceMSAAFB(std::array{
                FrameBuffer::Attachment(Resources.Get<Texture2D>("Scene Radiance MSAA"), FrameBuffer::ClearColor(0.0)),
                FrameBuffer::Attachment(Resources.Get<Texture2D>("Motion Vectors MSAA"), FrameBuffer::ClearColor(0.0)),
            }, &FBDepthAttachmentMSAA),
            CubemapPipeline(PipelineFromFile("Mesh To Radiance Cubemap", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "Nodes/MeshToRadiance.glsl", PipelineCubemapDefines)),
            HDRiPipeline(PipelineFromFile("Mesh To Radiance HDRi", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "Nodes/MeshToRadiance.glsl", PipelineHDRIDefines)),
            MaterialSampler(Sampler::Params{}),
            PreviousFrameSamplerDepth(Sampler::Params{
                .Magnification = Sampler::F_Nearest,
                .Minification = Sampler::F_Nearest,
                .WarpModeU = Sampler::W_ClampToEdge,
                .WarpModeV = Sampler::W_ClampToEdge,
                .WarpModeW = Sampler::W_ClampToEdge,
                .MipMode = Sampler::M_NoMip,
            }),
            PreviousFrameSamplerColor(Sampler::Params{
                .Magnification = Sampler::F_Linear,
                .Minification = Sampler::F_Linear,
                .WarpModeU = Sampler::W_ClampToEdge,
                .WarpModeV = Sampler::W_ClampToEdge,
                .WarpModeW = Sampler::W_ClampToEdge,
                .MipMode = Sampler::M_Linear,
            })
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

            if (Resources.HasChanged<Bool>(VUseScreenSpaceReflections))
            {
                if (Resources.GetValue<bool>(VUseScreenSpaceReflections))
                {
                    Resources.SetValue<UInt>(VUsePreviousRadiance, Resources.GetValue<UInt>(VUsePreviousRadiance) + 1);
                }
                else
                {
                    Resources.SetValue<UInt>(VUsePreviousRadiance, Resources.GetValue<UInt>(VUsePreviousRadiance) - 1);
                }
            }
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
                
                SetUniform(CubemapPipeline, "SkyLightCubeMap", 0, Resources.Get<TextureCube>(Cubemap), MaterialSampler);
                SetUniform(CubemapPipeline, "SkyLightMipCount", Resources.Get<TextureCube>(Cubemap).MipCount());
                
                pipeline = &CubemapPipeline;
                break;
            
            case 1: // HDRI Sampling
                Bind(HDRiPipeline);
                
                SetUniform(HDRiPipeline, "SkyLightHDRi", 0, Resources.Get<Texture2D>(HDRi), MaterialSampler);
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

            // Screen space effects
            if (Resources.GetValue<Bool>(VUseScreenSpaceReflections) || Resources.GetValue<Bool>(VUseSSAO))
            {
                SetUniform(*pipeline, "texPreviousDepth", 3, Resources.Get<Texture2D>(PrevSceneDepth), PreviousFrameSamplerDepth);
                SetUniform(*pipeline, "ViewportSize", Resources.GetValue<Size2D>("Scene Radiance"));
            }

            // Screen space reflections
            if (Resources.GetValue<Bool>(VUseScreenSpaceReflections))
            {
                SetUniform(*pipeline, "SSRMode", 1u);
                SetUniform(*pipeline, "texPreviousRadiance", 2, Resources.Get<Texture2D>(PrevSceneRadiance), PreviousFrameSamplerColor);
                SetUniform(*pipeline, "PreviousRadianceMips", static_cast<float>(Resources.Get<Texture2D>(PrevSceneRadiance).MipCount()));
            }
            else
            {
                SetUniform(*pipeline, "SSRMode", 0u);
            }

            // Screen space AO
            if (Resources.GetValue<Bool>(VUseSSAO))
            {
                // SetUniform(*pipeline, "SSAOMode", 1u);
            }
            else
            {
                // SetUniform(*pipeline, "SSAOMode", 0u);
            }

            // Motion vectors
            SetUniform(*pipeline, "WriteMotionVectors", Resources.GetValue<UInt>(VUseMotionVectors));
            
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
            if (Resources.GetPreviousCamerasBuffer())
            {
                SetUniform(1, *Resources.GetPreviousCamerasBuffer());
            }
            
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
                
                if (Material.colorTexture != UINT64_MAX)                SetUniform(GLTFTexColor, 4, Resources.Scene().textures[Material.colorTexture], MaterialSampler);
                if (Material.normalTexture != UINT64_MAX)               SetUniform(GLTFTexNormal, 5, Resources.Scene().textures[Material.normalTexture], MaterialSampler);
                if (Material.metallicRoughnessTexture != UINT64_MAX)    SetUniform(GLTFTexMR, 6, Resources.Scene().textures[Material.metallicRoughnessTexture], MaterialSampler);
                if (Material.occlusionTexture != UINT64_MAX)            SetUniform(GLTFTexAO, 7, Resources.Scene().textures[Material.occlusionTexture], MaterialSampler);
                
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
        Location VUseScreenSpaceReflections;
        Location VUsePreviousRadiance;
        Location VUseMotionVectors;
        Location VUsePreviousMotionVectors;
        Location PrevSceneRadiance;
        Location PrevSceneDepth;
        Location VUseSSAO;
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
        Sampler MaterialSampler;
        Sampler PreviousFrameSamplerDepth;
        Sampler PreviousFrameSamplerColor;
    };
}