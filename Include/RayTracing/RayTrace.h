#pragma once

#include "RayTrace.h"
#include "Types.h"
#include "Math/Transforms.h"
#include "Modeling/Mesh.h"

struct Hit
{
    Hit() :  t(), u(), v(), face(std::numeric_limits<uint32_t>::max()) {}
    Hit(float T, float U, float V, uint32_t Face) : t(T), u(U), v(V), face(Face) {}
    
    float t;
    float u, v;
    uint32_t face;
    
    INLINE bool IsValid() const {return face != std::numeric_limits<uint32_t>::max();}
    INLINE operator bool () const {return face != std::numeric_limits<uint32_t>::max();}
    INLINE bool operator==(const Hit& outer) const {return ((IsValid() && outer.IsValid()) || (!IsValid() && !outer.IsValid())) && (!IsValid() || (t == outer.t && u == outer.u && v == outer.v));}
    INLINE bool operator!=(const Hit& outer) const {return ((IsValid() && !outer.IsValid()) || (!IsValid() && outer.IsValid())) || (IsValid() && (t == outer.t || u == outer.u || v == outer.v));}
};

Hit IntersectTriangle(const Mesh::Face& Face, const Ray& Ray);

float VertexInterpolate(const Hit& Hit, float a, float b, float c);
Math::Vector2f VertexInterpolate(const Hit& Hit, Math::Vector2f a, Math::Vector2f b, Math::Vector2f c);
Math::Vector3f VertexInterpolate(const Hit& Hit, Math::Vector3f a, Math::Vector3f b, Math::Vector3f c);
Math::Vector4f VertexInterpolate(const Hit& Hit, Math::Vector4f a, Math::Vector4f b, Math::Vector4f c);

class TraceRay
{
public:
    class iterator;
    
    TraceRay(Mesh& Mesh, const Ray& Ray, const Math::Transform4f& WorldToModel = Math::MakeMatrix4Identity<float>());
    
    Hit Next();
    INLINE Hit ClosestHit() const {return m_ClosestHit;}
    
    class iterator
    {
    public:
        iterator() : m_Hit(), m_TraceRay(nullptr) {}
        iterator(TraceRay& TraceRay);

        iterator& operator++ ();
        INLINE Hit operator* () const {return m_Hit;}
        INLINE bool operator!= (const iterator& outer) const {return m_TraceRay != outer.m_TraceRay || m_Hit != outer.m_Hit;}
        INLINE bool operator== (const iterator& outer) const {return m_TraceRay == outer.m_TraceRay && m_Hit == outer.m_Hit;}
    
    private:
        Hit m_Hit;
        TraceRay* m_TraceRay;
    };
    
    iterator begin() {return iterator(*this);}
    iterator end() const {return iterator();}
    
private:
    Mesh::Faces MeshFaces;
    Mesh::Face::iterator Current;
    Mesh::Face::iterator End;
    Mesh::VertexType FaceType;
    Ray m_Ray;
    Hit m_ClosestHit;
};