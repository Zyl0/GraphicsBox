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

/* ____________________________________ Passes ____________________________________ */

struct CameraData
{
    // Matrices
    AlignedMatrix4f     Camera_WorldToView;
    AlignedMatrix4f     Camera_WorldToProj;
    AlignedMatrix4f     Camera_ViewToWorld;
    AlignedMatrix4f     Camera_ViewToProj;
    AlignedMatrix4f     Camera_ProjToView;
    AlignedMatrix4f     Camera_ProjToWorld;
    
    // Camera properties
    AlignedVector3f     Camera_WorldPosition;
    Vector3f            Camera_WorldUp;
    float               Camera_AspectRatio;
    AlignedVector3f     Camera_WorldForward;
    AlignedVector3f     Camera_WorldRight;
    
    // Screen
    Vector2f            Camera_ProjToViewport;
    Vector2f            Camera_ViewportToProj;
};

void UpdateCameraData(CameraData& Data, const Camera& camera)
{
    Data.Camera_WorldToView    = camera.View();
    Data.Camera_ViewToProj     = camera.Projection();

    Data.Camera_ViewToWorld    = camera.InverseView();
    Data.Camera_ProjToView     = camera.InverseProjection();

    Data.Camera_WorldToProj    = camera.Projection() * camera.View();
    Data.Camera_ProjToWorld    = Inverse(camera.Projection() * camera.View());

    Data.Camera_WorldPosition  = camera.GetWorldPosition();
    Data.Camera_WorldForward   = camera.GetWorldDirection();
    Data.Camera_WorldUp        = camera.GetWorldUp();
    Data.Camera_WorldRight     = camera.GetWorldRight();
    Data.Camera_AspectRatio    = camera.GetAspectRatio();
    
    Data.Camera_ProjToViewport = Vector2f(1.0f, -1.0f); // TODO find the right one between GL and vulkan
    Data.Camera_ViewportToProj = Vector2f(1.0f, -1.0f);
}

struct DirectionalLight
{
    AlignedVector3f LightDir = AlignedVector3f(Vector3f{0.0f, -1.0f, 0.0f});
    
    Vector3f LightColor = {1.0f, 1.0f, 1.0f};
    float LightIntensity = 1.0f;
};

struct ProceduralSkylight
{
    // XYZ is color, W is scale
    Vector4f AtmosphereDayColor = {0.3f, 0.5f, 0.8f, 1.0f};

    // XYZ is color, W is scale
    Vector4f AtmosphereNightColor = {0.006f, 0.008f, 0.01f, 1.0f};

    // XYZ is color, W is scale
    Vector4f SunSetHighEnergy = {0.8f, 0.7f, 0.3f, 1.0f};

    // XYZ is color, W is scale
    Vector4f SunSetLowEnergy = {1.0f, 0.5f, 0.5f, 1.0f};

    // XYZ is color, W is scale
    Vector4f AtmosphericOcclusion = {0.9f, 0.9f, 1.0f, 1.0f};
    
    float AngleDayTime = 0.3f;
    float AngleSunSetHigh = 0.4f;
    float AngleSunSetLow = 0.5f;
    float AngleNight = 0.6f;

    Vector3f AverageGroundColor = {0.3f, 0.6f, 0.2f};
    float AverageGroundRoughness = 0.8f;

    float AverageGroundMetalness = 0.01f;
    float FogHeight = 0.2;
};

struct SceneBuffers
{
    GLTF::GPUScene Scene;
    UniformBuffer ProceduralSkyParameters;
    UniformBuffer Lights;
    UniformBuffer Camera;
    
    SceneBuffers() = default;
};

class DrawSky
{
public:    
    DrawSky() : 
        m_Pipeline(PipelineFromFile("Background sky", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "SkylightToRadiance.glsl"))
    {}
    
    // void Update(double DeltaTime);
    void Draw(const SceneBuffers& SceneObjects)
    {
        DebugScopeMarker scope("Draw Sky");
        
        Bind(m_Pipeline);
        
        // glDisable(GL_DEPTH_TEST);
        
        // Scene buffers
        SetUniform(0, SceneObjects.Camera);
        SetUniform(1, SceneObjects.Lights);
        SetUniform(2, SceneObjects.ProceduralSkyParameters);
        
        // Draw screen quad
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        UnBind(m_Pipeline);
    }
    
private:
    Pipeline m_Pipeline;
};

class DrawScene
{
public:
    struct MaterialParams
    {
        Vector3f BaseColor = {1.0f};
        float Roughness = 1.0f;
        float Metalness = 0.0f;
    };
    
    DrawScene() : 
        m_Pipeline(PipelineFromFile("Mesh To Radiance", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "MeshToRadiance.glsl"))
    {}
    
    // void Update(double DeltaTime);
    void Draw(const SceneBuffers& SceneObjects)
    {
        DebugScopeMarker scope("Draw Scene");
        
        Bind(m_Pipeline);
        
        // Scene buffers
        SetUniform(0, SceneObjects.Camera);
        SetUniform(1, SceneObjects.Lights);
        SetUniform(2, SceneObjects.ProceduralSkyParameters);
        
        SetUniform(m_Pipeline, "Model", MakeHomogeneousIdentity<float>());

        // Material
        SetUniform(m_Pipeline, "BaseColor", m_Material.BaseColor);
        SetUniform(m_Pipeline, "Roughness", m_Material.Roughness);
        SetUniform(m_Pipeline, "Metalness", m_Material.Metalness);
        
        SetUniform(m_Pipeline, "IndirectLightingSampleCount", m_SkyLightSampleCount);
        
        GLTF::MeshInstance Instance = SceneObjects.Scene.instances[0];
        const MeshObject& Mesh = SceneObjects.Scene.meshes[Instance.mesh];
        const Mesh::VertexGroup& Group = Mesh.GetGroups()[Instance.vertexGroup]; 

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
        UnBind(m_Pipeline);
    }
    
    MaterialParams& Material() { return m_Material; }
    
    uint32_t& SkyLightSampleCount() {return m_SkyLightSampleCount;}
    
private:
    Pipeline m_Pipeline;
    MaterialParams m_Material{};
    uint32_t m_SkyLightSampleCount = 64;
};

class PostProcess
{
public:

    PostProcess() : 
        m_Pipeline(PipelineFromFile("Post Process", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "PostProcess.glsl")),
        m_Sampler({
            .Magnification = Sampler::F_Nearest,
            .Minification = Sampler::F_Nearest,
        })
    {}
    
    // void Update(double DeltaTime);
    void Draw(const SceneBuffers& SceneObjects, const Texture2D& SceneRadiance)
    {
        DebugScopeMarker scope("Apply Tone Mapping");
        
        Bind(m_Pipeline);
        
        // Scene buffers
        SetUniform(0, SceneObjects.Camera);
        SetUniform(1, SceneObjects.Lights);
        SetUniform(2, SceneObjects.ProceduralSkyParameters);
        
        SetUniform(m_Pipeline, "SceneRadiance", 0, SceneRadiance, m_Sampler);
        
        // Draw screen quad
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        UnBind(m_Pipeline);
    }
    
private:
    Pipeline m_Pipeline;
    Sampler m_Sampler;
};

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
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    
    // Application resources lifetime
    {
        uint32_t CurrentWidth = kBaseWidth, CurrentHeight = kBaseHeight;
        
        // GPU buffers data
        CameraData cameraData;
        DirectionalLight lightsData{};
        ProceduralSkylight proceduralSkylightData{};
        
        Texture2D SceneRadianceRT(CurrentWidth, CurrentHeight, Texture::Packed_R11F_G11F_B10F, Texture::RGB);
        FrameBuffer SceneRadianceFB(FrameBuffer::ExternalAttachment(SceneRadianceRT, FrameBuffer::ClearColor(0.f)));
        
        SceneBuffers GPUScene;
        {
            std::filesystem::path path;
            if (GetAbsoluteFilePath(std::filesystem::path("Meshes") / "Sphere.glb" ,path))
            {
                AssertOrError( GLTF::LoadGPUScene(path, GPUScene.Scene), "Failed to load scene")
            }
        }

        FlyCamera camera;
        camera.SetProjection(CurrentWidth, CurrentHeight, Math::Radians(45.0f), kZNear, kZFar);
        camera.SetTranslation(-4,0,0);
        
        // Passes
        DrawSky DrawSkyPass{};
        DrawScene DrawScenePass{};
        PostProcess DrawPostProcessPass{};

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
                
                SceneRadianceRT.Data(CurrentWidth, CurrentHeight);
                SceneRadianceFB.Resize(CurrentWidth, CurrentHeight);
            }

            // Update scene
            {
                curr_clock = clock();
                clock_t dcl = curr_clock - prev_clock;
                double deltaTime = static_cast<double>(dcl) / 1000000.0;
                prev_clock = curr_clock;
                
                UpdateCamera(window, deltaTime, camera);
                UpdateCameraData(cameraData, camera);
                
                GPUScene.Camera.Data(&cameraData, sizeof(cameraData));
                GPUScene.Lights.Data(&lightsData, sizeof(lightsData));
                GPUScene.ProceduralSkyParameters.Data(&proceduralSkylightData, sizeof(proceduralSkylightData));
            }

            glClear(GL_COLOR_BUFFER_BIT );

            // Draw scene
            {
                Bind(SceneRadianceFB);
                SceneRadianceFB.Clear();
                
                DrawSkyPass.Draw(GPUScene);
                
                glEnable(GL_CULL_FACE);
                DrawScenePass.Draw(GPUScene);
                glDisable(GL_CULL_FACE);
                
                UnBind(SceneRadianceFB);
                
                
                DrawPostProcessPass.Draw(GPUScene, SceneRadianceRT);
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
                ImGui::ColorEdit3("Surface Color", DrawScenePass.Material().BaseColor.data());
                ImGui::SliderFloat("Surface Roughness", &DrawScenePass.Material().Roughness, 0.0f, 1.0f);
                ImGui::SliderFloat("Surface Metalness", &DrawScenePass.Material().Metalness, 0.0f, 1.0f);
                
                ImGui::Separator();

                // Directional Light
                ImGui::DragFloat3("Light Direction", lightsData.LightDir.vector.data(), 0.1f);
                ImGui::ColorEdit3("Light Color", lightsData.LightColor.data());
                ImGui::SliderFloat("Light Intensity", &lightsData.LightIntensity, 0.1f, 10.0f);
                
                ImGui::Separator();
                
                // Procedural sky Light
                ImGui::ColorEdit4("Atmosphere Day Color", proceduralSkylightData.AtmosphereDayColor.data());
                ImGui::ColorEdit4("Atmosphere Night Color", proceduralSkylightData.AtmosphereNightColor.data());
                ImGui::ColorEdit4("Atmosphere Sunset High Color", proceduralSkylightData.SunSetHighEnergy.data());
                ImGui::ColorEdit4("Atmosphere Sunset Low Color", proceduralSkylightData.SunSetLowEnergy.data());
                ImGui::ColorEdit4("Atmosphere Occlusion Color", proceduralSkylightData.AtmosphericOcclusion.data());
                ImGui::SliderFloat("Atmosphere Angle Day", &proceduralSkylightData.AngleDayTime, 0, 1);
                ImGui::SliderFloat("Atmosphere Angle Sunset High", &proceduralSkylightData.AngleSunSetHigh, 0, 1);
                ImGui::SliderFloat("Atmosphere Angle Sunset Low", &proceduralSkylightData.AngleSunSetLow, 0, 1);
                ImGui::SliderFloat("Atmosphere Angle Night", &proceduralSkylightData.AngleNight, 0, 1);
                
                ImGui::ColorEdit3("Ground Color", proceduralSkylightData.AverageGroundColor.data());
                ImGui::SliderFloat("Ground Roughness", &proceduralSkylightData.AverageGroundRoughness, 0, 1);
                ImGui::SliderFloat("Ground Metalness", &proceduralSkylightData.AverageGroundMetalness, 0, 1);
                
                ImGui::SliderFloat("Atmosphere Occlusion Fog Height", &proceduralSkylightData.FogHeight, 0, 0.5);
                
                ImGui::Separator();
                
                ImGui::SliderInt("Sky light Sample Count", (int*)&DrawScenePass.SkyLightSampleCount(), 1, 1024);
                
                ImGui::End();

                ImGui::Render();
                ImGui::EndFrame();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                
                proceduralSkylightData.AngleDayTime = std::min(proceduralSkylightData.AngleDayTime, proceduralSkylightData.AngleSunSetHigh);
                proceduralSkylightData.AngleSunSetHigh = std::min(proceduralSkylightData.AngleSunSetHigh, proceduralSkylightData.AngleSunSetLow);
                proceduralSkylightData.AngleSunSetLow = std::min(proceduralSkylightData.AngleSunSetLow, proceduralSkylightData.AngleNight);
                proceduralSkylightData.AngleNight = std::min(proceduralSkylightData.AngleNight, 1.f);
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