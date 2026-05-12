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
            SceneDepth(Resources.Add<Texture2D>("Scene Depth", SceneRadianceSize.x, SceneRadianceSize.y, Texture::Type::UnsignedInt, Texture::Layout::D))
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
                Resources.SetValue<Size2D>(VSceneRadianceSize, SceneRadianceSize);
            }
        }
        
        void OnExecute(const CommandContext& Resources) override {}

    public:
        Size2D SceneRadianceSize;
        Location VSceneRadianceSize;
        Location SceneRadiance;
        Location SceneDepth;
    };
}