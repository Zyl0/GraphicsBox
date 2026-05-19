#pragma once

#include "Modules/FrameGraph/Commands.h"

namespace FrameGraph
{
    class NativeResolutionRadiance  : public ICommand
    {
    public:
        NativeResolutionRadiance(CommandContext& Resources):
            ICommand(Resources),
            SceneRadianceSize(Resources.GetValue<Size2D>("Output")),
            VSceneRadianceSize(Resources.AddVariable<Size2D>("Scene Radiance", SceneRadianceSize)),
            SceneRadiance(Resources.Add<Texture2D>("Scene Radiance", SceneRadianceSize.x, SceneRadianceSize.y, Texture::Type::Packed_R11F_G11F_B10F, Texture::Layout::RGB)),
            SceneRadianceMSAA(Resources.Add<Texture2D>("Scene Radiance MSAA", SceneRadianceSize.x, SceneRadianceSize.y, Texture::Type::Packed_R11F_G11F_B10F, Texture::Layout::RGB, (uint8_t)4)),
            SceneDepth(Resources.Add<Texture2D>("Scene Depth", SceneRadianceSize.x, SceneRadianceSize.y, Texture::Type::UnsignedInt, Texture::Layout::D)),
            SceneDepthMSAA(Resources.Add<Texture2D>("Scene Depth MSAA", SceneRadianceSize.x, SceneRadianceSize.y, Texture::Type::UnsignedInt, Texture::Layout::D, (uint8_t)4)),
            VUseMSAA(Resources.AddVariable("Use MSAA", false)),
            VMSAASampleCount(Resources.AddVariable<UInt>("MSAA Sample Count", 4))
        {}
        
        ~NativeResolutionRadiance() override = default;

    protected:
        void OnReloadShaders(CommandContext& Resources) override {}
        
        void OnUpdate(CommandContext& Resources, double DeltaTime) override
        {
            if (Resources.HasChanged<Size2D>("Output"))
            {
                SceneRadianceSize = Resources.GetValue<Size2D>("Output");
                Resources.Get<Texture2D>(SceneRadiance).Data(SceneRadianceSize.x, SceneRadianceSize.y);
                Resources.Get<Texture2D>(SceneDepth).Data(SceneRadianceSize.x, SceneRadianceSize.y);
                Resources.Get<Texture2D>(SceneRadianceMSAA).Data(SceneRadianceSize.x, SceneRadianceSize.y);
                Resources.Get<Texture2D>(SceneDepthMSAA).Data(SceneRadianceSize.x, SceneRadianceSize.y);
                Resources.SetValue<Size2D>(VSceneRadianceSize, SceneRadianceSize);
            }

            if (Resources.HasChanged<Bool>(VUseMSAA) || Resources.HasChanged<UInt>(VMSAASampleCount))
            {
                Bool UseMSAA = Resources.GetValue<Bool>(VUseMSAA);
                UInt SampleCount = Resources.GetValue<UInt>(VMSAASampleCount);

                SampleCount = UseMSAA ? SampleCount : 0;
                
                if (UseMSAA)
                {
                    Resources.Get<Texture2D>(SceneRadianceMSAA).Data((uint8_t) SampleCount);
                    Resources.Get<Texture2D>(SceneDepthMSAA).Data((uint8_t) SampleCount);
                }                
            }
        }
        
        void OnExecute(const CommandContext& Resources) override {}

    public:
        Size2D SceneRadianceSize;
        Location VSceneRadianceSize;
        Location SceneRadiance;
        Location SceneRadianceMSAA;
        Location SceneDepth;
        Location SceneDepthMSAA;
        Location VUseMSAA;
        Location VMSAASampleCount;
    };
}