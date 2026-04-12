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

using namespace Math;

/* ____________________________________ Constants ____________________________________ */

constexpr size_t kBaseWidth = 1280;
constexpr size_t kBaseHeight = 720;
constexpr float kZNear = 0.01f;
constexpr float kZFar = 1000.0f;

/* ____________________________________ States ____________________________________ */

bool RequestShaderReload = false;
bool RequestRebake = true;
bool RebakeEveryFrame = false;

float CameraSpeed = 1.0f;

bool UseFrustumCulling = false;

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
#ifdef CONFIG_DEBUG
        EngineRuntimeBREAKPOINT
#endif // CONFIG_DEBUG
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
    
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
        RequestShaderReload = true;
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
    
    PositionDir = PositionDir * static_cast<float>(deltaTime) * (CameraSpeed * 100) * 2.0f;
    camera.Translate(Transpose(camera.GetWorldRotation().GetRotationMatrix()) * PositionDir);
        
    if (GetKey(window, GLFW_KEY_Q))
        rotateDir += 1;
    if (GetKey(window, GLFW_KEY_E))
        rotateDir -= 1;
    
    camera.RotateRadians(0, rotateDir * Pi * deltaTime * (CameraSpeed * 1000) / 5);
}
#endif // WINDOW_GLFW

/* ____________________________________ Render Data ____________________________________ */

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

struct SceneBuffers
{
    GLTF::GPUScene Scene;
    UniformBuffer Lights;
    UniformBuffer Camera;

    TextureCube SkylightCube{0, 0, Texture::Byte, Texture::R};
    Texture2D SkylightHDRI{0, 0, Texture::Byte, Texture::R};
    
    Sampler BaseSampler{{}};
    
    // States
    uint32_t SkylightMethod = 1;
    
    SceneBuffers() = default;
};

/* ____________________________________ Helpers ____________________________________ */

bool frustumCullingTest(
    const Matrix4f &ViewProj,
    const Matrix4f &Model,
    Point3f boundMin, Point3f boundMax)
{
    //todo fix
    
    Matrix4f MVP = ViewProj * Model;
    Matrix4f InverseMVP = Inverse(MVP);
    
    static const Point3f frustum[8]= { 
        { -1, -1, -1 },
        { -1, -1, 1 },
        { -1, 1, -1 },
        { -1, 1, 1 },
        { 1, -1, -1 },
        { 1, -1, 1 },
        { 1, 1, -1 },
        { 1, 1, 1 },
    };

    Point3f bounds[8]= { 
        { boundMin.x, boundMin.y, boundMin.z },
        { boundMin.x, boundMin.y,  boundMax.z },
        { boundMin.x, boundMax.y, boundMin.z },
        { boundMin.x, boundMax.y,  boundMax.z },
        { boundMax.x, boundMin.y, boundMin.z },
        { boundMax.x, boundMin.y,  boundMax.z },
        { boundMax.x,  boundMax.y, boundMin.z },
        { boundMax.x,  boundMax.y,  boundMax.z },
    };
    
    bool areBoundsInFrustum = true;
    {
        bool validPlans[6] = {false};
        for (size_t i = 0; i < 8; i++)
        {
            Vector4f tp = MVP * Vector4f(bounds[i], 1);

            validPlans[0] |= (tp.x > -tp.w);
            validPlans[1] |= (tp.x <  tp.w);
            validPlans[2] |= (tp.y > -tp.w);
            validPlans[3] |= (tp.y <  tp.w);
            validPlans[4] |= (tp.z > -tp.w);
            validPlans[5] |= (tp.z <  tp.w);        
        }
        for (size_t i = 0; i < 6; i++)
        {
            if(!validPlans[i]) 
                areBoundsInFrustum = false;
        }
    }

    // todo fix
    bool isFrustumInBounds = true;
    {
        bool validPlans[6] = {false};
        for (size_t i = 0; i < 8; i++)
        {
            Vector4f boundPosition = InverseMVP * Vector4f(frustum[i], 1);
            boundPosition /= boundPosition.w;
    
            validPlans[0] |= (boundPosition.x >= boundMin.x);
            validPlans[1] |= (boundPosition.x <= boundMax.x);
            validPlans[2] |= (boundPosition.y >= boundMin.y);
            validPlans[3] |= (boundPosition.y <= boundMax.y);
            validPlans[4] |= (boundPosition.z >= boundMin.z);
            validPlans[5] |= (boundPosition.z <= boundMax.z);
        
        }
        for (size_t i = 0; i < 6; i++)
        {
            if(!validPlans[i]) 
                isFrustumInBounds = false;
        }
    }
    return areBoundsInFrustum && isFrustumInBounds;
}

/* ____________________________________ Baking Passes ____________________________________ */

/* ____________________________________ Real time Passes ____________________________________ */

class DrawSky
{
public:
    DrawSky() : 
        m_PipelineCubemap(PipelineFromFile("Background sky", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "SkylightToRadiance.glsl", m_PipelineCubemapDefines)),
        m_PipelineHDRI(PipelineFromFile("Background sky", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "SkylightToRadiance.glsl", m_PipelineHDRIDefines))
    {}
    
    // void Update(double DeltaTime);
    void Draw(const SceneBuffers& SceneObjects)
    {
        DebugScopeMarker scope("Draw Sky");

        switch (SceneObjects.SkylightMethod)
        {
        case 0: // Cubemap Sampling
            Bind(m_PipelineCubemap);
                
            // Scene buffers
            SetUniform(0, SceneObjects.Camera);
            SetUniform(1, SceneObjects.Lights);
            SetUniform(m_PipelineCubemap, "SkyLightCubeMap", 0, SceneObjects.SkylightCube, SceneObjects.BaseSampler);
        
            // Draw screen quad
            glDrawArrays(GL_TRIANGLES, 0, 3);
        
            UnBind(m_PipelineCubemap);
            break;
            
        case 1: // HDRI Sampling
            Bind(m_PipelineHDRI);
                
            // Scene buffers
            SetUniform(0, SceneObjects.Camera);
            SetUniform(1, SceneObjects.Lights);
            SetUniform(m_PipelineHDRI, "SkyLightHDRi", 0, SceneObjects.SkylightHDRI, SceneObjects.BaseSampler);
        
            // Draw screen quad
            glDrawArrays(GL_TRIANGLES, 0, 3);
        
            UnBind(m_PipelineHDRI);
            break;
        }
    }
    
    void Reload()
    {
        PipelineUpdateFromFile(m_PipelineCubemap, "SkylightToRadiance.glsl", m_PipelineCubemapDefines);
        PipelineUpdateFromFile(m_PipelineHDRI, "SkylightToRadiance.glsl", m_PipelineHDRIDefines);
    }

private:
    Shader::DefineArray<1> m_PipelineCubemapDefines = {Shader::Define("USE_CUBEMAP_SKYLIGHT", "")};
    Shader::DefineArray<1> m_PipelineHDRIDefines = {Shader::Define("USE_HDRI_SKYLIGHT", "")};
    
    Pipeline m_PipelineCubemap;
    Pipeline m_PipelineHDRI;
};

class DrawScene
{
public:    
    DrawScene() : 
        m_PipelineCubemap(PipelineFromFile("Mesh To Radiance", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "MeshToRadiance.glsl", m_PipelineCubemapDefines)),
        m_PipelineHDRI(PipelineFromFile("Mesh To Radiance", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "MeshToRadiance.glsl", m_PipelineHDRIDefines))
    {}
    
    // void Update(double DeltaTime);
    void Draw(const SceneBuffers& SceneObjects, const Camera& Camera)
    {
        DebugScopeMarker scope("Draw Scene");
        
        const Pipeline* pipeline = nullptr;
        Matrix4f ViewProj = Camera.Projection() * Camera.View();
        
        switch (SceneObjects.SkylightMethod)
        {
        case 0: // Cubemap Sampling
            Bind(m_PipelineCubemap);
                
            // Scene buffers
            SetUniform(0, SceneObjects.Camera);
            SetUniform(1, SceneObjects.Lights);
            SetUniform(m_PipelineCubemap, "SkyLightCubeMap", 0, SceneObjects.SkylightCube, SceneObjects.BaseSampler);
            SetUniform(m_PipelineCubemap, "SkyLightMipCount",  SceneObjects.SkylightCube.MipCount());
            
            SetUniform(m_PipelineCubemap, "IndirectLightingSampleCount", m_SkyLightSampleCount);
            
            pipeline = &m_PipelineCubemap;
            break;
            
        case 1: // HDRI Sampling
            Bind(m_PipelineHDRI);
                
            // Scene buffers
            SetUniform(0, SceneObjects.Camera);
            SetUniform(1, SceneObjects.Lights);
            SetUniform(m_PipelineHDRI, "SkyLightHDRi", 0, SceneObjects.SkylightHDRI, SceneObjects.BaseSampler);
            SetUniform(m_PipelineHDRI, "SkyLightMipCount",  SceneObjects.SkylightHDRI.MipCount());
            
            SetUniform(m_PipelineCubemap, "IndirectLightingSampleCount", m_SkyLightSampleCount);
            
            pipeline = &m_PipelineHDRI;
            break;
                        
        default:
            return;
        }
        
        for (const GLTF::MeshInstance& Instance : SceneObjects.Scene.instances)
        {
            const MeshObject& Mesh = SceneObjects.Scene.meshes[Instance.mesh];
            const Mesh::VertexGroup& Group = Mesh.GetGroups()[Instance.vertexGroup];
            const GLTF::Transform& Transform = SceneObjects.Scene.transforms[Instance.transform];
            const GLTF::Material& Material = SceneObjects.Scene.materials[Instance.material];
            
            switch (Transform.Type)
            {
            case GLTF::Transform::Properties:
                {
                    Transform4f TransformMatrix = Transform.Value.asProperties.GetTransform();
            
                    if (UseFrustumCulling && !frustumCullingTest(ViewProj, TransformMatrix, Group.BoundsMin, Group.BoundsMax)) continue;
            
                    SetUniform(*pipeline, "Model", TransformMatrix);
                }
                break;
                
            case GLTF::Transform::Matrix:
                {
                    if (UseFrustumCulling && !frustumCullingTest(ViewProj, Transform.Value.asMatrix, Group.BoundsMin, Group.BoundsMax)) continue;
            
                    SetUniform(*pipeline, "Model", Transform.Value.asMatrix);
                }
                break;
                
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
            }

            
            // Material
            SetUniform(*pipeline, "BaseColor", Material.color.XYZ());
            SetUniform(*pipeline, "Roughness", Material.roughness);
            SetUniform(*pipeline, "Metalness", Material.metallic);
            SetUniform(*pipeline, "UseColorTexture", Material.colorTexture != UINT64_MAX);
            SetUniform(*pipeline, "UseNormalTexture", Material.normalTexture != UINT64_MAX);
            SetUniform(*pipeline, "UseMRTexture", Material.metallicRoughnessTexture != UINT64_MAX);
            SetUniform(*pipeline, "UseAOTexture", Material.occlusionTexture != UINT64_MAX);
            if (Material.colorTexture != UINT64_MAX) SetUniform(*pipeline, "texColor", 1, SceneObjects.Scene.textures[Material.colorTexture], SceneObjects.BaseSampler);
            if (Material.normalTexture != UINT64_MAX) SetUniform(*pipeline, "texNormal", 2, SceneObjects.Scene.textures[Material.normalTexture], SceneObjects.BaseSampler);
            if (Material.metallicRoughnessTexture != UINT64_MAX) SetUniform(*pipeline, "texMR", 3, SceneObjects.Scene.textures[Material.metallicRoughnessTexture], SceneObjects.BaseSampler);
            if (Material.occlusionTexture != UINT64_MAX) SetUniform(*pipeline, "texAO", 4, SceneObjects.Scene.textures[Material.occlusionTexture], SceneObjects.BaseSampler);
            
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
        }
        
        UnBind(*pipeline);
    }
    
    void Reload()
    {
        PipelineUpdateFromFile(m_PipelineCubemap, "MeshToRadiance.glsl", m_PipelineCubemapDefines);
        PipelineUpdateFromFile(m_PipelineHDRI, "MeshToRadiance.glsl", m_PipelineHDRIDefines);
    }
    
    uint32_t& SkyLightSampleCount() {return m_SkyLightSampleCount;}
    
private:
    Shader::DefineArray<1> m_PipelineCubemapDefines = {Shader::Define("USE_CUBEMAP_SKYLIGHT", "")};
    Shader::DefineArray<1> m_PipelineHDRIDefines = {Shader::Define("USE_HDRI_SKYLIGHT", "")};
    
    Pipeline m_PipelineCubemap;
    Pipeline m_PipelineHDRI;
    
    uint32_t m_SkyLightSampleCount = 32;
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
        
        SetUniform(m_Pipeline, "SceneRadiance", 0, SceneRadiance, m_Sampler);
        
        // Draw screen quad
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        UnBind(m_Pipeline);
    }
    
    void Reload()
    {
        PipelineUpdateFromFile(m_Pipeline, "PostProcess.glsl");
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
    AddSearchPath(RESOURCES_SAMPLE_SCENES);

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
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepth(1.0f);
    
#ifndef CONFIG_DEBUG
    UseFrustumCulling = true;
#endif // CONFIG_DEBUG 
    
    // Application resources lifetime
    {
        uint32_t CurrentWidth = kBaseWidth, CurrentHeight = kBaseHeight;
        
        // GPU buffers data
        CameraData cameraData;
        DirectionalLight lightsData{};
        
        Texture2D SceneRadianceRT(CurrentWidth, CurrentHeight, Texture::Packed_R11F_G11F_B10F, Texture::RGB);
        Texture2D SceneDepthRT(CurrentWidth, CurrentHeight, Texture::UnsignedInt, Texture::D);
        FrameBuffer SceneRadianceFB(FrameBuffer::Attachment(SceneRadianceRT, FrameBuffer::ClearColor(0.f)), &SceneDepthRT);

        SceneBuffers GPUScene;
        
        // Load scene data
        {
            std::filesystem::path path;
            // if (GetAbsoluteFilePath(std::filesystem::path("glTF-Sample-Assets") / "Models" / "ABeautifulGame" / "glTF-Binary" /"ABeautifulGame.glb" ,path))
            // if (GetAbsoluteFilePath(std::filesystem::path("glTF-Sample-Assets") / "Models" / "MetalRoughSpheres" / "glTF-Binary" /"MetalRoughSpheres.glb" ,path))
            if (GetAbsoluteFilePath(std::filesystem::path("glTF-Sample-Assets") / "Models" / "MetalRoughSpheres" / "glTF" /"MetalRoughSpheres.gltf" ,path))
            {
                AssertOrError( GLTF::LoadGPUScene(path, GPUScene.Scene), "Failed to load scene")
            }
        }

        // Load cubemap
        {
            const std::filesystem::path folder = std::filesystem::path("Textures") / "CubeMaps" / "LearnOpenGL";
            Image Front = ImageLoad(GetAbsoluteFilePath(folder / "front.jpg"), Image::UnsignedByte);
            Image Back = ImageLoad(GetAbsoluteFilePath(folder / "back.jpg"), Image::UnsignedByte);
            Image Left = ImageLoad(GetAbsoluteFilePath(folder / "left.jpg"), Image::UnsignedByte);
            Image Right = ImageLoad(GetAbsoluteFilePath(folder / "right.jpg"), Image::UnsignedByte);
            Image Top = ImageLoad(GetAbsoluteFilePath(folder / "top.jpg"), Image::UnsignedByte);
            Image Bottom = ImageLoad(GetAbsoluteFilePath(folder / "bottom.jpg"), Image::UnsignedByte);
            
            std::array faces{
                TextureCube::FacePair(TextureCube::Front, Front),
                TextureCube::FacePair(TextureCube::Back, Back),
                TextureCube::FacePair(TextureCube::Left, Left),
                TextureCube::FacePair(TextureCube::Right, Right),
                TextureCube::FacePair(TextureCube::Up, Top),
                TextureCube::FacePair(TextureCube::Down, Bottom),
            };

            GPUScene.SkylightCube.Data(faces);
        }
        
        // Load HDRi
        {
            std::filesystem::path path;
            if (GetAbsoluteFilePath(std::filesystem::path("Textures") / "HDRi" / "san_giuseppe_bridge_4k.hdr" ,path))
            {
                GPUScene.SkylightHDRI.Data(ImageLoad(path, Image::Float));
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
                SceneDepthRT.Data(CurrentWidth, CurrentHeight);
                SceneRadianceFB.Resize(CurrentWidth, CurrentHeight);
            }
            
            // Handle shader reload
            if (RequestShaderReload)
            {
                DrawSkyPass.Reload();
                DrawScenePass.Reload();
                DrawPostProcessPass.Reload();
                
                RequestRebake = true;
                
                RequestShaderReload = false;
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
            }
            
            // Bake non real time data
            if (RequestRebake || RebakeEveryFrame)
            {
                RequestRebake = false;
            }

            glClear(GL_COLOR_BUFFER_BIT );

            // Draw scene
            {
                Bind(SceneRadianceFB);
                SceneRadianceFB.Clear();
                
                DrawSkyPass.Draw(GPUScene);
                
                glEnable(GL_CULL_FACE);
                glEnable(GL_DEPTH_TEST);
                glClear(GL_DEPTH_BUFFER_BIT);
                
                DrawScenePass.Draw(GPUScene, camera);
                glDisable(GL_CULL_FACE);
                glDisable(GL_DEPTH_TEST);
                
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
                
                // Directional Light
                ImGui::DragFloat3("Light Direction", lightsData.LightDir.vector.data(), 0.1f);
                ImGui::ColorEdit3("Light Color", lightsData.LightColor.data());
                ImGui::SliderFloat("Light Intensity", &lightsData.LightIntensity, 0.1f, 10.0f);
                
                ImGui::Separator();

                static const char* SkyLightMethodNames[] =
                {
                    "Cubemap", "HDRi"
                };
                ImGui::ListBox("Sky Light Method", (int*)&GPUScene.SkylightMethod, SkyLightMethodNames, 2);
                GPUScene.SkylightMethod = Math::Clamp(GPUScene.SkylightMethod, 0u, 1u);
                
                ImGui::SliderInt("Sky light Sample Count", (int*)&DrawScenePass.SkyLightSampleCount(), 1, 1024);

                ImGui::Separator();
                
                if (GPUScene.SkylightMethod == 3)
                {
                    ImGui::Checkbox("Rebake data every frame", &RebakeEveryFrame);
                    if (ImGui::Button("Rebake data"))
                    {
                        RequestRebake = true;
                    }
                    
                    ImGui::Separator();
                }
                
                ImGui::SliderFloat("Camera Speed", &CameraSpeed, 0.1f, 2.0f);
                ImGui::Checkbox("Use Frustum Culling", &UseFrustumCulling);
                
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