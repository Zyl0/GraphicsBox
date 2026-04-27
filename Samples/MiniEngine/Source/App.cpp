#include "App.h"

#include <ranges>
#include <chrono>
#include <thread>

#include "imgui.h"
#include "Modules/Window/Module.h"
#include "Modules/ImGui/Module.h"
#include "Modules/Rendering/Module.h"

namespace Engine
{
    static constexpr float IdleDeltaTime = (1.0f / 5.0f); // 5 fps
    
    void App::Run()
    {
        // Register components
        for (TypeHash ID : m_Engine.m_UpdateOrder)
        {
            m_Engine.m_Modules[ID]->RegisterComponents();
        }
        
        // Awake modules
        for (TypeHash ID : m_Engine.m_UpdateOrder)
        {
            m_Engine.m_Modules[ID]->Initialize();
        }
        
        // Awake scene
        m_Engine.InitializeScene(); // TODO prevent registering any new component system once the scene has awaken
        
        Context EngineContext = m_Engine.GetContext();
        Window::Module* WindowModule = GetModule<Window::Module>(EngineContext);
        ImGui::Module* ImGuiModule = GetModule<ImGui::Module>(EngineContext);
        
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
                for (TypeHash ID : m_Engine.m_UpdateOrder)
                {
                    m_Engine.m_Modules[ID]->Tick(deltaTime);
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
                // double deltaTime = deltaTimeMs / 1000.0;
                prev_clock = curr_clock;

                // Slowdown app when not on focus
                if (!WindowModule->HasFocus())
                {
                    if (deltaTime < IdleDeltaTime)
                    {
                        using namespace std::this_thread; // sleep_for, sleep_until
                        using namespace std::chrono; // nanoseconds, system_clock, seconds

                        sleep_for(milliseconds(static_cast<uint32_t>(IdleDeltaTime - deltaTime)));
                    }
                }
                
                m_Engine.TickScene(deltaTime);
                
                // Tick modules
                for (TypeHash ID : m_Engine.m_UpdateOrder)
                {
                    // Pause the update when window is reduced
                    // TODO maybe this check should be in the module themselves so some can still be updated if not screen dependant
                    if (WindowModule->IsNotReduced())
                    {
                        m_Engine.m_Modules[ID]->Tick(deltaTime);
                    }
                }

                // Basic editor UI
                if (ImGuiModule != nullptr && WindowModule->IsNotReduced())
                {
                    // TODO do an integration with a viewport and a tab layout
                    ImGuiModule->_BeginFrame();

                    // TODO split
                    ImGui::Begin("Settings");
                    // Tick modules
                    for (TypeHash ID : m_Engine.m_UpdateOrder)
                    {
                        IModule* module = m_Engine.m_Modules[ID].get();
                        
                        ImGui::PushID(module);
                        module->EditorUI();
                        ImGui::PopID();
                    }
                    ImGui::End();

                    ImGuiModule->_EndFrame();
                }
            }
        }
        
        // Shutdown scene
        m_Engine.TerminateScene();
        
        // Shutdown modules
        for (size_t i = m_Engine.m_UpdateOrder.size(); i-- > 0;)
        {
            m_Engine.m_Modules[m_Engine.m_UpdateOrder[i]]->Shutdown();
        }
    }
}
