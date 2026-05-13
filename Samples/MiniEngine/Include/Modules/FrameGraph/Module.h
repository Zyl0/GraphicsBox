#pragma once

#include "Shared/Annotations.h"

#include "Core/Module.h"
#include "Commands.h"

#include "Rendering/FrameBuffers.h"

namespace FrameGraph
{
    class Module : public Engine::IModule
    {
    public:
        Module() = default;
        ~Module() override = default;

        void RegisterDependencies(Engine::Spec& spec) override;

        void Initialize() override;

        void Tick(double deltaTime) override;

        void Shutdown() override;

        void EditorUI() override;

        template<typename T> 
        INLINE Location PushNode()
        {
            return m_CommandPool->PushNode<T>();
        }

        INLINE CommandContext& Resources() {return m_CommandPool->Context();}

    private:
        std::optional<CommandPool> m_CommandPool;
        std::optional<FrameBuffer> m_OutputFrameBuffer;
        Location TexOutput;
        Location VOutputSize;
    };
}