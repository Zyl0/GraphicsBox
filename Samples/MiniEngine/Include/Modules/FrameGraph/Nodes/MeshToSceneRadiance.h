#pragma once

#include "Modules/FrameGraph/Commands.h"
#include "Modules/FrameGraph/Helpers/DrawCommands.h"
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
            VUseMotionVectors(Resources.GetLocation<UInt>("Use Motion Vectors")),
            VUsePreviousMotionVectors(Resources.GetLocation<UInt>("Use Previous Motion Vectors")),
            PrevSceneRadiance(Resources.GetLocation<Texture2D>("Previous Scene Radiance")),
            PrevSceneDepth(Resources.GetLocation<Texture2D>("Previous Scene Depth")),
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
            PipelineVariant.SetParameter(kParamMaterialParamMode, Resources.Scene().Extension & Scene::GPUScene::MaterialsAsBuffers ? 1 : 0);
            
            glEnable(GL_DEPTH_TEST);
            glClear(GL_DEPTH_BUFFER_BIT); // Color clear is done when drawing the skylight
            
            
            // Draw scene
            DrawGLTFSettings settings{};
            settings.MaterialSampler = &MaterialSampler;
            settings.CurrentCameraBufferBinding = 0;
            settings.PreviousCameraBufferBinding = 1;
            settings.GLTFMaterialBufferBinding = 2;
            settings.FrustumCulling = Resources.GetValue<Bool>(VUseFrustumCulling);
            settings.TextureUnitOffset = 4;
            {
                DebugScopeMarker opaqueScope("Draw Opaque");
                
                glEnable(GL_CULL_FACE);
                
                PipelineVariant.SetParameter(kParamMaterialBlendMode, 0);
                const Pipeline& pipeline = Pipelines.GetVariant(PipelineVariant);
                
                settings.MaterialFlagsRequireFilter = GLTF::Material::None;
                settings.MaterialFlagsExcludeFilter = GLTF::Material::Masked | GLTF::Material::Transparent | GLTF::Material::TwoSided;
                DrawSceneWithFilters(Resources, pipeline, settings);
            }
            {
                DebugScopeMarker opaqueScope("Draw Masked");
                
                PipelineVariant.SetParameter(kParamMaterialBlendMode, 1);
                const Pipeline& maskedPipeline = Pipelines.GetVariant(PipelineVariant);
                
                settings.MaterialFlagsRequireFilter = GLTF::Material::Masked;
                settings.MaterialFlagsExcludeFilter = GLTF::Material::TwoSided;
                DrawSceneWithFilters(Resources, maskedPipeline, settings);
            }
            {
                DebugScopeMarker opaqueScope("Draw Opaque - Two Sided");
                
                glEnable(GL_CULL_FACE);
                
                PipelineVariant.SetParameter(kParamMaterialBlendMode, 0);
                const Pipeline& pipeline = Pipelines.GetVariant(PipelineVariant);
                
                settings.MaterialFlagsRequireFilter = GLTF::Material::TwoSided;
                settings.MaterialFlagsExcludeFilter = GLTF::Material::Masked | GLTF::Material::Transparent;
                DrawSceneWithFilters(Resources, pipeline, settings);
            }
            {
                DebugScopeMarker opaqueScope("Draw Masked - Two Sided");
                
                PipelineVariant.SetParameter(kParamMaterialBlendMode, 1);
                const Pipeline& maskedPipeline = Pipelines.GetVariant(PipelineVariant);

                settings.MaterialFlagsRequireFilter = GLTF::Material::Masked | GLTF::Material::TwoSided;
                settings.MaterialFlagsExcludeFilter = GLTF::Material::None;
                DrawSceneWithFilters(Resources, maskedPipeline, settings);
            }
            {
                DebugScopeMarker opaqueScope("Draw Transparent");
                
                // Transparent blend setup
                glEnable(GL_BLEND);
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
                glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
                
                PipelineVariant.SetParameter(kParamMaterialBlendMode, 2);
                const Pipeline& transparentPipeline = Pipelines.GetVariant(PipelineVariant);
                
                settings.MaterialFlagsRequireFilter = GLTF::Material::Transparent;
                settings.MaterialFlagsExcludeFilter = GLTF::Material::None;
                DrawSceneWithFilters(Resources, transparentPipeline, settings);
                
                glDisable(GL_BLEND);
            }
            
            glDisable(GL_DEPTH_TEST);

            // Resolve MSAA
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
        
        void DrawSceneWithFilters(const CommandContext& Resources, const Pipeline& pipeline, const DrawGLTFSettings& settings) const
        {
            Bind(pipeline);
            
            // Light properties
            SetUniform(pipeline, "LightDirection", Resources.GetValue<Math::Vector3f>(VDirectionalLightDirection));
            SetUniform(pipeline, "LightColor", Resources.GetValue<Math::Vector3f>(VDirectionalLightColor));
            SetUniform(pipeline, "LightIntensity", Resources.GetValue<Float>(VDirectionalLightIntensity));
            SetUniform(pipeline, "IndirectLightingSampleCount", Resources.GetValue<UInt>(VIndirectLightSamples));
            
            // Ambient light properties
            switch (Resources.GetValue<UInt>(VSkylightMethod))
            {
            case 0: // Cubemap Sampling           
                // Scene texture buffers
                SetUniform(pipeline, "SkyLightCubeMap", 1, Resources.Get<TextureCube>(Cubemap), MaterialSampler);
                // SetUniform(pipeline, "SkyLightMipCount", Resources.Get<TextureCube>(Cubemap).MipCount());
                break;
            
            case 1: // HDRI Sampling
                // Scene texture buffers
                SetUniform(pipeline, "SkyLightHDRi", 1, Resources.Get<Texture2D>(HDRi), MaterialSampler);
                // SetUniform(pipeline, "SkyLightMipCount", Resources.Get<Texture2D>(HDRi).MipCount());
                break;
                        
            default:
                return;
            }

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
            
            // Material buffer
            AssertOrWarnCallF(RequestBufferBinding(pipeline, "ParamBuffer", settings.GLTFMaterialBufferBinding) >= 0,, "Failed to request GLTF material Uniform Binding");
            
            DrawGLTFScene(Resources, pipeline, Resources.GetMainCamera(), settings);
            
            UnBind(pipeline);
        }

    private:
    
        PipelineMatrix::MatrixDesc GLTFPipelineOptions = PipelineMatrix::MatrixDesc::Literal
        {
            {"SKYLIGHT_METHOD", {"SKYLIGHT_CUBE_MAP", "SKYLIGHT_HDRI"}},
            {"GLTF_MATERIAL_PARAMS_MODE", {"GLTF_MATERIAL_PARAMS_UNIFORMS", "GLTF_MATERIAL_PARAMS_UNIFORM_BUFFER"}},
            {"GLTF_MATERIAL_OPACITY", {"GLTF_MATERIAL_OPAQUE", "GLTF_MATERIAL_MASKED", "GLTF_MATERIAL_TRANSPARENT"}},
        };
        static constexpr PipelineMatrix::Name kParamSkylight = 0;
        static constexpr PipelineMatrix::Name kParamMaterialParamMode = 1;
        static constexpr PipelineMatrix::Name kParamMaterialBlendMode = 2;
        
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
