#include "RayTracing/RayTrace.h"

using namespace Math;

Hit IntersectTriangle(const Mesh::Face& Face, const Ray& Ray)
{
    // TODO maybe Face should not contain a ref to the mesh
    
    const Vector3f &a = Face[0u].Position(), &b = Face[1u].Position(), &c = Face[2u].Position();
    Vector3f e1(a, b), e2(a, c);
    
    Vector3f pvec = Cross(Ray.direction, e2);
    float det = Dot(e1, pvec);
        
    float inv_det = 1 / det;
    Vector3f tvec(a, Ray.origin);
        
    float u = Dot(tvec, pvec) * inv_det;
    if(u < 0 || u > 1) return Hit();
        
    Vector3f qvec = Cross(tvec, e1);
    float v = Dot(Ray.direction, qvec) * inv_det;
    if(v < 0 || u + v > 1) return Hit();
        
    float t = Dot(e2, qvec) * inv_det;
    if(t < 0 || t > Ray.distance) return Hit();
        
    return Hit(t, u, v, Face.FirstVertex());
}

float VertexInterpolate(const Hit& Hit, float a, float b, float c)
{
    return (1 - Hit.u - Hit.v) * a + Hit.u * b + Hit.v * c;
}

Math::Vector2f VertexInterpolate(const Hit& Hit, Math::Vector2f a, Math::Vector2f b, Math::Vector2f c)
{
    return a * (1.f - Hit.u - Hit.v) + b * Hit.u + c * Hit.v;
}

Math::Vector3f VertexInterpolate(const Hit& Hit, Math::Vector3f a, Math::Vector3f b, Math::Vector3f c)
{
    return a * (1.f - Hit.u - Hit.v) + b * Hit.u + c * Hit.v;
}

Math::Vector4f VertexInterpolate(const Hit& Hit, Math::Vector4f a, Math::Vector4f b, Math::Vector4f c)
{
    return a * (1.f - Hit.u - Hit.v) + b * Hit.u + c * Hit.v;
}

TraceRay::TraceRay(Mesh& Mesh, const Ray& Ray, const Math::Transform4f& WorldToModel):
    MeshFaces(Mesh),
    Current(MeshFaces.begin()),
    End(MeshFaces.end()),
    FaceType(Mesh.GetMeshType()),
    m_Ray(Ray)
{    
    Vector3f end =  m_Ray.origin + m_Ray.distance * m_Ray.direction;
    
    Vector4f t = WorldToModel * Vector4f(m_Ray.origin, 1.0f);
    m_Ray.origin = Vector3f(t.XYZ()) / t.w;
    
    t = WorldToModel * Vector4f(end, 1.0f);
    end = Vector3f(t.XYZ()) / t.w;
    
    m_Ray.distance = Magnitude(end - m_Ray.origin);
    m_Ray.direction = Normalize(end - m_Ray.origin);
}

Hit TraceRay::Next()
{
    for (; Current != End; ++Current)
    {
        switch (FaceType)
        {            
        case Mesh::TRIANGLE_STRIP_ADJACENCY:
        case Mesh::TRIANGLES_ADJACENCY:
        case Mesh::TRIANGLE_STRIP:
        case Mesh::TRIANGLE_FAN:
        case Mesh::TRIANGLES:
            if (Hit hit = IntersectTriangle(*Current, m_Ray); hit)
            {
                if (!m_ClosestHit || m_ClosestHit.t > hit.t)
                {
                    m_ClosestHit = hit;
                    return hit;
                }
            }
            break;
            
        case Mesh::POINTS:
        case Mesh::LINE_STRIP:
        case Mesh::LINE_LOOP:
        case Mesh::LINES:
        case Mesh::LINE_STRIP_ADJACENCY:
        case Mesh::LINES_ADJACENCY:
        case Mesh::PATCHES:
        case Mesh::QUAD_STRIP:
        case Mesh::QUADS:
        case Mesh::_Count:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported face type for ray tracing")
        }
    }
    
    return Hit();
}

TraceRay::iterator::iterator(TraceRay& TraceRay): m_Hit(TraceRay.Next()), m_TraceRay(&TraceRay)
{
    if (!m_Hit)
    {
        m_TraceRay = nullptr;
        m_Hit = Hit();
    }
}

TraceRay::iterator& TraceRay::iterator::operator++()
{
    if (m_TraceRay == nullptr) return *this;
    
    m_Hit = m_TraceRay->Next();
    
    if (!m_Hit)
    {
        m_TraceRay = nullptr;
        m_Hit = Hit();
    }
    
    return *this;
}
