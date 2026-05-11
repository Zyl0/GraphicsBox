#pragma once

#include "Modules/Rendering/Tools/Commands.h"

namespace Rendering::Graph
{
    class NativeResolutionRadiance  : public Command
    {
    public:
        NativeResolutionRadiance(CommandContext& Resources):
            Command(Resources),
            SceneRadianceSize(Resources.GetValue<Size2D>("Output")),
            SceneRadiance(Resources.Add<Texture2D>("Scene Radiance", SceneRadianceSize.x, SceneRadianceSize.y, Texture::Type::Packed_R11F_G11F_B10F, Texture::Layout::RGB)),
            SceneDepth(Resources.Add<Texture2D>("Scene Depth", SceneRadianceSize.x, SceneRadianceSize.y, Texture::Type::UnsignedInt, Texture::Layout::D)),
            VSceneRadianceSize(Resources.AddVariable<Size2D>("Scene Radiance", SceneRadianceSize))
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