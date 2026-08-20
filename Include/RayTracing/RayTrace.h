#pragma once

#include <stack>

#include "RayTrace.h"
#include "Types.h"
#include "Math/Box.h"
#include "Math/Transforms.h"
#include "Memory/Functions.h"
#include "Modeling/Mesh.h"
#include "Shared/Assertion.h"

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

float VertexInterpolateTriangle(const Hit& Hit, float a, float b, float c);
Math::Vector2f VertexInterpolateTriangle(const Hit& Hit, Math::Vector2f a, Math::Vector2f b, Math::Vector2f c);
Math::Vector3f VertexInterpolateTriangle(const Hit& Hit, Math::Vector3f a, Math::Vector3f b, Math::Vector3f c);
Math::Vector4f VertexInterpolateTriangle(const Hit& Hit, Math::Vector4f a, Math::Vector4f b, Math::Vector4f c);

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

template <
    typename T,
    typename MetaType,
    Math::Box3f(ElementBounds)(const T& Element, const MetaType& Tree),
    Math::Vector3f(ElementCenter)(const T& Element, const MetaType& Tree)
    >
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
    INLINE bool Empty() const {return Tree.empty();}

    void Invalidate()
    {
        Tree.clear();
        Head = std::numeric_limits<uint32_t>::max();
    }
    void Rebuild();
    
    std::vector<T> Elements;
    std::vector<Node> Tree;
    uint32_t Head = std::numeric_limits<uint32_t>::max();
    uint32_t LeafSize = 0;
    MetaType Meta;

private:
    static Node MakeNode(const Math::Box3f& bounds, const uint32_t left, const uint32_t right)
    {
        Node node;
        node.Bounds = bounds;
        node.SetLeftNode(left);
        node.SetRightNode(right);
    
        AssertOrError(node.IsNode(), "BLAS Node is not node")
    
        return node;
    }
    static Node MakeLeaf(const Math::Box3f& bounds, const uint32_t begin, const uint32_t end)
    {
        Node leaf;
        leaf.Bounds = bounds;
        leaf.SetLeafBegin(begin);
        leaf.SetLeafEnd(end);
    
        AssertOrError(leaf.IsLeaf(), "BLAS Node is not leaf")
    
        return leaf;
    }
    Math::Box3f GroupBounds(const uint32_t begin, const uint32_t end)
    {
        Math::Box3f box = ElementBounds(Elements[begin], Meta);
    
        for(uint32_t i= begin +1; i < end; i++)
        {
            box.Insert(ElementBounds(Elements[i], Meta));
        }
        
        return box;
    }
    Math::Box3f GroupCentroidBounds(const uint32_t begin, const uint32_t end)
    {
        Math::Vector3f center = ElementCenter(Elements[begin], Meta);
        Math::Box3f centroidBox(center, center);
    
        for(uint32_t i= begin +1; i < end; i++)
        {
            centroidBox.Insert(ElementCenter(Elements[i], Meta));
        }
        
        return centroidBox;
    }
    uint32_t BuildNode(uint32_t begin, uint32_t end);
};

template <
    typename T,
    typename MetaType,
    Math::Box3f(ElementBounds)(const T& Element, const MetaType& Tree),
    Math::Vector3f(ElementCenter)(const T& Element, const MetaType& Tree)
    >
void BVHT<T, MetaType, ElementBounds, ElementCenter>::Rebuild()
{
    Invalidate();
    Head = BuildNode(0, Elements.size());
}

template <
    typename T,
    typename MetaType,
    Math::Box3f(ElementBounds)(const T& Element, const MetaType& Tree),
    Math::Vector3f(ElementCenter)(const T& Element, const MetaType& Tree)
    >
uint32_t BVHT<T, MetaType, ElementBounds, ElementCenter>::BuildNode(uint32_t begin, uint32_t end)
{
    if (LeafSize > 1 ? (end - begin < (LeafSize + 1)) : (end - begin < 2))
    {
        uint32_t index = Tree.size();
        Tree.push_back( MakeLeaf( GroupBounds(begin, end), begin, end ) );
        return index;
    }

    Math::Box3f centersBounds = GroupCentroidBounds(begin, end);
    Math::Vector3f d = Math::Vector3f(centersBounds.a, centersBounds.b);
    
    // Pick the widest centroid axis
    uint32_t axis;
    if(d.x > d.y && d.x > d.z)
    {
        axis = 0;
    }
    else if(d.y > d.z)
    {
        axis = 1;
    }
    else
    {
        axis = 2;
    }

    float cut = centersBounds.Center(axis);
    
    auto pm= std::partition(Elements.data() + begin, Elements.data() + end, 
        [axis, cut,this]( const T& Node ) 
        {
            auto bounds = ElementBounds(Node, this->Meta);
            return bounds.Center(axis) < cut; 
        }
    );
    uint32_t m = std::distance(Elements.data(), pm);
    
    // Ensure partition separated the faces in two groups
    if(m == begin || m == end)
    {
        m = (begin + end) / 2;
    }
    AssertOrError(m != begin && m != end, "Failed to sperate a group of primitive faces into two sub groups")
    
    uint32_t LeftSubGroup = BuildNode(begin, m);
    
    uint32_t RightSubGroup = BuildNode(m, end);
    
    uint32_t index = Tree.size();
    Tree.push_back( MakeNode( Math::Box3f(Tree[LeftSubGroup].Bounds, Tree[RightSubGroup].Bounds), LeftSubGroup, RightSubGroup ) );
    return index;
}

struct BLASDesc
{
    Mesh::VertexGroup VertexGroup;
    Mesh::VertexType VertexType;
    const Mesh* MeshRef;
};

// Face index
using BLASElement = uint32_t;

Math::Box3f BLASElementGetBounds(const BLASElement& Element, const BLASDesc& Tree);
Math::Vector3f BLASElementGetCenter(const BLASElement& Element, const BLASDesc& Tree);

// Bottom Layer Acceleration Structure
// 
// Bottom layer is the layer of the geometry itself
using BLAS = BVHT<BLASElement, BLASDesc, BLASElementGetBounds, BLASElementGetCenter>;

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

Math::Box3f TLASElementGetBounds(const TLASElement& Element, const TLASDesc& Tree);
Math::Vector3f TLASElementGetCenter(const TLASElement& Element, const TLASDesc& Tree);

// Mesh Layer Acceleration Structure
// 
// The mesh layer is the tree of 
using TLAS = BVHT<TLASElement, TLASDesc, TLASElementGetBounds, TLASElementGetCenter>;

void TLASAddInstance(TLAS& BVH, const BLAS& BLAS, size_t Material, const Math::Transform4f& Transform);
INLINE void TLASRebuild(TLAS& BVH) {BVH.Rebuild();}

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