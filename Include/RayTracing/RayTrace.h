#pragma once

#include <stack>

#include "RayTrace.h"
#include "Types.h"
#include "Math/Box.h"
#include "Math/Transforms.h"
#include "Memory/Functions.h"
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

Hit IntersectTriangle(const Mesh::ConstFace& Face, const Ray& Ray);

float VertexInterpolate(const Hit& Hit, float a, float b, float c);
Math::Vector2f VertexInterpolate(const Hit& Hit, Math::Vector2f a, Math::Vector2f b, Math::Vector2f c);
Math::Vector3f VertexInterpolate(const Hit& Hit, Math::Vector3f a, Math::Vector3f b, Math::Vector3f c);
Math::Vector4f VertexInterpolate(const Hit& Hit, Math::Vector4f a, Math::Vector4f b, Math::Vector4f c);

class TraceRay
{
public:    
    TraceRay(const Mesh& Mesh, const Ray& Ray, const Math::Transform4f& WorldToModel = Math::MakeMatrix4Identity<float>());
    TraceRay(const Mesh& Mesh, unsigned FirstVertex, unsigned VertexCount, const Ray& Ray, const Math::Transform4f& WorldToModel = Math::MakeMatrix4Identity<float>());

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
    Mesh::ConstFaces MeshFaces;
    Mesh::ConstFaces::iterator Current;
    Mesh::ConstFaces::iterator End;
    Mesh::VertexType FaceType;
    Ray m_Ray;
    Hit m_ClosestHit;
    unsigned m_FirstVertex;
    unsigned m_VertexCount;
};

struct BoxHit
{
    float tmin, tmax;
    
    BoxHit() : tmin(FLT_MAX), tmax(-FLT_MAX) {}
    BoxHit( const float _tmin, const float _tmax ) : tmin(_tmin), tmax(_tmax) {}
    
    operator bool( ) const { return tmin <= tmax; }
};

BoxHit IntersectBox(const Math::Box3f& Box, const Ray& Ray);
BoxHit IntersectBox(const Math::Box3f& Box, const Ray& Ray, Math::Vector3f InverseDirection);

template < typename T, typename MetaType = void>
struct BVHT
{
    BVHT() = default;
    
    struct Node
    {
        Math::Box3f Bounds;
        uint32_t Left;
        uint32_t Right;
        
        INLINE bool IsNode() const {return SampleBoolFromMask(Right, 31);}
        INLINE bool IsLeaf() const {return !IsNode();}
        
        INLINE uint32_t LeftIndex() const {return Left & (~(1 << 31));}
        INLINE uint32_t RightIndex() const {return Right & (~(1 << 31));}
        
        void SetLeftNode(uint32_t index) {Left = index; Left = SetBoolAt(Left, 31, true);}
        void SetRightNode(uint32_t index) {Right = index; Right = SetBoolAt(Right, 31, true);}
        
        void SetLeafBegin(uint32_t index) {Left = index; Left = SetBoolAt(Left, 31, false);}
        void SetLeafEnd(uint32_t index) {Right = index; Right = SetBoolAt(Right, 31, false);}
    };
    
    INLINE Math::Box3f Bounds() const {return !Tree.empty() ? Tree[Head].Bounds : Math::Box3f();}
    INLINE bool Empty() const {return !Tree.empty();}
    
    std::vector<T> Elements;
    std::vector<Node> Tree;
    uint32_t Head = std::numeric_limits<uint32_t>::max();
    MetaType Meta;
};

struct BLASDesc
{
    Mesh::VertexGroup VertexGroup;
    Mesh::VertexType VertexType;
    const Mesh* MeshRef;
    
    // Face count per leaf
    uint32_t LeafSize;
};

// Face index
using BLASElement = uint32_t;

// Bottom Layer Acceleration Structure
// 
// Bottom layer is the layer of the geometry itself
using BLAS = BVHT<BLASElement, BLASDesc>;

BLAS BuildBLAS(const Mesh& Mesh, uint8_t VertexGroup, uint32_t LeafSize = 2);

struct BVHHit
{
    uint32_t NodeIndex;
    BoxHit Hit;
    
    BVHHit() : NodeIndex(std::numeric_limits<uint32_t>::max()), Hit() {}
    BVHHit(uint32_t Index, BoxHit Hit) : NodeIndex(Index), Hit(Hit) {}
    
    INLINE operator bool () const {return NodeIndex != std::numeric_limits<uint32_t>::max();}
};

class TraceRayBLAS
{
public:
    TraceRayBLAS(const BLAS& Mesh, const Ray& Ray, const Math::Transform4f& WorldToModel = Math::MakeMatrix4Identity<float>());
    
    Hit Next();
    INLINE Hit ClosestHit() const {return m_ClosestHit;}
    
    INLINE void ClipFar(float Distance) {m_tmax = Distance;};
    
    INLINE Ray GetRay() const {return m_Ray;}
    
    class iterator
    {
    public:
        iterator() : m_Hit(), m_TraceRay(nullptr) {}
        iterator(TraceRayBLAS& TraceRay);

        iterator& operator++ ();
        INLINE Hit operator* () const {return m_Hit;}
        INLINE bool operator!= (const iterator& outer) const {return m_TraceRay != outer.m_TraceRay || m_Hit != outer.m_Hit;}
        INLINE bool operator== (const iterator& outer) const {return m_TraceRay == outer.m_TraceRay && m_Hit == outer.m_Hit;}
    
    private:
        Hit m_Hit;
        TraceRayBLAS* m_TraceRay;
    };
    
    iterator begin() {return iterator(*this);}
    iterator end() const {return iterator();}
    
private:
    const BLAS* m_Blas;
    std::stack<uint32_t> m_IterationStack;
    BVHHit m_CurrentBVHHit;
    Hit m_ClosestHit;
    uint32_t m_CurrentElementIndex;
    float m_tmax;
    Ray m_Ray;
};

// struct MLASElement
// {
//     
// };
// 
// struct MLASDesc
// {
//     
// };
// 
// // Mesh Layer Acceleration Structure
// // 
// // The mesh layer is the tree of 
// using MLAS = BVHT<MLASElement, MLASDesc>;

struct TLASElement
{
    const BLAS* BLAS;
    Math::Transform4f ModelToWorld;
    Math::Transform4f WorldToModel;
    size_t MaterialIndex;
    
    INLINE Math::Point3f Center() const;
    float Center(size_t Index) const;
};

struct TLASDesc
{
    
};

// Mesh Layer Acceleration Structure
// 
// The mesh layer is the tree of 
using TLAS = BVHT<TLASElement, TLASDesc>;

void TLASAddInstance(TLAS& BVH, const BLAS& BLAS, size_t Material, const Math::Transform4f& Transform);
void TLASRebuild(TLAS& BVH);

class TraceRayTLAS
{
public:
    TraceRayTLAS(const TLAS& Scene, const Ray& Ray):
        m_Tlas(&Scene), m_tmax(Ray.distance), m_Ray(Ray)
    {
        m_IterationStack.push(m_Tlas->Head);
    }
    
    BVHHit Next();
    
    INLINE void ClipFar(float Distance) {m_tmax = Distance;};
    
    INLINE Ray GetRay() const {return m_Ray;}
    
    class iterator
    {
    public:
        iterator() : m_Hit(), m_TraceRay(nullptr) {}
        iterator(TraceRayTLAS& TraceRay);

        iterator& operator++ ();
        INLINE BVHHit operator* () const {return m_Hit;}
        INLINE bool operator!= (const iterator& outer) const {return m_TraceRay != outer.m_TraceRay || m_Hit.NodeIndex != outer.m_Hit.NodeIndex;}
        INLINE bool operator== (const iterator& outer) const {return m_TraceRay == outer.m_TraceRay && m_Hit.NodeIndex == outer.m_Hit.NodeIndex;}
    
    private:
        BVHHit m_Hit;
        TraceRayTLAS* m_TraceRay;
    };
    
    iterator begin() {return iterator(*this);}
    iterator end() const {return iterator();}
    
private:
    const TLAS* m_Tlas;
    std::stack<uint32_t> m_IterationStack;
    float m_tmax;
    Ray m_Ray;
};