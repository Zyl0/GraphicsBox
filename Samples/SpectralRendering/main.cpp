#include "Shared/Assertion.h"
#include "Math/RMath.h"
#include "Files/Files.h"
#include "Modeling/Mesh.h"
#include "Rendering/Rendering.h"
#include "Importers/GLTF/SceneLoader.h"

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
        camera.SetProjection(CurrentWidth, CurrentHeight, Math::Radians(45.0f));

        // Material Properties
        Math::Vector3f BaseColorRGB = {1.0f};
        float Roughness = 1.0f;
        float Metalness = 0.0f;

        // Directional Light
        Math::Vector3f LightDir = {0.0f, -1.0f, 0.0f};
        Math::Vector3f LightColorRGB = {1.0f, 1.0f, 1.0f};
        float LightIntensity = 1.0f;

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

                camera.SetProjection(CurrentWidth, CurrentHeight, Math::Radians(45.0f));
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

            // Draw scene
            {
                Bind(sRGBMeshToRadiance);

                // Vertex Shader data
                SetUniform(sRGBMeshToRadiance, "ViewProjection", camera.Projection() * camera.View());
                SetUniform(sRGBMeshToRadiance, "InverseViewProjection", camera.InverseView() * camera.InverseProjection());
                SetUniform(sRGBMeshToRadiance, "Model", MakeHomogeneousIdentity<float>());
                SetUniform(sRGBMeshToRadiance, "InverseModel", MakeHomogeneousIdentity<float>());

                // Material
                SetUniform(sRGBMeshToRadiance, "BaseColorRGB", BaseColorRGB);
                SetUniform(sRGBMeshToRadiance, "Roughness", Roughness);
                SetUniform(sRGBMeshToRadiance, "Metalness", Metalness);

                // Directional Light
                SetUniform(sRGBMeshToRadiance, "LightDir", Normalize(LightDir));
                SetUniform(sRGBMeshToRadiance, "LightColor", LightColorRGB);
                SetUniform(sRGBMeshToRadiance, "LightIntensity", LightIntensity);

                SetUniform(sRGBMeshToRadiance, "CameraPosition", camera.GetWorldPosition());

                UnBind(sRGBMeshToRadiance);
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
                
                // Material
                ImGui::ColorEdit3("Surface Color RGB", BaseColorRGB.data());
                ImGui::SliderFloat("Surface Roughness", &Roughness, 0.0f, 1.0f);
                ImGui::SliderFloat("Surface Metalness", &Metalness, 0.0f, 1.0f);

                // Directional Light
                ImGui::DragFloat3("Light Direction", LightDir.data());
                ImGui::ColorEdit3("Light Color RGB", LightColorRGB.data());
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