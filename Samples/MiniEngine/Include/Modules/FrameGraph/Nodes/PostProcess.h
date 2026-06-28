#pragma once

#include "Modules/FrameGraph/Commands.h"

#include "Rendering/FrameBuffers.h"
#include "Rendering/Pipelines.h"
#include "Rendering/Uniforms.h"

namespace FrameGraph
{
    class ToneMappingCommand : public ICommand
    {
    public:
        ToneMappingCommand(CommandContext& Resources): 
            ICommand(Resources),
            VEmptyVAO(Resources.GetLocation<VertexArrayObject>("Empty VAO")),
            FrameBuffer(FrameBuffer::Attachment(Resources.Get<Texture2D>("Output"), FrameBuffer::ClearColor(0.0f))),
            Pipeline(PipelineFromFile("Post Process", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "Nodes/PostProcess.glsl")),
            Sampler({
                .Magnification = Sampler::F_Nearest,
                .Minification = Sampler::F_Nearest,
            })
        {}
        ~ToneMappingCommand() override = default;

    protected:
        void OnReloadShaders(CommandContext& Resources) override
        {
            PipelineUpdateFromFile(Pipeline, "Nodes/PostProcess.glsl");
        }
        
        void OnUpdate(CommandContext& Resources, double DeltaTime) override
        {
            if (Resources.HasChanged<Size2D>("Output"))
            {
                Size2D size = Resources.GetValue<Size2D>("Output");
                FrameBuffer.Resize(size.x, size.y);
            }
        }
        
        void OnExecute(const CommandContext& Resources) override
        {
            Bind(Pipeline);
            Bind(FrameBuffer);
            Bind(Resources.Get<VertexArrayObject>(VEmptyVAO));
            
            // Scene storage buffers
            SetUniform(0, Resources.GetCameraBuffer());
        
            SetUniform(Pipeline, "SceneRadiance", 0, Resources.Get<Texture2D>("Scene Radiance"), Sampler);
        
            // Draw screen quad
            glDrawArrays(GL_TRIANGLES, 0, 3);
        
            UnBind(Pipeline);
            UnBind(FrameBuffer);
        }

    private:
        Location VEmptyVAO;
        FrameBuffer FrameBuffer;
        Pipeline Pipeline;
        Sampler Sampler;
    };
}
