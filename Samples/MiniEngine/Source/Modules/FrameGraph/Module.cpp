#include "Modules/FrameGraph/Module.h"

#include "Core/Spec.h"
#include "Core/Module.h"
#include "Modules/Editor/Module.h"
#include "Modules/Window/Module.h"
#include "Modules/Rendering/Module.h"

#include "imgui.h"
#include "Modules/FrameGraph/DebugViews/TextureViewer.h"
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

        Rendering::Module* Rendering = Engine::GetModule<Rendering::Module>(Context());
        AssertOrError(Rendering != nullptr, "Rendering is null")

        Bool EnableReflectionData = Engine::GetModule<Editor::Module>(Context()) != nullptr;
        Bool EnableDebugViews = EnableReflectionData;
        
        m_CommandPool.emplace(EnableReflectionData, EnableDebugViews);

        uint32_t InitialWidth, InitialHeight;
        Window->GetFrameBufferSize(InitialWidth, InitialHeight);

        // TODO introduce a way to have outputs to the graph
        TexOutput = Resources().Add<Texture2D>("Output", InitialWidth, InitialHeight,  Texture::UnsignedByte, Texture::RGB);
        VOutputSize = Resources().AddVariable<FrameGraph::Size2D>("Output", FrameGraph::Size2D{InitialWidth, InitialHeight});
        Resources().Add<VertexArrayObject>("Empty VAO");

        m_OutputFrameBuffer.emplace(FrameBuffer::Attachment(Resources().Get<Texture2D>(TexOutput), FrameBuffer::ClearColor(0.0f)));

        m_CommandPool->PushDebugView<TextureViewer>();
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

        if (NextWidth == 0 || NextHeight == 0) return;

        // Render Graph
        if (m_CommandDebugView || m_GraphDebugView)
        {
            if (m_CommandDebugView)
            {
                // Execute commands up to debug view
                for (Location ListIndex = 0; ListIndex < m_CommandLists.size(); ListIndex++)
                {
                    const auto& CommandList = m_CommandLists[ListIndex];

                    if (ListIndex == m_UpToCommandList)
                    {
                        m_CommandPool->RenderToCommandDebugView(deltaTime, CommandList, m_UpToCommand, m_CurrentDebugView);
                        
                        break;
                    }
                    else
                    {
                        m_CommandPool->Render(CommandList);
                    }
                }                
            }
            else // m_GraphDebugView
            {
                // Execute commands up to debug view
                for (Location ListIndex = 0; ListIndex < m_CommandLists.size(); ListIndex++)
                {
                    const auto& CommandList = m_CommandLists[ListIndex];

                    if (ListIndex == m_UpToCommandList)
                    {
                        m_CommandPool->RenderToGraphDebugView(CommandList, m_UpToCommand, m_CurrentDebugView);
                        
                        break;
                    }
                    else
                    {
                        m_CommandPool->Render(CommandList);
                    }
                }       
            }
        }
        else
        {        
            // Execute commands as is
            for (const auto& CommandList : m_CommandLists)
            {
                m_CommandPool->Render(CommandList);
            }
        }

        // Move results to viewport
        // TODO cleanup and integrate an engine viewport
        glViewport(0, 0, NextWidth, NextHeight);
        glClear(GL_COLOR_BUFFER_BIT);

        glBlitNamedFramebuffer(m_OutputFrameBuffer->Handle(), Window->ViewportFrameBuffer(), 
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
        Bool EnableReflectionData = Engine::GetModule<Editor::Module>(Context()) != nullptr;
        if (!EnableReflectionData || !m_CommandPool->Context().HasReflection()) return;

        auto& Resources = m_CommandPool->Context();

        if (ImGui::BeginTabBar("Tools"))
        {
            // Resources object explorer
            if (ImGui::BeginTabItem("Rendering Objects"))
            {
                if(ImGui::Button("Select None"))
                {
                    UnsetDebugView();
                }
                ImGui::SameLine();
                static char SearchBuffer[256] = {'\0'};
                ImGui::InputText("Search objects", SearchBuffer, 256);

                // Objects
                if (ImGui::BeginTable("Object Table", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
                {
                    ImGui::TableSetupColumn("Name");
                    ImGui::TableSetupColumn("Type");
                    ImGui::TableHeadersRow();

                    for (const auto& Object : Resources.ReflectedObjects<VertexBuffer>())
                    {
                        if (SearchBuffer[0] != '\0' && Object.FieldName.find(SearchBuffer) == std::string::npos) continue;

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        // if (ImGui::Selectable(Object.FieldName.data(), , ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {}
                        ImGui::Text(Object.FieldName.data());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("VertexBuffer");
                    }

                    for (const auto& Object : Resources.ReflectedObjects<IndexBuffer>())
                    {
                        if (SearchBuffer[0] != '\0' && Object.FieldName.find(SearchBuffer) == std::string::npos) continue;

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        // if (ImGui::Selectable(Object.FieldName.data(), , ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {}
                        ImGui::Text(Object.FieldName.data());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("IndexBuffer");
                    }

                    for (const auto& Object : Resources.ReflectedObjects<VertexArrayObject>())
                    {
                        if (SearchBuffer[0] != '\0' && Object.FieldName.find(SearchBuffer) == std::string::npos) continue;

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        // if (ImGui::Selectable(Object.FieldName.data(), , ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {}
                        ImGui::Text(Object.FieldName.data());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("VertexArrayObject");
                    }

                    for (const auto& Object : Resources.ReflectedObjects<UniformBuffer>())
                    {
                        if (SearchBuffer[0] != '\0' && Object.FieldName.find(SearchBuffer) == std::string::npos) continue;

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        // if (ImGui::Selectable(Object.FieldName.data(), , ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {}
                        ImGui::Text(Object.FieldName.data());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("UniformBuffer");
                    }

                    for (const auto& Object : Resources.ReflectedObjects<StorageBuffer>())
                    {
                        if (SearchBuffer[0] != '\0' && Object.FieldName.find(SearchBuffer) == std::string::npos) continue;

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        // if (ImGui::Selectable(Object.FieldName.data(), , ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {}
                        ImGui::Text(Object.FieldName.data());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("StorageBuffer");
                    }

                    for (const auto& Object : Resources.ReflectedObjects<Texture2D>())
                    {
                        if (SearchBuffer[0] != '\0' && Object.FieldName.find(SearchBuffer) == std::string::npos) continue;

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        bool IsSelected = m_CurrentDebugView == m_TextureViewer && m_TextureViewerTexType == T2D && m_TextureViewerResource == Object.Location;
                        if (ImGui::Selectable(Object.FieldName.data(), IsSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
                        {
                            Location CommandList = m_CommandLists.size() - 1;
                            SetDebugViewTexture(Object.Location, T2D, 0, 0, 0, Resources.Get<Texture2D>(Object.Location).SampleCount());
                        }
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Texture2D");
                    }
                    
                    for (const auto& Object : Resources.ReflectedObjects<Texture2DArray>())
                    {
                        if (SearchBuffer[0] != '\0' && Object.FieldName.find(SearchBuffer) == std::string::npos) continue;

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        // if (ImGui::Selectable(Object.FieldName.data(), , ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {}
                        ImGui::Text(Object.FieldName.data());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Texture2DArray");
                    }

                    for (const auto& Object : Resources.ReflectedObjects<Texture3D>())
                    {
                        if (SearchBuffer[0] != '\0' && Object.FieldName.find(SearchBuffer) == std::string::npos) continue;

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        // if (ImGui::Selectable(Object.FieldName.data(), , ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {}
                        ImGui::Text(Object.FieldName.data());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Texture3D");
                    }

                    for (const auto& Object : Resources.ReflectedObjects<TextureCube>())
                    {
                        if (SearchBuffer[0] != '\0' && Object.FieldName.find(SearchBuffer) == std::string::npos) continue;

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        // if (ImGui::Selectable(Object.FieldName.data(), , ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {}
                        ImGui::Text(Object.FieldName.data());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("TextureCube");
                    }
                    
                    ImGui::EndTable();
                }

                ImGui::Separator();

                // Commands
                {
                    static int CurrentItem = -1;

                    size_t CommandCount = 0;
                    for (const auto & list : m_CommandLists) CommandCount += list.size();
                
                    struct CLCTX
                    {
                        std::span<const std::span<const Location>> CommandLists;
                        CommandPool& CommandPool;
                        size_t Count;
                    } ctx {.CommandLists = m_CommandLists, .CommandPool = *m_CommandPool, .Count = CommandCount};
                
                    auto GetName = [](void* Context, int index) -> const char*
                    {
                        if (index < 0) return nullptr;

                        CLCTX* CTX = (CLCTX*)Context;
                    
                        std::span<const ctti::detail::cstring> CommandNames = CTX->CommandPool.CommandNames();

                        return CommandNames[index].begin();
                    };

                    if (ImGui::ListBox("CommandList(s)", &CurrentItem, GetName, &ctx, CommandCount))
                    {
                        if (CurrentItem >= 0)
                        {
                            CommandCount = 0;
                            size_t CommandList = 0;
                            for (const auto & list : m_CommandLists)
                            {
                                if (CommandCount + list.size() < static_cast<Location>(CurrentItem))
                                {
                                    CommandCount += list.size();
                                    CommandList++;
                                }
                                else
                                {
                                    m_UpToCommand = list[static_cast<Location>(CurrentItem) - CommandCount];
                                    m_UpToCommandList = CommandList;
                                    break;
                                }
                            }
                        }
                        else if (m_UpToCommandList > 0)
                        {
                            m_UpToCommand = m_CommandLists.back().back();
                            m_UpToCommandList = m_CommandLists.size() - 1;
                        }
                    }
                }
                
                ImGui::EndTabItem();
            }

            // Resources variables explorer
            if (ImGui::BeginTabItem("Graph Variables"))
            {            
                static char SearchBuffer[256] = {'\0'};
                ImGui::InputText("Search variables", SearchBuffer, 256);

                if (ImGui::BeginTable("Variable Table", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX))
                {
                    ImGui::TableSetupColumn("Name");
                    ImGui::TableSetupColumn("Type");
                    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Reset");
                    ImGui::TableHeadersRow();

                    for (const auto& Object : Resources.ReflectedVariables<Bool>())
                    {
                        if (SearchBuffer[0] != '\0' && Object.FieldName.find(SearchBuffer) == std::string::npos) continue;

                        ImGui::PushID(&Object);
                        
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        // if (ImGui::Selectable(Object.FieldName.data(), , ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {}
                        ImGui::Text(Object.FieldName.data());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Bool");
                        ImGui::TableSetColumnIndex(2);
                        {
                            Bool Copy = Resources.GetValue<Bool>(Object.Location);
                            if (ImGui::Checkbox("##check", &Copy))
                            {
                                Resources.SetValue<Bool>(Object.Location, Copy);
                            }
                        }
                        ImGui::TableSetColumnIndex(3);
                        if (ImGui::Button("reset"))
                        {
                            Resources.SetValue<Bool>(Object.Location, Object.BaseValue);
                        }

                        ImGui::PopID();
                    }

                    for (const auto& Object : Resources.ReflectedVariables<UInt>())
                    {
                        if (SearchBuffer[0] != '\0' && Object.FieldName.find(SearchBuffer) == std::string::npos) continue;

                        ImGui::PushID(&Object);
                        
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        // if (ImGui::Selectable(Object.FieldName.data(), , ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {}
                        ImGui::Text(Object.FieldName.data());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("UInt");
                        ImGui::TableSetColumnIndex(2);
                        {
                            UInt Copy = Resources.GetValue<UInt>(Object.Location);
                            if (Object.MinValue != 0 && Object.MaxValue != 0)
                            {
                                if (ImGui::SliderInt("##slider", (int*)&Copy, Object.MinValue, std::min(Object.MaxValue, (UInt)INT32_MAX)))
                                {
                                    Resources.SetValue<UInt>(Object.Location, Copy);
                                }
                            }
                            else
                            {
                                if (ImGui::DragInt("##drag", (int*)&Copy, 1, std::max(0u, Object.MinValue), std::min(Object.MaxValue, (UInt)INT32_MAX) ))
                                {
                                    Resources.SetValue<UInt>(Object.Location, Copy);
                                }
                            }
                            
                        }
                        ImGui::TableSetColumnIndex(3);
                        if (ImGui::Button("reset"))
                        {
                            Resources.SetValue<UInt>(Object.Location, Object.BaseValue);
                        }

                        ImGui::PopID();
                    }

                    for (const auto& Object : Resources.ReflectedVariables<Int>())
                    {
                        if (SearchBuffer[0] != '\0' && Object.FieldName.find(SearchBuffer) == std::string::npos) continue;

                        ImGui::PushID(&Object);
                        
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        // if (ImGui::Selectable(Object.FieldName.data(), , ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {}
                        ImGui::Text(Object.FieldName.data());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Int");
                        ImGui::TableSetColumnIndex(2);
                        {
                            Int Copy = Resources.GetValue<Int>(Object.Location);
                            if (Object.MinValue != 0 && Object.MaxValue != 0)
                            {
                                if (ImGui::SliderInt("##slider", &Copy, Object.MinValue, Object.MaxValue))
                                {
                                    Resources.SetValue<Int>(Object.Location, Copy);
                                }
                            }
                            else
                            {
                                if (ImGui::DragInt("##drag", (int*)&Copy, 1, std::max(INT32_MIN, Object.MinValue), std::min(Object.MaxValue, INT32_MAX) ))
                                {
                                    Resources.SetValue<Int>(Object.Location, Copy);
                                }
                            }
                        }
                        ImGui::TableSetColumnIndex(3);
                        if (ImGui::Button("reset"))
                        {
                            Resources.SetValue<Int>(Object.Location, Object.BaseValue);
                        }

                        ImGui::PopID();
                    }

                    for (const auto& Object : Resources.ReflectedVariables<Float>())
                    {
                        if (SearchBuffer[0] != '\0' && Object.FieldName.find(SearchBuffer) == std::string::npos) continue;

                        ImGui::PushID(&Object);
                        
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        // if (ImGui::Selectable(Object.FieldName.data(), , ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {}
                        ImGui::Text(Object.FieldName.data());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Float");
                        ImGui::TableSetColumnIndex(2);
                        {
                            Float Copy = Resources.GetValue<Float>(Object.Location);
                            if (Object.MinValue != 0.0f && Object.MaxValue != 0.0f)
                            {
                                if (ImGui::SliderFloat("##slider", &Copy, Object.MinValue, Object.MaxValue))
                                {
                                    Resources.SetValue<Float>(Object.Location, Copy);
                                }
                            }
                            else
                            {
                                if (ImGui::DragFloat("##drag", &Copy, 1, Object.MinValue, Object.MaxValue ))
                                {
                                    Resources.SetValue<Float>(Object.Location, Copy);
                                }
                            }
                        }
                        ImGui::TableSetColumnIndex(3);
                        if (ImGui::Button("reset"))
                        {
                            Resources.SetValue<Float>(Object.Location, Object.BaseValue);
                        }

                        ImGui::PopID();
                    }

                    for (const auto& Object : Resources.ReflectedVariables<Size2D>())
                    {
                        if (SearchBuffer[0] != '\0' && Object.FieldName.find(SearchBuffer) == std::string::npos) continue;

                        ImGui::PushID(&Object);
                        
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        // if (ImGui::Selectable(Object.FieldName.data(), , ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {}
                        ImGui::Text(Object.FieldName.data());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Size2D");
                        ImGui::TableSetColumnIndex(2);
                        {
                            Size2D Copy = Resources.GetValue<Size2D>(Object.Location);
                            if (ImGui::DragInt2("##drag", (int*)Copy.data(), 1, 0, INT_MAX))
                            {
                                Resources.SetValue<Size2D>(Object.Location, Copy);
                            }
                        }
                        ImGui::TableSetColumnIndex(3);
                        if (ImGui::Button("reset"))
                        {
                            Resources.SetValue<Size2D>(Object.Location, Object.BaseValue);
                        }

                        ImGui::PopID();
                    }

                    for (const auto& Object : Resources.ReflectedVariables<Rect>())
                    {
                        if (SearchBuffer[0] != '\0' && Object.FieldName.find(SearchBuffer) == std::string::npos) continue;

                        ImGui::PushID(&Object);
                        
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        // if (ImGui::Selectable(Object.FieldName.data(), , ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {}
                        ImGui::Text(Object.FieldName.data());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Rect");
                        ImGui::TableSetColumnIndex(2);
                        {
                            Rect Copy = Resources.GetValue<Rect>(Object.Location);
                            if (ImGui::DragInt2("position", (int*)&(Copy.Position), 1, 0, INT_MAX))
                            {
                                Resources.SetValue<Rect>(Object.Location, Copy);
                            }
                            ImGui::SameLine();
                            if (ImGui::DragInt2("size", (int*)&(Copy.Size), 1, 0, INT_MAX))
                            {
                                Resources.SetValue<Rect>(Object.Location, Copy);
                            }
                        }
                        ImGui::TableSetColumnIndex(3);
                        if (ImGui::Button("reset"))
                        {
                            Resources.SetValue<Rect>(Object.Location, Object.BaseValue);
                        }

                        ImGui::PopID();
                    }

                    for (const auto& Object : Resources.ReflectedVariables<Math::Vector3f>())
                    {
                        if (SearchBuffer[0] != '\0' && Object.FieldName.find(SearchBuffer) == std::string::npos) continue;

                        ImGui::PushID(&Object);
                        
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        // if (ImGui::Selectable(Object.FieldName.data(), , ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {}
                        ImGui::Text(Object.FieldName.data());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Vector3f");
                        ImGui::TableSetColumnIndex(2);
                        {
                            Math::Vector3f Copy = Resources.GetValue<Math::Vector3f>(Object.Location);
                            if (ImGui::DragFloat3("##check", Copy.data()))
                            {
                                Resources.SetValue<Math::Vector3f>(Object.Location, Copy);
                            }
                        }
                        ImGui::TableSetColumnIndex(3);
                        if (ImGui::Button("reset"))
                        {
                            Resources.SetValue<Math::Vector3f>(Object.Location, Object.BaseValue);
                        }

                        ImGui::PopID();
                    }

                    ImGui::EndTable();
                }

                ImGui::EndTabItem();
            }

            // Commands
            if (ImGui::BeginTabItem("Commands"))
            {
                int CurrentItem = 0;

                size_t CommandCount = 0;
                for (const auto & list : m_CommandLists) CommandCount += list.size();
                
                struct CLCTX
                {
                    std::span<const std::span<const Location>> CommandLists;
                    CommandPool& CommandPool;
                    size_t Count;
                } ctx {.CommandLists = m_CommandLists, .CommandPool = *m_CommandPool, .Count = CommandCount};
                
                auto GetName = [](void* Context, int index) -> const char*
                {
                    if (index < 0) return nullptr;

                    CLCTX* CTX = (CLCTX*)Context;

                    
                    std::span<const ctti::detail::cstring> CommandNames = CTX->CommandPool.CommandNames();

                    return CommandNames[index].begin();
                };

                ImGui::ListBox("CommandList(s)", &CurrentItem, GetName, &ctx, CommandCount);
                
                ImGui::EndTabItem();
            }

            // Debug Views
            if (ImGui::BeginTabItem("Debug Views"))
            {
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();

            ImGui::Separator();

            if (m_CommandDebugView)
            {
                m_CommandPool->CommandDebugViewEditorUI(m_CurrentDebugView);
            }
            if (m_GraphDebugView)
            {
                m_CommandPool->GraphDebugViewEditorUI(m_CurrentDebugView);
            }
        }
    }

    void Module::UnsetDebugView()
    {
        m_CommandDebugView = false;
        m_GraphDebugView = false;
        m_CurrentDebugView = std::numeric_limits<Location>::max();
        m_UpToCommandList = std::numeric_limits<Location>::max();
        m_UpToCommand = std::numeric_limits<Location>::max();
    }

    void Module::SetDebugViewTexture(Location Texture, ETextureViewer_TexType TexType, size_t Mip, size_t Depth,
        size_t Index, size_t Sample)
    {
        if (!m_CommandPool->HasGraphDebugViews()) return;
        
        TextureViewer* TextureViewerInst = m_CommandPool->GetDebugView<TextureViewer>(m_TextureViewer);
        if (TextureViewerInst == nullptr) return;
        
        m_GraphDebugView = true;
        m_CurrentDebugView = m_TextureViewer;
        SelectLatestCommands();

        m_TextureViewerTexType = TexType;
        m_TextureViewerResource = Texture;

        switch (TexType)
        {
        case T2D:
            TextureViewerInst->SetTargetTexture(m_TextureViewerResource, TextureViewer::T_Texture2D, Mip, Depth, Index, Sample);
            break;
            
        case TCube:
            TextureViewerInst->SetTargetTexture(m_TextureViewerResource, TextureViewer::T_TextureCube, Mip, Depth, Index, Sample);
            break;
            
        case T3D:
            TextureViewerInst->SetTargetTexture(m_TextureViewerResource, TextureViewer::T_Texture3D, Mip, Depth, Index, Sample);
            break;
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported texture type")
        }
    }

    void Module::SelectLatestCommands()
    {
        m_EditorCurrentCommand = -1;

        if (m_CommandLists.empty()) return;
        
        m_UpToCommandList = m_CommandLists.size() - 1;
        m_UpToCommand = m_CommandLists.back().back();
    }
}