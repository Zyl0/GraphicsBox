#include "Shared/Assertion.h"
#include "Math/RMath.h"
#include "Files/Files.h"
#include "Modeling/Mesh.h"
#include "Rendering/Rendering.h"
#include "Importers/GLTF/SceneLoader.h"
#include "Image/ColorSpaces.h"

#include <imgui.h>

// Implementation using the mini engine
// #define USE_MINI_ENGINE

#ifdef USE_MINI_ENGINE
#else // USE_MINI_ENGINE

#include "backends/imgui_impl_opengl3.h"
#include "Camera/FlyCamera.h"

#ifdef WINDOW_GLFW
#include <GLFW/glfw3.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_glfw.h"
#endif // WINDOW_GLFW

constexpr size_t kBaseWidth = 1280;
constexpr size_t kBaseHeight = 720;
constexpr float kZNear = 0.01f;
constexpr float kZFar = 1000.0f;

using namespace Math;

/* ____________________________________ Debug ____________________________________ */

void GLAPIENTRY MessageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
    // Ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    const char* NullString = "";
    const char* errSource = NullString;
    const char* errType = NullString;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             errSource = "Source: API";                 break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   errSource = "Source: Window System";       break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: errSource = "Source: Shader Compiler";     break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     errSource = "Source: Third Party";         break;
    case GL_DEBUG_SOURCE_APPLICATION:     errSource = "Source: Application";         break;
    case GL_DEBUG_SOURCE_OTHER:           errSource = "Source: Other";               break;
    }

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               errType = "Type: Error";                 break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: errType = "Type: Deprecated Behaviour";  break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  errType = "Type: Undefined Behaviour";   break;
    case GL_DEBUG_TYPE_PORTABILITY:         errType = "Type: Portability";           break;
    case GL_DEBUG_TYPE_PERFORMANCE:         errType = "Type: Performance";           break;
    case GL_DEBUG_TYPE_MARKER:              errType = "Type: Marker";                break;
    case GL_DEBUG_TYPE_OTHER:               errType = "Type: Other";                 break;

    case GL_DEBUG_TYPE_PUSH_GROUP:
    case GL_DEBUG_TYPE_POP_GROUP:
        return;
    }

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        EngineLoggerErrorF("OpenGL Validation Error High [%s] [%s]: %s", errSource, errType, message);
        EngineRuntimeBREAKPOINT
        break;

    case GL_DEBUG_SEVERITY_MEDIUM:
        EngineLoggerErrorF("OpenGL Validation Error Medium [%s] [%s]: %s", errSource, errType, message);
        break;

    case GL_DEBUG_SEVERITY_LOW:
        EngineLoggerErrorF("OpenGL Validation Error Low [%s] [%s]: %s", errSource, errType, message);
        break;

    case GL_DEBUG_SEVERITY_NOTIFICATION:
        EngineLoggerErrorF("OpenGL Validation Notification [%s] [%s]: %s", errSource, errType, message);
        break;
    }
}

/* ____________________________________ Window ____________________________________ */

#ifdef WINDOW_GLFW
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

bool GetKey(GLFWwindow* window, unsigned code)
{
    int state = glfwGetKey(window, code);
    
    if (state == GLFW_PRESS)
    {
        return true;
    }
    
    return false;
}

void UpdateCamera(GLFWwindow* window, double deltaTime, FlyCamera& camera)
{
    Vector3f PositionDir(0, 0, 0);
    float rotateDir = 0.0f;
    const float speed = 1000.0f;

    if (GetKey(window, GLFW_KEY_LEFT_SHIFT))
        PositionDir.y += 1;
    if (GetKey(window, GLFW_KEY_LEFT_CONTROL))
        PositionDir.y -= 1;
    if (GetKey(window, GLFW_KEY_W))
        PositionDir.x += 1;
    if (GetKey(window, GLFW_KEY_S))
        PositionDir.x -= 1;
    if (GetKey(window, GLFW_KEY_A))
        PositionDir.z += 1;
    if (GetKey(window, GLFW_KEY_D))
        PositionDir.z -= 1;
    
    PositionDir = PositionDir * static_cast<float>(deltaTime) * speed * 2.0f;
    camera.Translate(Transpose(camera.GetWorldRotation().GetRotationMatrix()) * PositionDir);
        
    if (GetKey(window, GLFW_KEY_Q))
        rotateDir += 1;
    if (GetKey(window, GLFW_KEY_E))
        rotateDir -= 1;
    
    camera.RotateRadians(0, rotateDir * Pi * deltaTime * speed / 5);
}
#endif // WINDOW_GLFW

/* ____________________________________ Process ____________________________________ */

int main(void)
{
    int RC = EXIT_SUCCESS; // Ok

#ifdef WINDOW_GLFW
    GLFWwindow* window = nullptr;

    AddSearchPath(RESOURCES_GLOBAL);
    AddSearchPath(RESOURCES_PROJECT);

    ShaderAddSearchPath(SHADERS_GLOBAL);
    ShaderAddSearchPath(SHADERS_PROJECT);
    
    glfwSetErrorCallback(error_callback);
    AssertOrErrorCall(glfwInit(), RC = EXIT_FAILURE; goto terminate_main, "Failed to initialise GLFW")
    

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    window = glfwCreateWindow(kBaseWidth, kBaseHeight, "ColorBox", nullptr, nullptr);
    AssertOrErrorCall(window, RC = EXIT_FAILURE; goto terminate_glfw_window, "Failed to create GLFW window")
    EngineLoggerLog("Initialized GLFW window");
    
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
#endif // WINDOW_GLFW

    AssertOrErrorCall(glewInit() == GLEW_OK, RC = EXIT_FAILURE; goto terminate_context, "Failed to initialize GLEW")

    EngineLoggerLog("Initialized GLEW");
    EngineLoggerLogF("OpenGL Version: %s", glGetString(GL_VERSION));
    EngineLoggerLogF("GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    EngineLoggerLogF("Vendor: %s", glGetString(GL_VENDOR));
    EngineLoggerLogF("Renderer: %s", glGetString(GL_RENDERER));
    
#ifdef CONFIG_DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // Makes sure errors are displayed synchronously
    glDebugMessageCallback(MessageCallback, 0);
#endif // CONFIG_DEBUG

    ImGui::CreateContext();
#ifdef WINDOW_GLFW
    AssertOrErrorCall(ImGui_ImplGlfw_InitForOpenGL(window, true), RC = EXIT_FAILURE; goto terminate_glfw_ui, "Could not initialize ImGUI")
#endif // WINDOW_GLFW
    AssertOrErrorCall(ImGui_ImplOpenGL3_Init("#version 430"), RC = EXIT_FAILURE; goto terminate_ui, "Could not initialize ImGUI")
    
    // Application resources lifetime
    {
        uint32_t CurrentWidth = kBaseWidth, CurrentHeight = kBaseHeight;

        std::string sRGBShaderCode = ShaderFileToString("MeshTosRGBRadiance.glsl");
        
        Shader sRGBMeshToRadianceVS(Shader::VERTEX_SHADER, sRGBShaderCode);
        Shader sRGBMeshToRadianceFS(Shader::FRAGMENT_SHADER, sRGBShaderCode);

        std::array sRGBMeshToRadianceShaders
        {
            Pipeline::ShaderPair{Shader::VERTEX_SHADER, sRGBMeshToRadianceVS},
            Pipeline::ShaderPair{Shader::FRAGMENT_SHADER, sRGBMeshToRadianceFS},
        };
        Pipeline sRGBMeshToRadiance(sRGBMeshToRadianceShaders, "sRGBMeshToRadiance");
        
        std::string SpectralSlicedShaderCode = ShaderFileToString("MeshToRadianceSpectralSlices.glsl");
        
        Shader SpectralSlicedMeshToRadianceVS(Shader::VERTEX_SHADER, SpectralSlicedShaderCode);
        Shader SpectralSlicedMeshToRadianceFS(Shader::FRAGMENT_SHADER, SpectralSlicedShaderCode);

        std::array SpectralSlicedMeshToRadianceShaders
        {
            Pipeline::ShaderPair{Shader::VERTEX_SHADER, SpectralSlicedMeshToRadianceVS},
            Pipeline::ShaderPair{Shader::FRAGMENT_SHADER, SpectralSlicedMeshToRadianceFS},
        };
        Pipeline SpectralSlicedMeshToRadiance(SpectralSlicedMeshToRadianceShaders, "SpectralSlicedMeshToRadiance");
        
        GLTF::GPUScene Scene;
        {
            std::filesystem::path path;
            if (GetAbsoluteFilePath(std::filesystem::path("Meshes") / "Sphere.glb" ,path))
            // if(engine::files::GetAbsoluteFilePath(std::filesystem::path("Scenes") / "BistroGLTF" / "exterior.glb" ,path))
            {
                AssertOrError( GLTF::LoadGPUScene(path, Scene), "Failed to load scene")
            }
        }

        FlyCamera camera;
        camera.SetProjection(CurrentWidth, CurrentHeight, Math::Radians(45.0f), kZNear, kZFar);
        camera.SetTranslation(-4,0,0);

        // Material Properties
        Math::Vector3f BaseColorRGB = {1.0f};
        float BaseColorSpectralSlices[16] = {1.0f};
        float Roughness = 1.0f;
        float Metalness = 0.0f;

        // Directional Light
        Math::Vector3f LightDir = {0.0f, -1.0f, 0.0f};
        Math::Vector3f LightColorRGB = {1.0f, 1.0f, 1.0f};
        float LightColorSpectralSliced[16] = {1.0f};
        float LightIntensity = 1.0f;
        
        // Sliced spectral rendering
        uint32_t SampleCount = 8;
        Math::Vector4f CurrentBaseColorSpectralSlices[4] = {1.0f};
        Math::Vector4f CurrentLightColorSpectralSliced[4] = {1.0f};
        
        // Method
        int Mode = 0;
        
        for (int i = 0; i < 16; ++i)
        {
            BaseColorSpectralSlices[i] = BaseColorSpectralSlices[0];
            LightColorSpectralSliced[i] = LightColorSpectralSliced[0];
        }

        // keep track of time during the execution
        clock_t prev_clock = clock();
        clock_t curr_clock;
        
#ifdef WINDOW_GLFW
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
#endif // WINDOW_GLFW

            // Handle resizing
            if (CurrentWidth != width || CurrentHeight != height)
            {
                const float ratio = width / (float)height;
                CurrentWidth = width; CurrentHeight = height;

                glViewport(0, 0, CurrentWidth, CurrentHeight);

                camera.SetProjection(CurrentWidth, CurrentHeight, Math::Radians(45.0f), kZNear, kZFar);
            }

            // Update scene
            {
                curr_clock = clock();
                clock_t dcl = curr_clock - prev_clock;
                double deltaTime = static_cast<double>(dcl) / 1000000.0;
                prev_clock = curr_clock;
                
                UpdateCamera(window, deltaTime, camera);
            }

            glClear(GL_COLOR_BUFFER_BIT );

            // Draw scene sRGB
            if (Mode == 0)
            {
                DebugScopeMarker scope("Draw Radiance sRGB");
                
                GLTF::MeshInstance Instance = Scene.instances[0];
                MeshObject& Mesh = Scene.meshes[Instance.mesh];
                Mesh::VertexGroup Group = Mesh.GetGroups()[Instance.vertexGroup]; 
                
                Bind(sRGBMeshToRadiance);

                // Vertex Shader data
                SetUniform(sRGBMeshToRadiance, "ViewProjection", camera.Projection() * camera.View());
                // SetUniform(sRGBMeshToRadiance, "InverseViewProjection", camera.InverseView() * camera.InverseProjection());
                SetUniform(sRGBMeshToRadiance, "Model", MakeHomogeneousIdentity<float>());
                // SetUniform(sRGBMeshToRadiance, "InverseModel", MakeHomogeneousIdentity<float>());

                // Material
                SetUniform(sRGBMeshToRadiance, "BaseColorRGB", BaseColorRGB);
                SetUniform(sRGBMeshToRadiance, "Roughness", Roughness);
                SetUniform(sRGBMeshToRadiance, "Metalness", Metalness);

                // Directional Light
                SetUniform(sRGBMeshToRadiance, "LightDir", Normalize(LightDir));
                SetUniform(sRGBMeshToRadiance, "LightColorRGB", LightColorRGB);
                SetUniform(sRGBMeshToRadiance, "LightIntensity", LightIntensity);

                SetUniform(sRGBMeshToRadiance, "CameraPosition", camera.GetWorldPosition());

                Bind(Mesh.GetVAO());
                
                if (Mesh.GetIndexBuffer().has_value())
                {
                    const IndexBuffer& indexBuffer = Mesh.GetIndexBuffer().value();
                    
                    Bind(indexBuffer);
                    glDrawElements(ToGLGeometryType(Mesh.GetVertexType()), Group.VertexCount, ToGLIndexType(indexBuffer.GetIndexType()), (void*)(Group.FirstVertex * ToGLIndexSize(indexBuffer.GetIndexType())));
                    UnBind(indexBuffer);
                }
                else
                {
                    glDrawArrays(ToGLGeometryType(Mesh.GetVertexType()), Group.FirstVertex, Group.VertexCount);
                }
                
                UnBind(Mesh.GetVAO());
                
                UnBind(sRGBMeshToRadiance);
            }
            
            // Spectral Rendering Slices
            if (Mode == 1)
            {
                DebugScopeMarker scope("Draw Radiance - Spectral Slices");
                
                GLTF::MeshInstance Instance = Scene.instances[0];
                MeshObject& Mesh = Scene.meshes[Instance.mesh];
                Mesh::VertexGroup Group = Mesh.GetGroups()[Instance.vertexGroup]; 
                
                for (size_t i = 0; i < 3; i++)
                {
                    CurrentBaseColorSpectralSlices[i].x = BaseColorSpectralSlices[i * 4 + 0];
                    CurrentBaseColorSpectralSlices[i].y = BaseColorSpectralSlices[i * 4 + 1];
                    CurrentBaseColorSpectralSlices[i].z = BaseColorSpectralSlices[i * 4 + 2];
                    CurrentBaseColorSpectralSlices[i].w = BaseColorSpectralSlices[i * 4 + 3];
                    CurrentLightColorSpectralSliced[i].x = LightColorSpectralSliced[i * 4 + 0];
                    CurrentLightColorSpectralSliced[i].y = LightColorSpectralSliced[i * 4 + 1];
                    CurrentLightColorSpectralSliced[i].z = LightColorSpectralSliced[i * 4 + 2];
                    CurrentLightColorSpectralSliced[i].w = LightColorSpectralSliced[i * 4 + 3];
                }
                
                Bind(SpectralSlicedMeshToRadiance);

                // Vertex Shader data
                SetUniform(SpectralSlicedMeshToRadiance, "ViewProjection", camera.Projection() * camera.View());
                SetUniform(SpectralSlicedMeshToRadiance, "Model", MakeHomogeneousIdentity<float>());

                // Material
                SetUniform(SpectralSlicedMeshToRadiance, "BaseColorPack0", CurrentBaseColorSpectralSlices[0]);
                SetUniform(SpectralSlicedMeshToRadiance, "BaseColorPack1", CurrentBaseColorSpectralSlices[1]);
                SetUniform(SpectralSlicedMeshToRadiance, "BaseColorPack2", CurrentBaseColorSpectralSlices[2]);
                SetUniform(SpectralSlicedMeshToRadiance, "BaseColorPack3", CurrentBaseColorSpectralSlices[3]);
                SetUniform(SpectralSlicedMeshToRadiance, "Roughness", Roughness);
                SetUniform(SpectralSlicedMeshToRadiance, "Metalness", Metalness);

                // Directional Light
                SetUniform(SpectralSlicedMeshToRadiance, "LightDir", Normalize(LightDir));
                SetUniform(SpectralSlicedMeshToRadiance, "LightColorPack0", CurrentLightColorSpectralSliced[0]);
                SetUniform(SpectralSlicedMeshToRadiance, "LightColorPack1", CurrentLightColorSpectralSliced[1]);
                SetUniform(SpectralSlicedMeshToRadiance, "LightColorPack2", CurrentLightColorSpectralSliced[2]);
                SetUniform(SpectralSlicedMeshToRadiance, "LightColorPack3", CurrentLightColorSpectralSliced[3]);
                SetUniform(SpectralSlicedMeshToRadiance, "LightIntensity", LightIntensity);

                SetUniform(SpectralSlicedMeshToRadiance, "CameraPosition", camera.GetWorldPosition());
                
                SetUniform(SpectralSlicedMeshToRadiance, "SampleCount", SampleCount);
                SetUniform(SpectralSlicedMeshToRadiance, "XYZToRec709sRGB", Rec709::FromXYZ());

                Bind(Mesh.GetVAO());
                
                if (Mesh.GetIndexBuffer().has_value())
                {
                    const IndexBuffer& indexBuffer = Mesh.GetIndexBuffer().value();
                    
                    Bind(indexBuffer);
                    glDrawElements(ToGLGeometryType(Mesh.GetVertexType()), Group.VertexCount, ToGLIndexType(indexBuffer.GetIndexType()), (void*)(Group.FirstVertex * ToGLIndexSize(indexBuffer.GetIndexType())));
                    UnBind(indexBuffer);
                }
                else
                {
                    glDrawArrays(ToGLGeometryType(Mesh.GetVertexType()), Group.FirstVertex, Group.VertexCount);
                }
                
                UnBind(Mesh.GetVAO());
                
                UnBind(SpectralSlicedMeshToRadiance);
            }

            // Draw UI
            {
#ifdef WINDOW_GLFW
                ImGui_ImplGlfw_NewFrame();
#endif // WINDOW_GLFW
#ifdef WINDOW_SDL3
                ImGui_ImplSDL3_NewFrame();
#endif // WINDOW_SDL3
                ImGui_ImplOpenGL3_NewFrame();
                ImGui::NewFrame();

                ImGui::Begin("Settings");
                
                static const char* MethodsNames[2] = {"sRGB", "Spectral - Slices"};
                ImGui::ListBox("Method", &Mode, MethodsNames, 2);
                
                ImGui::Separator();
                
                if (Mode == 1)
                {
                    ImGui::SliderInt("SampleCount", (int*)&SampleCount, 1, 16);
                    
                    // TODO
                    // static const char* SliceMethodsNames[2] = {"From EQ", "From sRGB"}; // TODO
                    // ImGui::ListBox("Colors From", &Mode, MethodsNames, 2);
                    
                    ImGui::Separator();
                }
                
                // Material
                if (Mode == 0)
                {
                    ImGui::ColorEdit3("Surface Color RGB", BaseColorRGB.data());
                }
                if (Mode == 1)
                {
                    static std::string names[32]{};
                    for (size_t i = 0; i < SampleCount; i++)
                    {
                        const size_t Range = (760 - 400) / SampleCount;
                        const size_t Offset = 400;
                        
                        names[i].clear();
                        ImGui::SliderFloat(names[i].append("c ").append(std::to_string(i * Range + Offset)).append(" nm").c_str(), &BaseColorSpectralSlices[i], 0.0f, 1.0f);
                        
                        ImGui::SameLine();

                        // Color preview
                        Vector4f preview = Rec709::ToXYZ() * Vector4f(Spectral::WavelengthToXYZ((float)(i * Range + Offset)), 1.0f);
                        ImVec4 color = ImVec4(preview.x, preview.y, preview.z, 1.0f);
                        ImGui::ColorButton(names[16 + i].append("##c ").append(std::to_string(i * Range + Offset)).append(" nm").c_str(), color, ImGuiColorEditFlags_NoTooltip, ImVec2(40, 20));
                    }
                }
                ImGui::SliderFloat("Surface Roughness", &Roughness, 0.0f, 1.0f);
                ImGui::SliderFloat("Surface Metalness", &Metalness, 0.0f, 1.0f);
                
                ImGui::Separator();

                // Directional Light
                ImGui::DragFloat3("Light Direction", LightDir.data(), 0.1);
                if (Mode == 0)
                {
                    ImGui::ColorEdit3("Light Color RGB", LightColorRGB.data());
                }
                if (Mode == 1)
                {
                    static std::string names[32]{};
                    for (size_t i = 0; i < SampleCount; i++)
                    {
                        const size_t Range = (760 - 400) / SampleCount;
                        const size_t Offset = 400;
                        
                        names[i].clear();
                        ImGui::SliderFloat(names[i].append("l ").append(std::to_string(i * Range + Offset)).append(" nm").c_str(), &LightColorSpectralSliced[i], 0.0f, 1.0f);
                        
                        ImGui::SameLine();
                        
                        // Color preview
                        Vector4f preview = Rec709::ToXYZ() * Vector4f(Spectral::WavelengthToXYZ((float)(i * Range + Offset)), 1.0f);
                        ImVec4 color = ImVec4(preview.x, preview.y, preview.z, 1.0f);
                        ImGui::ColorButton(names[16 + i].append("##l ").append(std::to_string(i * Range + Offset)).append(" nm").c_str(), color, ImGuiColorEditFlags_NoTooltip, ImVec2(40, 20));
                    }
                }
                ImGui::SliderFloat("Light Intensity", &LightIntensity, 0.1f, 10.0f);
                ImGui::End();

                ImGui::Render();
                ImGui::EndFrame();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }

#ifdef WINDOW_GLFW
            glfwSwapBuffers(window);
#endif // WINDOW_GLFW
        }
    }
    
terminate_ui:
    ImGui_ImplOpenGL3_Shutdown();
#ifdef WINDOW_GLFW
terminate_glfw_ui:
    ImGui_ImplGlfw_Shutdown();
#endif // WINDOW_GLFW
    
    ImGui::DestroyContext();

terminate_context:
#ifdef WINDOW_GLFW
    glfwDestroyWindow(window);
    
terminate_glfw_window:
    glfwTerminate();
#endif // WINDOW_GLFW
    
terminate_main:
    return RC;
}

#endif // !USE_MINI_ENGINE