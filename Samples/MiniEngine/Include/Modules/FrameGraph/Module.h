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

        void ClearCommandLists() {m_CommandLists.clear();}
        void AddCommandList(const CommandList& CommandList) {m_CommandLists.push_back(CommandList.Data());}

    private:
        enum ETextureViewer_TexType : uint8_t
        {
            T2D, TCube, T3D
        };
        
        void UnsetDebugView();
        void SetDebugViewTexture(Location Texture, ETextureViewer_TexType TexType, size_t Mip, size_t Depth, size_t Index, size_t Sample);
        void SelectLatestCommands();
        
        std::optional<CommandPool> m_CommandPool;
        std::optional<FrameBuffer> m_OutputFrameBuffer;
        std::vector<std::span<const Location>> m_CommandLists;
        Location TexOutput;
        Location VOutputSize;

        // Debug Views
        bool m_CommandDebugView;
        bool m_GraphDebugView;
        Location m_CurrentDebugView;
        Location m_UpToCommandList;
        Location m_UpToCommand;

        // Texture visualizer
        Location m_TextureViewer;
        ETextureViewer_TexType m_TextureViewerTexType;
        Location m_TextureViewerResource;

        // Editor
        int m_EditorCurrentCommand;
    };
}