#pragma once

#include "Modules/FrameGraph/Commands.h"

namespace FrameGraph
{
    class PreviousRadiance  : public ICommand
    {
    public:
        PreviousRadiance(CommandContext& Resources):
            ICommand(Resources),
            SceneRadianceSize(Resources.GetValue<Size2D>("Scene Radiance")),
            VUsePreviousRadiance(Resources.AddVariable<UInt>("Use Previous Radiance", 0u)),
            VSceneRadianceSize(Resources.GetLocation<Size2D>("Scene Radiance")),
            SceneRadiance(Resources.GetLocation<Texture2D>("Scene Radiance")),
            SceneDepth(Resources.GetLocation<Texture2D>("Scene Depth")),
            PrevSceneRadiance(Resources.Add<Texture2D>("Previous Scene Radiance", SceneRadianceSize.x, SceneRadianceSize.y, Texture::Type::Packed_R11F_G11F_B10F, Texture::Layout::RGB, true)),
            PrevSceneDepth(Resources.Add<Texture2D>("Previous Scene Depth", SceneRadianceSize.x, SceneRadianceSize.y, Texture::Type::UnsignedInt, Texture::Layout::D))
        {}

    protected:
        void OnReloadShaders(CommandContext& Resources) override {}
        void OnUpdate(CommandContext& Resources, double DeltaTime) override
        {
            if (Resources.HasChanged<Size2D>(VSceneRadianceSize))
            {
                SceneRadianceSize = Resources.GetValue<Size2D>(VSceneRadianceSize);
                Resources.Get<Texture2D>(PrevSceneRadiance).Data(SceneRadianceSize.x, SceneRadianceSize.y);
                Resources.Get<Texture2D>(PrevSceneDepth).Data(SceneRadianceSize.x, SceneRadianceSize.y);
            }
        }
        
        void OnExecute(const CommandContext& Resources) override
        {
            if (Resources.GetValue<UInt>(VUsePreviousRadiance) == 0) return;
            
            // TODO introduce a temporal rendering node to know when to reset history

            Size2D size = Resources.GetValue<Size2D>(VSceneRadianceSize);

            glCopyImageSubData(
                    Resources.Get<Texture2D>(SceneRadiance).Handle(), GL_TEXTURE_2D, 0,
                    0, 0, 0,
                    Resources.Get<Texture2D>(PrevSceneRadiance).Handle(), GL_TEXTURE_2D, 0,
                    0, 0, 0,
                    size.x, size.y, 1
                );

            Bind(Resources.Get<Texture2D>(SceneRadiance));
            glGenerateMipmap(GL_TEXTURE_2D);
            UnBind(Resources.Get<Texture2D>(SceneRadiance));

            glCopyImageSubData(
                    Resources.Get<Texture2D>(SceneDepth).Handle(), GL_TEXTURE_2D, 0,
                    0, 0, 0,
                    Resources.Get<Texture2D>(PrevSceneDepth).Handle(), GL_TEXTURE_2D, 0,
                    0, 0, 0,
                    size.x, size.y, 1
                );
        }

    private:
        Size2D SceneRadianceSize;
        Location VUsePreviousRadiance;
        Location VSceneRadianceSize;
        Location SceneRadiance;
        Location SceneDepth;
        Location PrevSceneRadiance;
        Location PrevSceneDepth;
    };
}