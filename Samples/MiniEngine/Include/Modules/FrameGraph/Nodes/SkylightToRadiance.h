#pragma once

#include "Modules/FrameGraph/Commands.h"

#include "Rendering/FrameBuffers.h"
#include "Rendering/Pipelines.h"
#include "Rendering/Sampler.h"
#include "Rendering/Uniforms.h"

namespace FrameGraph
{
    class SkylightToRadiance : public ICommand
    {
    public:
        SkylightToRadiance(CommandContext& Resources): 
            ICommand(Resources),
            VEmptyVAO(Resources.GetLocation<VertexArrayObject>("Empty VAO")),
            VSkylightMethod(Resources.AddVariable<UInt>("Skylight Method", /* HDRi */ 1)),
            VUseMSAA(Resources.GetLocation<Bool>("Use MSAA")),
            VMSAASampleCount(Resources.GetLocation<UInt>("MSAA Sample Count")),
            Cubemap(Resources.GetLocation<TextureCube>("Cubemap Skylight")),
            HDRi(Resources.GetLocation<Texture2D>("HDRi Skylight")),
            SceneRadianceFB(FrameBuffer::Attachment(Resources.Get<Texture2D>("Scene Radiance"), FrameBuffer::ClearColor(0.0f))),
            SceneRadianceMSAAFB(FrameBuffer::Attachment(Resources.Get<Texture2D>("Scene Radiance MSAA"), FrameBuffer::ClearColor(0.0))),
            CubemapPipeline(PipelineFromFile("Skylight To Radiance Cubemap", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "Nodes/SkylightToRadiance.glsl", PipelineCubemapDefines)),
            HDRiPipeline(PipelineFromFile("Skylight To Radiance HDRi", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "Nodes/SkylightToRadiance.glsl", PipelineHDRIDefines)),
            Sampler({})
        {}
        ~SkylightToRadiance() override = default;

    protected:
        void OnReloadShaders(CommandContext& Resources) override
        {
            PipelineUpdateFromFile(CubemapPipeline, "Nodes/SkylightToRadiance.glsl", PipelineCubemapDefines);
            PipelineUpdateFromFile(HDRiPipeline, "Nodes/SkylightToRadiance.glsl", PipelineHDRIDefines);
        }
        
        void OnUpdate(CommandContext& Resources, double DeltaTime) override
        {
            if (Resources.HasChanged<Size2D>("Scene Radiance"))
            {
                Size2D size = Resources.GetValue<Size2D>("Scene Radiance");
                SceneRadianceFB.Resize(size.x, size.y);
                SceneRadianceMSAAFB.Resize(size.x, size.y);
            }

            // if (Resources.HasChanged<Bool>(VUseMSAA)) // || Resources.HasChanged<UInt>(VMSAASampleCount)
            // {
            //     Bool UseMSAA = Resources.GetValue<Bool>(VUseMSAA);
            //     // UInt SampleCount = Resources.GetValue<UInt>(VMSAASampleCount);
            // 
            //     // SampleCount = UseMSAA ? SampleCount : 0;
            // 
            //     if (UseMSAA)
            //     {
            //         SceneRadianceMSAAFB.Retarget(FrameBuffer::RetargetAttachment(Resources.Get<Texture2D>("Scene Radiance MSAA")));
            //     }
            //     else
            //     {
            //         SceneRadianceFB.Retarget(FrameBuffer::RetargetAttachment(Resources.Get<Texture2D>("Scene Radiance")));
            //     }
            // }
        }
        
        void OnExecute(const CommandContext& Resources) override
        {
            Bool UseMSAA = Resources.GetValue<Bool>(VUseMSAA);
            if (UseMSAA)
            {
                Bind(SceneRadianceMSAAFB);
                SceneRadianceMSAAFB.Clear();
            }
            else
            {
                Bind(SceneRadianceFB);
                SceneRadianceFB.Clear();
            }
            
            const Pipeline* pipeline = nullptr;
            
            switch (Resources.GetValue<UInt>(VSkylightMethod))
            {
            case 0: // Cubemap Sampling
                Bind(CubemapPipeline);
                
                // Scene texture buffers
                SetUniform(CubemapPipeline, "SkyLightCubeMap", 0, Resources.Get<TextureCube>(Cubemap), Sampler);
            
                pipeline = &CubemapPipeline;
                break;
            
            case 1: // HDRI Sampling
                Bind(HDRiPipeline);
                
                // Scene texture buffers
                SetUniform(HDRiPipeline, "SkyLightHDRi", 0, Resources.Get<Texture2D>(HDRi), Sampler);
            
                pipeline = &HDRiPipeline;
                break;
                        
            default:
                return;
            }

            Bind(Resources.Get<VertexArrayObject>(VEmptyVAO));
            
            // Scene storage buffers
            SetUniform(0, Resources.GetCameraBuffer());
            
            // Draw screen quad
            glDrawArrays(GL_TRIANGLES, 0, 3);
            
            UnBind(*pipeline);

            if (UseMSAA)
            {
                UnBind(SceneRadianceMSAAFB);
            }
            else
            {
                UnBind(SceneRadianceFB);
            }
        }
    
    private:
        Location VEmptyVAO;
        Location VSkylightMethod;
        Location VUseMSAA;
        Location VMSAASampleCount;
        Location Cubemap;
        Location HDRi;
        FrameBuffer SceneRadianceFB;
        FrameBuffer SceneRadianceMSAAFB;
        Shader::DefineArray<1> PipelineCubemapDefines = {Shader::Define("USE_CUBEMAP_SKYLIGHT", "")};
        Shader::DefineArray<1> PipelineHDRIDefines = {Shader::Define("USE_HDRI_SKYLIGHT", "")};
        Pipeline CubemapPipeline;
        Pipeline HDRiPipeline;
        
        Sampler Sampler;
    };
}
