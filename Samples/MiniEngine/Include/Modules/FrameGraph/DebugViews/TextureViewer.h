#pragma once

#include "imgui.h"
#include "Modules/FrameGraph/Commands.h"
#include "Rendering/FrameBuffers.h"
#include "Rendering/Pipelines.h"
#include "Rendering/Sampler.h"
#include "Rendering/Uniforms.h"

namespace FrameGraph
{
    class TextureViewer : public IGraphDebugView
    {
    public:
        enum TextureViewType
        {
            T_Texture2D = 0,
            T_TextureCube,
            T_Texture3D,
            
        };
        
        TextureViewer(CommandContext& Resources):
            IGraphDebugView(Resources),
            OutputSize(Resources.GetValue<Size2D>("Output")),
            SampledTextureBuffer(Resources.Add<Texture2D>("Texture Viewer Copy Buffer", OutputSize.x, OutputSize.y, Texture::Type::Float, Texture::Layout::RGB)),
            SampledTextureFrame(FrameBuffer::Attachment(Resources.Get<Texture2D>(SampledTextureBuffer), FrameBuffer::ClearColor(0.0f))),
            SampleTexturePipeline(PipelineFromFile("Texture Viewer ReSample", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "DebugViews/TextureViewer.glsl", SampleTexturePipelineDefines)),
            FrameBuffer(FrameBuffer::Attachment(Resources.Get<Texture2D>("Output"), FrameBuffer::ClearColor(0.0f))),
            CheckerboardBackgroundPipeline(PipelineFromFile("Texture Viewer Checkerboard Background", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "DebugViews/TextureViewer.glsl", CheckerboardBackgroundPipelineDefines)),
            DisplayTexturePipeline(PipelineFromFile("Texture Viewer Display Texture", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "DebugViews/TextureViewer.glsl", DisplayTexturePipelineDefines)),
            Sampler(Sampler::Params{
                .Magnification = Sampler::F_Nearest,
                .Minification = Sampler::F_Nearest,
                .MipMode = Sampler::M_Nearest
            })
        {
            UnsetTexture();
        }

        ~TextureViewer() override {}

        void SetTargetTexture(Location Texture, TextureViewType Type, size_t Mip, size_t Depth, size_t Index, size_t Sample)
        {
            TargetTexture = Texture;
            TargetMipLevel = Mip;
            TargetArrayLayer = Index;
            TargetDepth = Depth;
            TargetSample = Sample;
            TargetType = Type;
        }
        
        void UnsetTexture()
        {
            TargetTexture = std::numeric_limits<Location>::max();
            ResetSettings();
        }

    protected:
        void OnReloadShaders(CommandContext& Resources) override
        {
            PipelineUpdateFromFile(SampleTexturePipeline,"DebugViews/TextureViewer.glsl", SampleTexturePipelineDefines);
            PipelineUpdateFromFile(CheckerboardBackgroundPipeline, "DebugViews/TextureViewer.glsl", CheckerboardBackgroundPipelineDefines);
            PipelineUpdateFromFile(DisplayTexturePipeline, "DebugViews/TextureViewer.glsl", DisplayTexturePipelineDefines);
        }
        
        void OnUpdate(CommandContext& Resources, double DeltaTime) override
        {
            if (Resources.HasChanged<Size2D>("Output"))
            {
                Size2D size = Resources.GetValue<Size2D>("Output");
                Resources.Get<Texture2D>(SampledTextureBuffer).Data(size.x, size.y);
                SampledTextureFrame.Resize(size.x, size.y);
                FrameBuffer.Resize(size.x, size.y);
            }
            
            // if (!IsValidTexture()) return;
        }
        
        void OnExecute(const CommandContext& Resources) override
        {
            if (!IsValidTexture()) return;

            Size2D TargetSize = Resources.GetValue<Size2D>("Output");
            
            // Pass 1: Sample source image
            {
                Bind(SampledTextureFrame);
                SampledTextureFrame.Clear();

                Bind(SampleTexturePipeline);

                SetUniform(SampleTexturePipeline, "InputTexType", (uint32_t)TargetType);
                switch (TargetType)
                {
                case T_Texture2D:
                    {
                        Size2D SourceSize;
                        const Texture2D& Texture = Resources.Get<Texture2D>(TargetTexture);
                        SourceSize.x = Texture.Width();
                        SourceSize.y = Texture.Height();

                        SetUniform(SampleTexturePipeline, "InputTex2D", 0, Texture, Sampler);
                        SetUniform(SampleTexturePipeline, "InputSize", SourceSize);
                        SetUniform(SampleTexturePipeline, "LodBias", Texture.MipCount() > 0 ? (float)(TargetMipLevel) / (float)(Texture.MipCount()) : 0.0f);
                    }
                    break;
                case T_TextureCube:
                    {
                    const TextureCube& Texture = Resources.Get<TextureCube>(TargetTexture);
                    }
                    break;
                case T_Texture3D:
                    {
                        Size2D SourceSize;
                        const Texture3D& Texture = Resources.Get<Texture3D>(TargetTexture);
                        SourceSize.x = Texture.Width();
                        SourceSize.y = Texture.Height();

                        SetUniform(SampleTexturePipeline, "InputTex3D", 0, Texture, Sampler);
                        SetUniform(SampleTexturePipeline, "InputSize", SourceSize);
                        SetUniform(SampleTexturePipeline, "DepthBias", (float)(TargetDepth) / (float)(Texture.Depth()));
                        SetUniform(SampleTexturePipeline, "LodBias", Texture.MipCount() > 0 ? (float)(TargetMipLevel) / (float)(Texture.MipCount()) : 0.0f);
                    }
                    break;

                SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported texture type")
                }

                SetUniform(SampleTexturePipeline, "OutputSize", TargetSize);
                SetUniform(SampleTexturePipeline, "RangeRed", RangeRed);
                SetUniform(SampleTexturePipeline, "RangeGreen", RangeGreen);
                SetUniform(SampleTexturePipeline, "RangeBlue", RangeBlue);
                SetUniform(SampleTexturePipeline, "RangeAlpha", RangeAlpha);

                SetUniform(SampleTexturePipeline, "TagOutOfBounds", 0u);

                // Draw screen quad
                glDrawArrays(GL_TRIANGLES, 0, 3);
                
                UnBind(SampleTexturePipeline);
                UnBind(SampledTextureFrame);
            }

            Bind(FrameBuffer);
            
            // Pass 2: Draw Grid
            {
                Bind(CheckerboardBackgroundPipeline);

                SetUniform(CheckerboardBackgroundPipeline, "GridSize", (uint32_t)4);
                
                // Draw screen quad
                glDrawArrays(GL_TRIANGLES, 0, 3);

                UnBind(CheckerboardBackgroundPipeline);
            }
            
            // Pass 3: display the image
            {
                Bind(DisplayTexturePipeline);

                SetUniform(DisplayTexturePipeline, "Input", 0, Resources.Get<Texture2D>(TargetTexture), Sampler);
                SetUniform(DisplayTexturePipeline, "ConvertToGreyscale", ConvertToGreyscale);
                SetUniform(DisplayTexturePipeline, "ShowRed", ShowRed);
                SetUniform(DisplayTexturePipeline, "ShowGreen", ShowGreen);
                SetUniform(DisplayTexturePipeline, "ShowBlue", ShowBlue);
                SetUniform(DisplayTexturePipeline, "ShowAlpha", ShowAlpha);
                SetUniform(DisplayTexturePipeline, "UseOETF", UseOETF);
                
                // Draw screen quad
                glDrawArrays(GL_TRIANGLES, 0, 3);

                UnBind(DisplayTexturePipeline);
            }
            
            UnBind(FrameBuffer);
            
            // Pass 4: Histogram
            
        }
        
        void EditorUI() override
        {            
            ImGui::Checkbox("Show Reds", &ShowRed);     ImGui::SameLine(); ImGui::DragFloat2("Red Range", RangeRed.data());
            ImGui::Checkbox("Show Greens", &ShowGreen); ImGui::SameLine(); ImGui::DragFloat2("Green Range", RangeGreen.data());
            ImGui::Checkbox("Show Blues", &ShowBlue);   ImGui::SameLine(); ImGui::DragFloat2("Blue Range", RangeBlue.data());
            ImGui::Checkbox("Show Alphas", &ShowAlpha); ImGui::SameLine(); ImGui::DragFloat2("Alpha Range", RangeAlpha.data());
            RangeRed.x = std::min(RangeRed.x, RangeRed.y);
            RangeGreen.x = std::min(RangeGreen.x, RangeGreen.y);
            RangeBlue.x = std::min(RangeBlue.x, RangeBlue.y);
            RangeAlpha.x = std::min(RangeAlpha.x, RangeAlpha.y);
            RangeRed.y = std::max(RangeRed.x, RangeRed.y);
            RangeGreen.y = std::max(RangeGreen.x, RangeGreen.y);
            RangeBlue.y = std::max(RangeBlue.x, RangeBlue.y);
            RangeAlpha.y = std::max(RangeAlpha.x, RangeAlpha.y);
            
            if (ImGui::Button("Reset Channels"))
            {
                ResetChannels();
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset Ranges"))
            {
                ResetRanges();
            }

            ImGui::Checkbox("EOTF", &UseOETF);
        }

    private:
        bool IsValidTexture() const
        {
            return TargetTexture != std::numeric_limits<Location>::max();
        }

        void ResetRanges()
        {
            RangeRed = {0.0f, 1.0f};
            RangeGreen = {0.0f, 1.0f};
            RangeBlue = {0.0f, 1.0f};
            RangeAlpha = {0.0f, 1.0f};
        }

        void ResetChannels()
        {
            ShowRed = true;
            ShowGreen = true;
            ShowBlue = true;
            ShowAlpha = true;
        }

        void ResetOETF()
        {
            UseOETF = false;
        }

        void ResetZoom()
        {
            ZoomUVCenter = {0.5f};
            ZoomFactor = 1.0f;
        }

        void ResetSettings()
        {
            ResetRanges();
            ResetChannels();
            ResetZoom();
            ResetOETF();
        }

        // Frame Graph
        Size2D OutputSize;
        Location SampledTextureBuffer;
        
        // Rendering objects
        FrameBuffer SampledTextureFrame;
        Shader::DefineArray<1> SampleTexturePipelineDefines = {Shader::Define("RESAMPLE_PASS", "")};
        Pipeline SampleTexturePipeline;
        FrameBuffer FrameBuffer;
        Shader::DefineArray<1> CheckerboardBackgroundPipelineDefines = {Shader::Define("BACKGROUND_GRID_PASS", "")};
        Pipeline CheckerboardBackgroundPipeline;
        Shader::DefineArray<1> DisplayTexturePipelineDefines = {Shader::Define("DISPLAY_PASS", "")};
        Pipeline DisplayTexturePipeline;
        Sampler Sampler;
        
        // Texture targeting
        Location TargetTexture;
        size_t TargetMipLevel = 0;
        size_t TargetArrayLayer = 0;
        size_t TargetDepth = 0;
        size_t TargetSample = 0;
        TextureViewType TargetType = T_Texture2D;

        // Viewer settings
        Math::Vector2f RangeRed;
        Math::Vector2f RangeGreen;
        Math::Vector2f RangeBlue;
        Math::Vector2f RangeAlpha;
        bool UseOETF = false;
        bool ConvertToGreyscale = false;
        UInt OETF = 0;
        bool ShowRed;
        bool ShowGreen;
        bool ShowBlue;
        bool ShowAlpha;
        
        // Zoom
        Math::Vector2f ZoomUVCenter;
        float ZoomFactor;
    };
}
