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
    UniformBuffer Lights;
    UniformBuffer Camera;

    UniformBuffer ProceduralSkyParameters;
    TextureCube SkylightCube{0, 0, Texture::Byte, Texture::R};
    Texture2D SkylightHDRI{0, 0, Texture::Byte, Texture::R};
    
    TextureCube BakedSkylightCube{2048, 2048, Texture::Packed_R11F_G11F_B10F, Texture::RGB, true};
    
    Sampler BaseSampler{{}};
    
    // States
    uint32_t SkylightMethod = 3;
    
    
    SceneBuffers() = default;
};

/* ____________________________________ Helpers ____________________________________ */

void BakingRenderToCubeMap(FrameBuffer& framebuffer, const Pipeline& pipeline, const TextureCube& TargetCubeMap, uint8_t targetMip, bool GenerateMips)
{
    Bind(pipeline);
    
    FlyCamera Camera;
    Camera.SetProjection(1, Math::Radians(50.f));
    Camera.SetTranslation(Math::Vector3f{0.0f, 0.0f, 0.0f});
    
    GLint location = GetUniformLocation(pipeline, "CameraProjToWorld");
    AssertOrErrorCall(location >= 0, return;, "Cannot upload transform matrices")
      
    // Right, +x
    {
        framebuffer.Retarget(FrameBuffer::RetargetAttachment(TargetCubeMap, TextureCube::Right, targetMip));
        
        Camera.SetRotationDegrees( 0, -90);
        SetUniform(location, Inverse(Camera.Projection() * Camera.View()));
        Bind(framebuffer);
            
        // Draw screen quad
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    
    // Left, -x
    {
        framebuffer.Retarget(FrameBuffer::RetargetAttachment(TargetCubeMap, TextureCube::Left, targetMip));
        
        Camera.SetRotationDegrees( 0, 90);
        SetUniform(location, Inverse(Camera.Projection() * Camera.View()));
        Bind(framebuffer);
            
        // Draw screen quad
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    
    // Up, +y
    {
        framebuffer.Retarget(FrameBuffer::RetargetAttachment(TargetCubeMap, TextureCube::Up, targetMip));
        
        Camera.SetRotationDegrees( 90, 180);
        SetUniform(location, Inverse(Camera.Projection() * Camera.View()));
        Bind(framebuffer);
            
        // Draw screen quad
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    
    // Down, -y
    {
        framebuffer.Retarget(FrameBuffer::RetargetAttachment(TargetCubeMap, TextureCube::Down, targetMip));
        
        Camera.SetRotationDegrees( -90, 180);
        SetUniform(location, Inverse(Camera.Projection() * Camera.View()));
        Bind(framebuffer);
            
        // Draw screen quad
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    
    // Back, +z
    {
        framebuffer.Retarget(FrameBuffer::RetargetAttachment(TargetCubeMap, TextureCube::Back, targetMip));
        
        Camera.SetRotationDegrees(0, 0);
        SetUniform(location, Inverse(Camera.Projection() * Camera.View()));
        Bind(framebuffer);
            
        // Draw screen quad
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    
    // Front, -z
    {
        framebuffer.Retarget(FrameBuffer::RetargetAttachment(TargetCubeMap, TextureCube::Front, targetMip));
        
        Camera.SetRotationDegrees(0, 180);
        SetUniform(location, Inverse(Camera.Projection() * Camera.View()));
        Bind(framebuffer);
            
        // Draw screen quad
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    
    UnBind(pipeline);
    
    if (GenerateMips && TargetCubeMap.HasMips())
    {
        Bind(TargetCubeMap);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        UnBind(TargetCubeMap);
    }
}

/* ____________________________________ Baking Passes ____________________________________ */

class HDRiToCubemap
{
public:
    HDRiToCubemap(const SceneBuffers& SceneObjects):
        m_Pipeline(PipelineFromFile("Baking HDRiToCubeMap", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "HDRIToCubeMap.glsl")),
        m_FrameBuffer(FrameBuffer::Attachment(SceneObjects.BakedSkylightCube.Width(), SceneObjects.BakedSkylightCube.Height(), 1u, FrameBuffer::ClearColor{0.0f}))
    {}
    
    void Draw(const SceneBuffers& SceneObjects)
    {
        DebugScopeMarker scope ("HDRi texture to Cubemap");
        
        Bind(m_Pipeline);
        SetUniform(m_Pipeline, "SkyLightHDRi", 0, SceneObjects.SkylightHDRI, SceneObjects.BaseSampler);
        
        BakingRenderToCubeMap(m_FrameBuffer, m_Pipeline, SceneObjects.BakedSkylightCube, 0, true);
        
        UnBind(m_Pipeline);
    }
    
    void Reload()
    {
        PipelineUpdateFromFile(m_Pipeline, "HDRIToCubeMap.glsl");
    }
    
private:
    Pipeline m_Pipeline;
    FrameBuffer m_FrameBuffer;
};

/* ____________________________________ Real time Passes ____________________________________ */

class DrawSky
{
public:
    DrawSky() : 
        m_PipelineProcedural(PipelineFromFile("Background sky", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "SkylightToRadiance.glsl", m_PipelineProceduralDefines)),
        m_PipelineCubemap(PipelineFromFile("Background sky", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "SkylightToRadiance.glsl", m_PipelineCubemapDefines)),
        m_PipelineHDRI(PipelineFromFile("Background sky", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "SkylightToRadiance.glsl", m_PipelineHDRIDefines))
    {}
    
    // void Update(double DeltaTime);
    void Draw(const SceneBuffers& SceneObjects)
    {
        DebugScopeMarker scope("Draw Sky");

        switch (SceneObjects.SkylightMethod)
        {
        case 0: // Procedural
            Bind(m_PipelineProcedural);
                
            // Scene buffers
            SetUniform(0, SceneObjects.Camera);
            SetUniform(1, SceneObjects.Lights);
            SetUniform(2, SceneObjects.ProceduralSkyParameters);
        
            // Draw screen quad
            glDrawArrays(GL_TRIANGLES, 0, 3);
        
            UnBind(m_PipelineProcedural);
            break;
            
        case 1: // Cubemap Sampling
        case 3: // Baked Cubemap Sampling
            Bind(m_PipelineCubemap);
                
            // Scene buffers
            SetUniform(0, SceneObjects.Camera);
            SetUniform(1, SceneObjects.Lights);
            if (SceneObjects.SkylightMethod == 1)
            {
                SetUniform(m_PipelineCubemap, "SkyLightCubeMap", 0, SceneObjects.SkylightCube, SceneObjects.BaseSampler);
            }
            if (SceneObjects.SkylightMethod == 3)
            {
                SetUniform(m_PipelineCubemap, "SkyLightCubeMap", 0, SceneObjects.BakedSkylightCube, SceneObjects.BaseSampler);
            }
        
            // Draw screen quad
            glDrawArrays(GL_TRIANGLES, 0, 3);
        
            UnBind(m_PipelineCubemap);
            break;
            
        case 2: // HDRI Sampling
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
        PipelineUpdateFromFile(m_PipelineProcedural, "SkylightToRadiance.glsl", m_PipelineProceduralDefines);
        PipelineUpdateFromFile(m_PipelineCubemap, "SkylightToRadiance.glsl", m_PipelineCubemapDefines);
        PipelineUpdateFromFile(m_PipelineHDRI, "SkylightToRadiance.glsl", m_PipelineHDRIDefines);
    }

private:
    Shader::DefineArray<1> m_PipelineProceduralDefines = {Shader::Define("USE_PROCEDURAL_SKYLIGHT", "")};
    Shader::DefineArray<1> m_PipelineCubemapDefines = {Shader::Define("USE_CUBEMAP_SKYLIGHT", "")};
    Shader::DefineArray<1> m_PipelineHDRIDefines = {Shader::Define("USE_HDRI_SKYLIGHT", "")};
    
    Pipeline m_PipelineProcedural;
    Pipeline m_PipelineCubemap;
    Pipeline m_PipelineHDRI;
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
        m_PipelineProcedural(PipelineFromFile("Mesh To Radiance", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "MeshToRadiance.glsl", m_PipelineProceduralDefines)),
        m_PipelineCubemap(PipelineFromFile("Mesh To Radiance", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "MeshToRadiance.glsl", m_PipelineCubemapDefines)),
        m_PipelineHDRI(PipelineFromFile("Mesh To Radiance", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "MeshToRadiance.glsl", m_PipelineHDRIDefines))
    {}
    
    // void Update(double DeltaTime);
    void Draw(const SceneBuffers& SceneObjects)
    {
        DebugScopeMarker scope("Draw Scene");

        GLTF::MeshInstance Instance = SceneObjects.Scene.instances[0];
        const MeshObject& Mesh = SceneObjects.Scene.meshes[Instance.mesh];
        const Mesh::VertexGroup& Group = Mesh.GetGroups()[Instance.vertexGroup];
        
        Bind(Mesh.GetVAO());
        if (Mesh.GetIndexBuffer().has_value())
        {
            const IndexBuffer& indexBuffer = Mesh.GetIndexBuffer().value();
            Bind(indexBuffer);
        }

        switch (SceneObjects.SkylightMethod)
        {
        case 0: // Procedural
            Bind(m_PipelineProcedural);
                
            // Scene buffers
            SetUniform(0, SceneObjects.Camera);
            SetUniform(1, SceneObjects.Lights);
            SetUniform(2, SceneObjects.ProceduralSkyParameters);

            SetUniform(m_PipelineProcedural, "Model", MakeHomogeneousIdentity<float>());

            // Material
            SetUniform(m_PipelineProcedural, "BaseColor", m_Material.BaseColor);
            SetUniform(m_PipelineProcedural, "Roughness", m_Material.Roughness);
            SetUniform(m_PipelineProcedural, "Metalness", m_Material.Metalness);
        
            SetUniform(m_PipelineProcedural, "IndirectLightingSampleCount", m_SkyLightSampleCount);
            break;
            
        case 1: // Cubemap Sampling
        case 3: // Baked Cubemap Sampling
            Bind(m_PipelineCubemap);
                
            // Scene buffers
            SetUniform(0, SceneObjects.Camera);
            SetUniform(1, SceneObjects.Lights);
            if (SceneObjects.SkylightMethod == 1)
            {
                SetUniform(m_PipelineCubemap, "SkyLightCubeMap", 0, SceneObjects.SkylightCube, SceneObjects.BaseSampler);
            }
            if (SceneObjects.SkylightMethod == 3)
            {
                SetUniform(m_PipelineCubemap, "SkyLightCubeMap", 0, SceneObjects.BakedSkylightCube, SceneObjects.BaseSampler);
            }
            
            SetUniform(m_PipelineCubemap, "Model", MakeHomogeneousIdentity<float>());

            // Material
            SetUniform(m_PipelineCubemap, "BaseColor", m_Material.BaseColor);
            SetUniform(m_PipelineCubemap, "Roughness", m_Material.Roughness);
            SetUniform(m_PipelineCubemap, "Metalness", m_Material.Metalness);
        
            SetUniform(m_PipelineCubemap, "IndirectLightingSampleCount", m_SkyLightSampleCount);
            break;
            
        case 2: // HDRI Sampling
            Bind(m_PipelineHDRI);
                
            // Scene buffers
            SetUniform(0, SceneObjects.Camera);
            SetUniform(1, SceneObjects.Lights);
            SetUniform(m_PipelineHDRI, "SkyLightHDRi", 0, SceneObjects.SkylightHDRI, SceneObjects.BaseSampler);

            SetUniform(m_PipelineHDRI, "Model", MakeHomogeneousIdentity<float>());

            // Material
            SetUniform(m_PipelineHDRI, "BaseColor", m_Material.BaseColor);
            SetUniform(m_PipelineHDRI, "Roughness", m_Material.Roughness);
            SetUniform(m_PipelineHDRI, "Metalness", m_Material.Metalness);
        
            SetUniform(m_PipelineHDRI, "IndirectLightingSampleCount", m_SkyLightSampleCount);
            break;
        }
                
        if (Mesh.GetIndexBuffer().has_value())
        {
            const IndexBuffer& indexBuffer = Mesh.GetIndexBuffer().value();
            
            glDrawElements(ToGLGeometryType(Mesh.GetVertexType()), Group.VertexCount, ToGLIndexType(indexBuffer.GetIndexType()), (void*)(Group.FirstVertex * ToGLIndexSize(indexBuffer.GetIndexType())));
        }
        else
        {
            glDrawArrays(ToGLGeometryType(Mesh.GetVertexType()), Group.FirstVertex, Group.VertexCount);
        }

        switch (SceneObjects.SkylightMethod)
        {
        case 0: // Procedural
            UnBind(m_PipelineProcedural);
            break;
            
        case 1: // Cubemap Sampling
            UnBind(m_PipelineCubemap);
            break;
            
        case 2: // HDRI Sampling
            UnBind(m_PipelineHDRI);
            break;
        }
        
        UnBind(Mesh.GetVAO());
    }
    
    void Reload()
    {
        PipelineUpdateFromFile(m_PipelineProcedural, "MeshToRadiance.glsl", m_PipelineProceduralDefines);
        PipelineUpdateFromFile(m_PipelineCubemap, "MeshToRadiance.glsl", m_PipelineCubemapDefines);
        PipelineUpdateFromFile(m_PipelineHDRI, "MeshToRadiance.glsl", m_PipelineHDRIDefines);
    }
    
    MaterialParams& Material() { return m_Material; }
    
    uint32_t& SkyLightSampleCount() {return m_SkyLightSampleCount;}
    
private:
    Shader::DefineArray<1> m_PipelineProceduralDefines = {Shader::Define("USE_PROCEDURAL_SKYLIGHT", "")};
    Shader::DefineArray<1> m_PipelineCubemapDefines = {Shader::Define("USE_CUBEMAP_SKYLIGHT", "")};
    Shader::DefineArray<1> m_PipelineHDRIDefines = {Shader::Define("USE_HDRI_SKYLIGHT", "")};
    
    Pipeline m_PipelineProcedural;
    Pipeline m_PipelineCubemap;
    Pipeline m_PipelineHDRI;
    
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
        FrameBuffer SceneRadianceFB(FrameBuffer::Attachment(SceneRadianceRT, FrameBuffer::ClearColor(0.f)));

        SceneBuffers GPUScene;
        
        // Load scene data
        {
            std::filesystem::path path;
            if (GetAbsoluteFilePath(std::filesystem::path("Meshes") / "Sphere.glb" ,path))
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
        HDRiToCubemap HDRiToCubemapPass(GPUScene);
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
            
            // Handle shader reload
            if (RequestShaderReload)
            {
                DrawSkyPass.Reload();
                DrawScenePass.Reload();
                DrawPostProcessPass.Reload();
                HDRiToCubemapPass.Reload();
                
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
                GPUScene.ProceduralSkyParameters.Data(&proceduralSkylightData, sizeof(proceduralSkylightData));
            }
            
            // Bake non real time data
            if (RequestRebake)
            {
                HDRiToCubemapPass.Draw(GPUScene);
                
                // RequestRebake = false;
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

                static const char* SkyLightMethodNames[] =
                {
                    "Procedural", "Cubemap", "HDRi", "Baked From HDRi"
                };
                ImGui::ListBox("Sky Light Method", (int*)&GPUScene.SkylightMethod, SkyLightMethodNames, 4);
                GPUScene.SkylightMethod = Math::Clamp(GPUScene.SkylightMethod, 0u, 3u);
                
                ImGui::SliderInt("Sky light Sample Count", (int*)&DrawScenePass.SkyLightSampleCount(), 1, 1024);

                ImGui::Separator();

                if (GPUScene.SkylightMethod == 0)
                {
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
                }        
                
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