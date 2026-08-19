#pragma once

#include "Modules/FrameGraph/Commands.h"
#include "Rendering/Debug.h"
#include "Rendering/FrameBuffers.h"

namespace FrameGraph
{    
    class MotionVectors  : public ICommand
    {
    public:
        MotionVectors(CommandContext& Resources):
            ICommand(Resources),
            SceneRadianceSize(Resources.GetValue<Size2D>("Scene Radiance")),
            VSceneRadianceSize(Resources.GetLocation<Size2D>("Scene Radiance")),
            VUseMSAA(Resources.GetLocation<Bool>("Use MSAA")),
            VMSAASampleCount(Resources.AddVariable<UInt>("MSAA Sample Count", 4)),
            VUseMotionVectors(Resources.AddVariable<UInt>("Use Motion Vectors", 0u)),
            VUsePreviousMotionVectors(Resources.AddVariable<UInt>("Use Previous Motion Vectors", 0u)),
            CurrentMotionVectors(Resources.Add<Texture2D>("Motion Vectors", SceneRadianceSize.x, SceneRadianceSize.y, Texture::Type::Half, Texture::Layout::RGB, true)),
            CurrentMotionVectorsMSAA(Resources.Add<Texture2D>("Motion Vectors MSAA", SceneRadianceSize.x, SceneRadianceSize.y, Texture::Type::Half, Texture::Layout::RGB, true, (uint8_t)4)),
            PrevMotionVectors(Resources.Add<Texture2D>("Previous Motion Vectors", SceneRadianceSize.x, SceneRadianceSize.y, Texture::Type::Half, Texture::Layout::RGB, true)),
            PrevMotionVectorsMSAA(Resources.Add<Texture2D>("Previous Motion Vectors MSAA", SceneRadianceSize.x, SceneRadianceSize.y, Texture::Type::Half, Texture::Layout::RGB, true, (uint8_t)4)),
            HasMotionVectorLock(false),
            FrameBufferBase(FrameBuffer::Attachment(Resources.Get<Texture2D>(CurrentMotionVectors), FrameBuffer::ClearColor(0.0f)), nullptr),
            FrameBufferMSAA(FrameBuffer::Attachment(Resources.Get<Texture2D>(CurrentMotionVectorsMSAA), FrameBuffer::ClearColor(0.0f)), nullptr)
        {}

    protected:
        void OnReloadShaders(CommandContext& Resources) override {}
        void OnUpdate(CommandContext& Resources, double DeltaTime) override
        {
            if (Resources.HasChanged<Size2D>(VSceneRadianceSize))
            {
                SceneRadianceSize = Resources.GetValue<Size2D>(VSceneRadianceSize);
                Resources.Get<Texture2D>(CurrentMotionVectors).Data(SceneRadianceSize.x, SceneRadianceSize.y);
                Resources.Get<Texture2D>(CurrentMotionVectorsMSAA).Data(SceneRadianceSize.x, SceneRadianceSize.y);
                Resources.Get<Texture2D>(PrevMotionVectors).Data(SceneRadianceSize.x, SceneRadianceSize.y);
                Resources.Get<Texture2D>(PrevMotionVectorsMSAA).Data(SceneRadianceSize.x, SceneRadianceSize.y);
                FrameBufferBase.Resize(SceneRadianceSize.x, SceneRadianceSize.y);
                FrameBufferMSAA.Resize(SceneRadianceSize.x, SceneRadianceSize.y);
            }

            if (Resources.HasChanged<Bool>(VUseMSAA) || Resources.HasChanged<UInt>(VMSAASampleCount))
            {
                Bool UseMSAA = Resources.GetValue<Bool>(VUseMSAA);
                UInt SampleCount = Resources.GetValue<UInt>(VMSAASampleCount);

                SampleCount = UseMSAA ? SampleCount : 0;
                
                if (UseMSAA)
                {
                    Resources.Get<Texture2D>(CurrentMotionVectorsMSAA).Data((uint8_t) SampleCount);
                    Resources.Get<Texture2D>(PrevMotionVectorsMSAA).Data((uint8_t) SampleCount);
                }                
            }

            if (Resources.HasChanged<UInt>(VUsePreviousMotionVectors))
            {
                if (Resources.GetValue<UInt>(VUsePreviousMotionVectors) > 0 && !HasMotionVectorLock)
                {
                    Resources.SetValue<UInt>(VUseMotionVectors, Resources.GetValue<UInt>(VUseMotionVectors) + 1);
                    HasMotionVectorLock = true;
                }
                else if (Resources.GetValue<UInt>(VUsePreviousMotionVectors) == 0 && HasMotionVectorLock)
                {
                    Resources.SetValue<UInt>(VUseMotionVectors, Resources.GetValue<UInt>(VUseMotionVectors) - 1);
                    HasMotionVectorLock = false;
                }
            }
        }
        
        void OnExecute(const CommandContext& Resources) override
        {
            if (Resources.GetValue<UInt>(VUsePreviousMotionVectors) > 0) 
            {
                DebugScopeMarker scope("Copy To Previous Motion Vectors");

                Size2D size = Resources.GetValue<Size2D>(VSceneRadianceSize);

                if (Resources.GetValue<Bool>(VUseMSAA))
                {
                    glCopyImageSubData(
                        Resources.Get<Texture2D>(CurrentMotionVectorsMSAA).Handle(), GL_TEXTURE_2D_MULTISAMPLE, 0,
                        0, 0, 0,
                        Resources.Get<Texture2D>(PrevMotionVectorsMSAA).Handle(), GL_TEXTURE_2D_MULTISAMPLE, 0,
                        0, 0, 0,
                        size.x, size.y, 1
                    );
                }
                else
                {
                    glCopyImageSubData(
                        Resources.Get<Texture2D>(CurrentMotionVectors).Handle(), GL_TEXTURE_2D, 0,
                        0, 0, 0,
                        Resources.Get<Texture2D>(PrevMotionVectors).Handle(), GL_TEXTURE_2D, 0,
                        0, 0, 0,
                        size.x, size.y, 1
                    );
                }
            }

            if (Resources.GetValue<Bool>(VUseMSAA))
            {
                FrameBufferMSAA.Clear();
            }
            else
            {
                FrameBufferBase.Clear();
            }
        }

        void RegisterDebugViews(CommandContext& Resources, CommandDebugViewList& DebugViews) override
        {
            DebugViews.PushDebugView<DebugView>(Resources);
        }

    private:
        class DebugView : public ICommandDebugView
        {
        public:
            DebugView(CommandContext& Resources): 
                ICommandDebugView(Resources),
                SceneRadianceSize(Resources.GetLocation<Size2D>("Scene Radiance")),
                VSceneRadianceSize(Resources.GetLocation<Size2D>("Scene Radiance")),
                MotionVectors(Resources.GetLocation<Texture2D>("Motion Vectors")),
                MotionVectorsMSAA(Resources.GetLocation<Texture2D>("Motion Vectors MSAA")),
                VUseMSAA(Resources.GetLocation<Bool>("Use MSAA")),
                ResolveMSAAFrameBuffer(FrameBuffer::Attachment(Resources.Get<Texture2D>("Motion Vectors"), FrameBuffer::ClearColor(0.0)), nullptr),
                MSAAFrameBuffer(FrameBuffer::Attachment(Resources.Get<Texture2D>("Motion Vectors MSAA"), FrameBuffer::ClearColor(0.0)), nullptr),
                DrawMotionVectors(PipelineFromFile("Visualize Motion Vectors", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "DebugViews/MotionVectors.glsl", DrawMotionVectorsDefines)),
                DrawMotionArrows(PipelineFromFile("Visualize Motion Arrows", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "DebugViews/MotionVectors.glsl", DrawMotionArrowsDefines)),
                FrameBuffer(FrameBuffer::Attachment(Resources.Get<Texture2D>("Output"), FrameBuffer::ClearColor(0.0f)))
            {}

            ~DebugView() override {}

        protected:
            void OnReloadShaders(CommandContext& Resources) override
            {
                PipelineUpdateFromFile(DrawMotionVectors, "DebugViews/MotionVectors.glsl", DrawMotionVectorsDefines);
                PipelineUpdateFromFile(DrawMotionArrows, "DebugViews/MotionVectors.glsl", DrawMotionArrowsDefines);
            }
            
            void OnUpdate(CommandContext& Resources, double DeltaTime) override
            {
                if (Resources.HasChanged<Size2D>(VSceneRadianceSize))
                {
                    SceneRadianceSize = Resources.GetValue<Size2D>(VSceneRadianceSize);
                    ResolveMSAAFrameBuffer.Resize(SceneRadianceSize.x, SceneRadianceSize.y);
                    MSAAFrameBuffer.Resize(SceneRadianceSize.x, SceneRadianceSize.y);
                    FrameBuffer.Resize(SceneRadianceSize.x, SceneRadianceSize.y);
                }
            }
            void OnPrepare(CommandContext& Resources, const ICommand& Caller, double DeltaTime) override {}
            void OnExecute(const CommandContext& Resources, const ICommand& Caller) override
            {
                // Resolve multi sampled motion
                if (Resources.GetValue<Bool>(VUseMSAA))
                {
                    Size2D SceneRadianceSize = Resources.GetValue<Size2D>(VSceneRadianceSize);

                    Bind(ResolveMSAAFrameBuffer, MSAAFrameBuffer);
                    
                    glBlitFramebuffer(
                        0, 0, SceneRadianceSize.x, SceneRadianceSize.y,
                        0, 0, SceneRadianceSize.x, SceneRadianceSize.y,
                        GL_COLOR_BUFFER_BIT, GL_NEAREST );
                }
                
                Bind(FrameBuffer);
                
                Bind(DrawMotionVectors);
                
                SetUniform(DrawMotionVectors, "MotionVectors", 0, Resources.Get<Texture2D>(MotionVectors));
                
                SetUniform(DrawMotionVectors, "RangeRed", Vector2f(-0.002, 0.002));
                SetUniform(DrawMotionVectors, "RangeGreen", Vector2f(-0.002, 0.002));
                SetUniform(DrawMotionVectors, "RangeBlue", Vector2f(-0.002, 0.002));
                SetUniform(DrawMotionVectors, "UseOETF", UseOETF);
                
                // Draw screen quad
                glDrawArrays(GL_TRIANGLES, 0, 3);
                
                UnBind(DrawMotionVectors);
                
                if (ShowArrows)
                {
                    Bind(DrawMotionArrows);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glBlendEquation(GL_FUNC_ADD);
                    
                    SetUniform(DrawMotionArrows, "MotionVectors", 0, Resources.Get<Texture2D>(MotionVectors));
                    
                    SetUniform(DrawMotionArrows, "viewportSize", Resources.GetValue<Size2D>(VSceneRadianceSize));
                    SetUniform(DrawMotionArrows, "gridSize", ArrowGridSize);
                    SetUniform(DrawMotionArrows, "motionScale", ArrowScale);
                    
                    SetUniform(DrawMotionArrows, "UseOETF", UseOETF);
                    glDrawArrays(GL_TRIANGLES, 0, 3);
                    
                    
                    glDisable(GL_BLEND);
                    UnBind(DrawMotionArrows);
                }
                
                UnBind(FrameBuffer);
            }
            
            void EditorUI() override
            {
                ImGui::Checkbox("Show Motion Vector Arrows", &ShowArrows);
                ImGui::DragFloat("Arrow Scale", &ArrowScale, 0.25f, 1, 128.f);
                ImGui::Checkbox("Use OETF", &UseOETF);
            }
            
        private:
            Size2D SceneRadianceSize;
            Location VSceneRadianceSize;
            Location MotionVectors;
            Location MotionVectorsMSAA;
            Location VUseMSAA;
            FrameBuffer ResolveMSAAFrameBuffer;
            FrameBuffer MSAAFrameBuffer;
            Shader::DefineArray<1> DrawMotionVectorsDefines = {Shader::Define("DISPLAY_PASS", "")};
            Pipeline DrawMotionVectors;
            Shader::DefineArray<1> DrawMotionArrowsDefines = {Shader::Define("MOTION_VECTORS_ARROWS_PASS", "")};
            Pipeline DrawMotionArrows;
            FrameBuffer FrameBuffer;
            
            float ArrowScale = 1.0f;
            bool ShowArrows = false;
            uint32_t ArrowGridSize = 8;
            bool UseOETF = true;
        };
        
        
        Size2D SceneRadianceSize;
        Location VSceneRadianceSize;
        Location VUseMSAA;
        Location VMSAASampleCount;
        Location VUseMotionVectors;
        Location VUsePreviousMotionVectors;
        Location CurrentMotionVectors;
        Location CurrentMotionVectorsMSAA;
        Location PrevMotionVectors;
        Location PrevMotionVectorsMSAA;
        bool HasMotionVectorLock;
        FrameBuffer FrameBufferBase;
        FrameBuffer FrameBufferMSAA;
    };
}
