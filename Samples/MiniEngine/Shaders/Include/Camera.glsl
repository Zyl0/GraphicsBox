#ifndef INCLUDE_GUARD_GLSL_CAMERA
#define INCLUDE_GUARD_GLSL_CAMERA

struct CameraData{

    // Matrices
    mat4    Camera_WorldToView;
    mat4    Camera_WorldToProj;
    mat4    Camera_ViewToWorld;
    mat4    Camera_ViewToProj;
    mat4    Camera_ProjToView;
    mat4    Camera_ProjToWorld;

    // Camera properties
    vec3    Camera_WorldPosition;
    vec3    Camera_WorldUp;
    float   Camera_AspectRatio;
    vec3    Camera_WorldForward;
    vec3    Camera_WorldRight;

    // Screen
    vec2    Camera_ProjToViewport;
    vec2    Camera_ViewportToProj;
};

vec3 CameraWorldPosition(in CameraData Camera)
{
    return Camera.Camera_WorldPosition;
}

vec4 WorldToView(in CameraData Camera, vec4 Position)
{
    return Camera.Camera_WorldToView * Position;
}

vec3 WorldToView(in CameraData Camera, vec3 Vector)
{
    return (Camera.Camera_WorldToView * vec4(Vector, 0.0f)).xyz;
}

vec4 WorldToProj(in CameraData Camera, vec4 Position)
{
    return Camera.Camera_WorldToProj * Position;
}

vec3 WorldToProj(in CameraData Camera, vec3 Vector)
{
    return (Camera.Camera_WorldToProj * vec4(Vector, 0.0f)).xyz;
}

vec4 ViewToWorld(in CameraData Camera, vec4 Position)
{
    return Camera.Camera_ViewToWorld * Position;
}

vec3 ViewToWorld(in CameraData Camera, vec3 Vector)
{
    return (Camera.Camera_ViewToWorld * vec4(Vector, 0.0f)).xyz;
}

vec4 ProjToWorld(in CameraData Camera, vec4 Position)
{
    return Camera.Camera_ProjToWorld * Position;
}

vec3 ProjToWorld(in CameraData Camera, vec3 Vector)
{
    return (Camera.Camera_ProjToWorld * vec4(Vector, 0.0f)).xyz;
}

vec4 ProjToViewport(in CameraData Camera, vec4 Proj)
{
    Proj.xy = Proj.xy * Camera.Camera_ProjToViewport;
    return Proj;
}

vec4 ViewportToProj(in CameraData Camera, vec4 Viewport)
{
    Viewport.xy = Viewport.xy * Camera.Camera_ViewportToProj;
    return Viewport;
}

#endif // INCLUDE_GUARD_GLSL_CAMERA