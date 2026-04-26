#include "App.h"

#include <ranges>

#include "Modules/Window/Module.h"

namespace Engine
{
    void App::Run()
    {
        // Register components
        for (auto& val : m_Engine.m_Modules | std::views::values)
        {
            val->RegisterComponents();
        }
        
        // Awake modules
        for (auto& val : m_Engine.m_Modules | std::views::values)
        {
            val->Initialize();
        }
        
        // Awake scene
        m_Engine.InitializeScene(); // TODO prevent registering any new component system once the scene has awaken
        
        Context EngineContext = m_Engine.GetContext();
        Window::Module* WindowModule = GetModule<Window::Module>(EngineContext);
        
        // keep track of time during the execution
        clock_t prev_clock = clock();
        clock_t curr_clock;
        
        
        // Headless runtime
        if (WindowModule == nullptr)
        {
            while (true) // TODO provide an engine comment to terminate the engine
            {
                curr_clock = clock();
                clock_t dcl = curr_clock - prev_clock;
                double deltaTime = static_cast<double>(dcl) / 1000000.0;
                prev_clock = curr_clock;
                
                m_Engine.TickScene(deltaTime);
                
                // Tick modules
                for (auto& val : m_Engine.m_Modules | std::views::values)
                {
                    val->Tick(deltaTime);
                }
            }
        }
        // Regular runtime
        else
        {
            while (!WindowModule->ShouldClose()) // TODO provide an engine comment to terminate the engine
            {
                curr_clock = clock();
                clock_t dcl = curr_clock - prev_clock;
                double deltaTime = static_cast<double>(dcl) / 1000000.0;
                prev_clock = curr_clock;
                
                m_Engine.TickScene(deltaTime);
                
                // Tick modules
                for (auto& val : m_Engine.m_Modules | std::views::values)
                {
                    val->Tick(deltaTime);
                }
            }
        }
        
        // Shutdown scene
        m_Engine.TerminateScene();
        
        // Shutdown modules
        for (auto& val : m_Engine.m_Modules | std::views::values)
        {
            val->Shutdown();
        }
    }
}
