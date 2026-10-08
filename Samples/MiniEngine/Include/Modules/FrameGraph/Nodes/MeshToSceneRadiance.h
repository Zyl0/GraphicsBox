#pragma once

#include "Modules/FrameGraph/Commands.h"
#include "Rendering/Pipelines.h"

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
            Pipelines(PipelineMatrixFromFile("Mesh To Radiance", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "Nodes/MeshToRadiance.glsl", GLTFPipelineOptions)),
            PipelineVariant(GLTFPipelineOptions),
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
            PipelineMatrixUpdateFromFile(Pipelines, "Nodes/MeshToRadiance.glsl");
        }
        
        void OnUpdate(CommandContext& Resources, double DeltaTime) override
        {
            if (Resources.HasChanged<Size2D>("Scene Radiance"))
            {
                Size2D SceneRadianceSize = Resources.GetValue<Size2D>("Scene Radiance");
                SceneRadianceFB.Resize(SceneRadianceSize.x, SceneRadianceSize.y);
                SceneRadianceMSAAFB.Resize(SceneRadianceSize.x, SceneRadianceSize.y);
            }

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
            

            PipelineVariant.SetParameter(kParamSkylight, Resources.GetValue<UInt>(VSkylightMethod));
            PipelineVariant.SetParameter(kParamMaterialParamMode, Resources.Scene().Extension & GLTF::GPUScene::MaterialsAsBuffers ? 1 : 0);
            const Pipeline& pipeline = Pipelines.GetVariant(PipelineVariant);

            Bind(pipeline);
            
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_DEPTH_BUFFER_BIT); // Color clear is done when drawing the skylight
            
            // States settings
            bool UseFrustumCulling = Resources.GetValue<Bool>(VUseFrustumCulling);
            
            // Light properties
            SetUniform(pipeline, "LightDirection", Resources.GetValue<Math::Vector3f>(VDirectionalLightDirection));
            SetUniform(pipeline, "LightColor", Resources.GetValue<Math::Vector3f>(VDirectionalLightColor));
            SetUniform(pipeline, "LightIntensity", Resources.GetValue<Float>(VDirectionalLightIntensity));
            SetUniform(pipeline, "IndirectLightingSampleCount", Resources.GetValue<UInt>(VIndirectLightSamples));

            // Screen space effects
            if (Resources.GetValue<Bool>(VUseScreenSpaceReflections) || Resources.GetValue<Bool>(VUseSSAO))
            {
                SetUniform(pipeline, "texPreviousDepth", 3, Resources.Get<Texture2D>(PrevSceneDepth), PreviousFrameSamplerDepth);
                SetUniform(pipeline, "ViewportSize", Resources.GetValue<Size2D>("Scene Radiance"));
            }

            // Screen space reflections
            if (Resources.GetValue<Bool>(VUseScreenSpaceReflections))
            {
                SetUniform(pipeline, "SSRMode", 1u);
                SetUniform(pipeline, "texPreviousRadiance", 2, Resources.Get<Texture2D>(PrevSceneRadiance), PreviousFrameSamplerColor);
                SetUniform(pipeline, "PreviousRadianceMips", static_cast<float>(Resources.Get<Texture2D>(PrevSceneRadiance).MipCount()));
            }
            else
            {
                SetUniform(pipeline, "SSRMode", 0u);
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
            SetUniform(pipeline, "WriteMotionVectors", Resources.GetValue<UInt>(VUseMotionVectors));
            
            // Scene storage buffers
            SetUniform(0, Resources.GetCameraBuffer());
            if (Resources.GetPreviousCamerasBuffer())
            {
                SetUniform(1, *Resources.GetPreviousCamerasBuffer());
            }

            // Uniform Data
            GLuint GLTFBaseColor, GLTFRoughness, GLTFMetalness, GLTFEmissiveColor, GLTFSpecularColor, GLTFMaterialFlags;
            GLuint  GLTFUseColorTexture, GLTFUseNormalTexture, GLTFUseMRTexture, GLTFUseAOTexture, GLTFUseSpecularTexture, GLTFUseEmissiveTexture;
            GLuint GLTFParamBuffer = -1;
            if (Resources.Scene().Extension & GLTF::GPUScene::MaterialsAsBuffers)
            {
                GLTFParamBuffer = GetUniformLocation(pipeline, "ParamBuffer");
            }
            else if (Resources.Scene().Extension & GLTF::GPUScene::MaterialsAsUnifiedBuffer)
            {
                AssertOrError(false, "Feature not yet supported")
            }
            else
            {
                GLTFBaseColor = GetUniformLocation(pipeline, "BaseColor");
                GLTFEmissiveColor = GetUniformLocation(pipeline, "EmissiveColor");
                GLTFSpecularColor = GetUniformLocation(pipeline, "SpecularColor");
                GLTFRoughness = GetUniformLocation(pipeline, "Roughness");
                GLTFMetalness = GetUniformLocation(pipeline, "Metalness");
                GLTFMaterialFlags = GetUniformLocation(pipeline, "MaterialFlags");
                
                GLTFUseColorTexture = GetUniformLocation(pipeline, "UseColorTexture");
                GLTFUseNormalTexture = GetUniformLocation(pipeline, "UseNormalTexture");
                GLTFUseMRTexture = GetUniformLocation(pipeline, "UseMRTexture");
                GLTFUseAOTexture = GetUniformLocation(pipeline, "UseAOTexture");
                GLTFUseSpecularTexture = GetUniformLocation(pipeline, "UseSpecularTexture");
                GLTFUseEmissiveTexture = GetUniformLocation(pipeline, "UseEmissiveTexture");
            }
            GLuint GLTFTexColor = GetUniformLocation(pipeline, "texColor");
            GLuint GLTFTexNormal = GetUniformLocation(pipeline, "texNormal");
            GLuint GLTFTexMR = GetUniformLocation(pipeline, "texMR");
            GLuint GLTFTexAO = GetUniformLocation(pipeline, "texAO");
            GLuint GLTFTexSpecular = GetUniformLocation(pipeline, "texSpecular");
            GLuint GLTFTexEmissive = GetUniformLocation(pipeline, "texEmissive");
            GLuint GLTFModelMatrix = GetUniformLocation(pipeline, "Model");
            
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
                
                if (Material.colorTexture != UINT64_MAX)                SetUniform(GLTFTexColor, 4, Resources.Scene().textures[Material.colorTexture], MaterialSampler);
                if (Material.normalTexture != UINT64_MAX)               SetUniform(GLTFTexNormal, 5, Resources.Scene().textures[Material.normalTexture], MaterialSampler);
                if (Material.metallicRoughnessTexture != UINT64_MAX)    SetUniform(GLTFTexMR, 6, Resources.Scene().textures[Material.metallicRoughnessTexture], MaterialSampler);
                if (Material.occlusionTexture != UINT64_MAX)            SetUniform(GLTFTexAO, 7, Resources.Scene().textures[Material.occlusionTexture], MaterialSampler);
                if (Material.specularTexture != UINT64_MAX)             SetUniform(GLTFTexSpecular, 7, Resources.Scene().textures[Material.specularTexture], MaterialSampler);
                if (Material.emissiveTexture != UINT64_MAX)             SetUniform(GLTFTexEmissive, 7, Resources.Scene().textures[Material.emissiveTexture], MaterialSampler);
                
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
            
            
            UnBind(pipeline);

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
    
        PipelineMatrix::MatrixDesc GLTFPipelineOptions = PipelineMatrix::MatrixDesc::Literal
        {
            {"SKYLIGHT_METHOD", {"SKYLIGHT_CUBE_MAP", "SKYLIGHT_HDRI"}},
            {"GLTF_MATERIAL_PARAMS_MODE", {"GLTF_MATERIAL_PARAMS_UNIFORMS", "GLTF_MATERIAL_PARAMS_UNIFORM_BUFFER"}}
        };
        static constexpr PipelineMatrix::Name kParamSkylight = 0;
        static constexpr PipelineMatrix::Name kParamMaterialParamMode = 1;
        
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
        PipelineMatrix Pipelines;
        PipelineMatrix::VariantDesc PipelineVariant;
        Sampler MaterialSampler;
        Sampler PreviousFrameSamplerDepth;
        Sampler PreviousFrameSamplerColor;
    };
}
