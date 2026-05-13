#include "Modules/FrameGraph/Module.h"

#include "Core/Spec.h"
#include "Core/Module.h"
#include "Modules/Window/Module.h"
#include "Modules/Rendering/Module.h"
#include "Rendering/GLHelper.h"

namespace FrameGraph
{
    void Module::RegisterDependencies(Engine::Spec& spec)
    {
        spec.Register<Window::Module>();
        spec.Register<Rendering::Module>();
    }

    void Module::Initialize()
    {
        Window::Module* Window = Engine::GetModule<Window::Module>(Context());
        AssertOrError(Window != nullptr, "Window is null")

        m_CommandPool.emplace();

        uint32_t InitialWidth, InitialHeight;
        Window->GetFrameBufferSize(InitialWidth, InitialHeight);

        // TODO introduce a way to have outputs to the graph
        TexOutput = Resources().Add<Texture2D>("Output", InitialWidth, InitialHeight,  Texture::UnsignedByte, Texture::RGB);
        VOutputSize = Resources().AddVariable<FrameGraph::Size2D>("Output", FrameGraph::Size2D{InitialWidth, InitialHeight});

        m_OutputFrameBuffer.emplace(FrameBuffer::Attachment(Resources().Get<Texture2D>(TexOutput), FrameBuffer::ClearColor(0.0f)));
    }

    void Module::Tick(double deltaTime)
    {
        Window::Module* Window = Engine::GetModule<Window::Module>(Context());
        AssertOrError(Window != nullptr, "Window is null")

        // Handle Window resize
        uint32_t NextWidth, NextHeight;
        if (Window->GetFrameBufferSize(NextWidth, NextHeight))
        {
            Resources().Get<Texture2D>(TexOutput).Data(NextWidth, NextHeight);
            Resources().SetValue<FrameGraph::Size2D>(VOutputSize, FrameGraph::Size2D{NextWidth, NextHeight});
        }

        // Handle Shader Reload
        if (Window->ShouldRecompileShaders())
        {
            m_CommandPool->ReloadShaders();
        }

        // Update scene            
        m_CommandPool->Update(deltaTime);
        
        // Execute commands
        for (const auto& CommandList : m_CommandLists)
        {
            m_CommandPool->Render(CommandList);
        }

        // Move results to viewport
        // TODO cleanup and integrate an engine viewport
        glViewport(0, 0, NextWidth, NextHeight);
        glClear(GL_COLOR_BUFFER_BIT);

        glBlitNamedFramebuffer(m_OutputFrameBuffer->Handle(), /*Main Frame buffer ??*/ 0, 
            0, 0, NextWidth, NextHeight, 
            0, 0, NextWidth, NextHeight, 
                GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    void Module::Shutdown()
    {
        m_OutputFrameBuffer.reset();
        m_CommandPool.reset();
    }


    void Module::EditorUI()
    {

    }
}