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
            VUseMSAA(Resources.AddVariable("Use MSAA", false)),
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

    private:
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
