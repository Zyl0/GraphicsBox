#include "App.h"

#include <ranges>
#include <chrono>
#include <thread>

#include "imgui.h"
#include "Modules/Editor/Module.h"
#include "Modules/Window/Module.h"
#include "Modules/ImGui/Module.h"
#include "Rendering/Debug.h"

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
        Editor::Module* EditorModule = GetModule<Editor::Module>(EngineContext);
        
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
            // Editor subviewport setup
            if (EditorModule != nullptr)
            {
                ImGuiIO& io = ImGui::GetIO();
                io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
                
                WindowModule->_EnableSubViewport(500, 500);
            }
            
            ImVec2 SubViewportSize = {500, 500};
            
            while (!WindowModule->ShouldClose()) // TODO provide an engine command to terminate the engine
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
                
                // Editor subviewport setup
                if (EditorModule != nullptr)
                {
                    WindowModule->_SetViewportSubViewport(SubViewportSize.x, SubViewportSize.y);
                }
                
                {
                    DebugScopeMarker scope = {"App Draw calls"};
                    
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
                }
                
                if (ImGuiModule != nullptr && WindowModule->IsNotReduced())
                {
                    DebugScopeMarker scope = {"App Editor UI"};
                    
                    WindowModule->_SetViewportMainWindow();
                    
                    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, WindowModule->ViewportFrameBuffer());
                    
                    ImGuiModule->_BeginFrame();
                    if (EditorModule != nullptr)
                    {
                        // Dock space
                        {
                            ImGuiViewport* viewport = ImGui::GetMainViewport();
                            ImGui::SetNextWindowPos(viewport->WorkPos);
                            ImGui::SetNextWindowSize(viewport->WorkSize);
                            ImGui::SetNextWindowViewport(viewport->ID);
                            
                            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
                            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
                            window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
                            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

                            ImGui::Begin("DockSpace", nullptr, window_flags);

                            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

                            ImGui::End();
                        }
                        
                        // Viewport
                        {
                            ImGui::Begin("Viewport");
                            SubViewportSize = ImGui::GetContentRegionAvail();

                            if (SubViewportSize.x > 0 && SubViewportSize.y > 0)
                            {
                                ImTextureID ViewportTexID = WindowModule->SubViewportWriteBuffer();
                                ImGui::Image(ViewportTexID, SubViewportSize, ImVec2(0,1), ImVec2(1, 0));
                            }

                            ImGui::End();
                        }
                        
                        {
                            DebugScopeMarker scope2 = {"App UI"};
                            
                            // Tick modules
                            for (TypeHash ID : m_Engine.m_UpdateOrder)
                            {
                                IModule* module = m_Engine.m_Modules[ID].get();

                                EditorWindowParams params = module->EditorWindow();
                                if (!params.Valid) continue;

                                ImGui::Begin(m_Engine.GetModuleName(ID).data());
                        
                                ImGui::PushID(module);
                                module->EditorUI();
                                ImGui::PopID();
                                ImGui::End();
                            }
                        }
                    }
                    else // Basic editor UI
                    {
                        DebugScopeMarker scope2 = {"App UI"};
                        
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
                    }
                    

                    ImGuiModule->_EndFrame();
                }
            }
            
            // Editor subviewport setup
            if (EditorModule != nullptr)
            {
                WindowModule->_DisableSubViewport();
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
