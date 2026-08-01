#include "RayTracing/RayTrace.h"

#include "Modeling/Mesh.h"
#include "Shared/Assertion.h"

using namespace Math;

Hit IntersectTriangle(const Mesh::ConstFace& Face, const Ray& Ray)
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

TraceRay::TraceRay(const Mesh& Mesh, const Ray& Ray, const Math::Transform4f& WorldToModel):
    MeshFaces(Mesh),
    Current(MeshFaces.begin()),
    End(MeshFaces.end()),
    FaceType(Mesh.GetMeshType()),
    m_Ray(Ray),
    m_FirstVertex(0),
    m_VertexCount(Mesh.GetVertexCount())
{    
    Vector3f end =  m_Ray.origin + m_Ray.distance * m_Ray.direction;
    
    Vector4f t = WorldToModel * Vector4f(m_Ray.origin, 1.0f);
    m_Ray.origin = Vector3f(t.XYZ()) / t.w;
    
    t = WorldToModel * Vector4f(end, 1.0f);
    end = Vector3f(t.XYZ()) / t.w;
    
    m_Ray.distance = Magnitude(end - m_Ray.origin);
    m_Ray.direction = Normalize(end - m_Ray.origin);
}

TraceRay::TraceRay(const Mesh& Mesh, unsigned FirstVertex, unsigned VertexCount, const Ray& Ray, const Math::Transform4f& WorldToModel):
    MeshFaces(Mesh),
    Current(Mesh::ConstFace::iterator(Mesh, FirstVertex)),
    End(Mesh::ConstFace::iterator(Mesh, FirstVertex + VertexCount)),
    FaceType(Mesh.GetMeshType()),
    m_Ray(Ray),
    m_FirstVertex(0),
    m_VertexCount(Mesh.GetVertexCount())
{
    AssertOrError(FirstVertex < Mesh.GetVertexCount(), "Vertex index out of range");
    AssertOrError(FirstVertex + VertexCount <= Mesh.GetVertexCount(), "Vertex count out of range");

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
    for (; Current != End && Current.IsValid(); ++Current)
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

BoxHit IntersectBox(const Math::Box3f& Box, const Ray& Ray)
{
    return IntersectBox(Box, Ray, Vector3f(1.f) / Ray.direction);
}

BoxHit IntersectBox(const Math::Box3f& Box, const Ray& Ray, Math::Vector3f InverseDirection)
{
    Point3f rmin= Box.a;
    Point3f rmax= Box.b;
    if(Ray.direction.x < 0) std::swap(rmin.x, rmax.x);
    if(Ray.direction.y < 0) std::swap(rmin.y, rmax.y);
    if(Ray.direction.z < 0) std::swap(rmin.z, rmax.z);
    Vector3f dmin= (rmin - Ray.origin) * InverseDirection;
    Vector3f dmax= (rmax - Ray.origin) * InverseDirection;
        
    float tmin= std::max(dmin.z, std::max(dmin.y, std::max(dmin.x, 0.f)));
    float tmax= std::min(dmax.z, std::min(dmax.y, std::min(dmax.x, Ray.distance)));
    return BoxHit(tmin, tmax);
}

static Box3f FaceBounds(Mesh::ConstFaces Mesh, uint32_t FaceIndex)
{
    Mesh::ConstFace face = Mesh[FaceIndex];
    Vector3f min = face.begin().operator*().Position();
    Vector3f max = min;
    
    for (const auto& vertex : face)
    {
        Point3f p = vertex.Position();
        min.x = std::min(min.x, p.x);
        min.y = std::min(min.y, p.y);
        min.z = std::min(min.z, p.z);

        max.x = std::max(max.x, p.x);
        max.y = std::max(max.y, p.y);
        max.z = std::max(max.z, p.z);
    }
    
    return {min, max};
}

static Box3f FaceGroupBounds(Mesh::ConstFaces Mesh, std::span<const BLASElement> Elements, const uint32_t begin, const uint32_t end)
{
    Box3f box = FaceBounds(Mesh, begin);
    
    for(uint32_t i= begin +1; i < end; i++)
    {
        box.Insert(FaceBounds(Mesh, Elements[i]));
    }
        
    return box;
}

static Box3f FaceGroupCentroidBounds(Mesh::ConstFaces Mesh, std::span<const BLASElement> Elements, const uint32_t begin, const uint32_t end)
{    
    Box3f box = FaceBounds(Mesh, begin), centroidBox(box.Center(), box.Center());
    
    for(uint32_t i= begin +1; i < end; i++)
    {
        box = FaceBounds(Mesh, Elements[i]);
        centroidBox.Insert(box.Center());
    }
        
    return centroidBox;
}

static BLAS::Node MakeBLASNode( const Box3f& bounds, const uint32_t left, const uint32_t right )
{
    BLAS::Node node;
    node.Bounds = bounds;
    node.SetLeftNode(left);
    node.SetRightNode(right);
    
    AssertOrError(node.IsNode(), "BLAS Node is not node")
    
    return node;
}

static BLAS::Node MakeBLASLeaf( const Box3f& bounds, const uint32_t begin, const uint32_t end )
{
    BLAS::Node leaf;
    leaf.Bounds = bounds;
    leaf.SetLeafBegin(begin);
    leaf.SetLeafEnd(end);
    
    AssertOrError(leaf.IsLeaf(), "BLAS Node is not leaf")
    
    return leaf;
}

static uint32_t BuildBLASNode(BLAS& BVH, Mesh::ConstFaces Mesh, const uint32_t begin, const uint32_t end)
{
    if(end - begin < (BVH.Meta.LeafSize + 1))
    {
        uint32_t index = BVH.Tree.size();
        BVH.Tree.push_back( MakeBLASLeaf( FaceGroupBounds(Mesh, BVH.Elements, begin, end), begin, end ) );
        return index;
    }
    
    Box3f cbounds = FaceGroupCentroidBounds(Mesh, BVH.Elements, begin, end);
    Vector3f d = Vector3f(cbounds.a, cbounds.b);
    
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

    float cut = cbounds.Center(axis);
    
    uint32_t* pm= std::partition(BVH.Elements.data() + begin, BVH.Elements.data() + end, 
        [axis, cut, Mesh]( uint32_t faceIndex ) 
        {
            Box3f bounds = FaceBounds(Mesh, faceIndex);
            return bounds.Center(axis) < cut; 
        }
    );
    uint32_t m = std::distance(BVH.Elements.data(), pm);
    
    // Ensure partition separated the faces in two groups
    if(m == begin || m == end)
    {
        m = (begin + end) / 2;
    }
    AssertOrError(m != begin && m != end, "Failed to sperate a group of primitive faces into two sub groups")
    
    uint32_t LeftSubGroup = BuildBLASNode(BVH, Mesh, begin, m);
    
    uint32_t RightSubGroup = BuildBLASNode(BVH, Mesh, m, end);
    
    uint32_t index = BVH.Tree.size();
    BVH.Tree.push_back( MakeBLASNode( Box3f(BVH.Tree[LeftSubGroup].Bounds, BVH.Tree[RightSubGroup].Bounds), LeftSubGroup, RightSubGroup ) );
    return index;
}

BLAS BuildBLAS(const Mesh& Mesh, uint8_t VertexGroup, uint32_t LeafSize)
{
    Mesh::VertexGroup Group = Mesh.GetVertexGroups()[VertexGroup];
    Mesh::VertexType Type = Mesh.GetMeshType();
    
    BLAS blas{};
    blas.Meta = {.VertexGroup = Group, .VertexType = Type, .MeshRef = &Mesh, .LeafSize = LeafSize};
    for (uint32_t i = Group.FirstVertex, iend = i + Group.VertexCount, increment = Mesh::FaceVertexIncrement(Type); i < iend; i+=increment)
    {
        blas.Elements.push_back(Mesh::FaceIndex(Type, i));
    }
    
    blas.Head = BuildBLASNode(blas, Mesh, 0, blas.Elements.size());

    return blas;
}

TraceRayBLAS::TraceRayBLAS(const BLAS& Mesh, const Ray& Ray, const Math::Transform4f& WorldToModel):
    m_Blas(&Mesh),
    m_Ray(Ray),
    m_IterationStack(),
    m_CurrentBVHHit(),
    m_ClosestHit(),
    m_CurrentElementIndex(std::numeric_limits<uint32_t>::max())
{
    Vector3f end =  m_Ray.origin + m_Ray.distance * m_Ray.direction;

    Vector4f t = WorldToModel * Vector4f(m_Ray.origin, 1.0f);
    m_Ray.origin = Vector3f(t.XYZ()) / t.w;

    t = WorldToModel * Vector4f(end, 1.0f);
    end = Vector3f(t.XYZ()) / t.w;

    m_Ray.distance = Magnitude(end - m_Ray.origin);
    m_Ray.direction = Normalize(end - m_Ray.origin);
    
    m_tmax = m_Ray.distance;
    
    m_IterationStack.push(m_Blas->Head);
}

Hit TraceRayBLAS::Next()
{
explore_bvh:
    if (!m_CurrentBVHHit)
    while (!m_IterationStack.empty())
    {
        uint32_t NodeIndex = m_IterationStack.top();
        m_IterationStack.pop();
        
        if (BoxHit hit = IntersectBox(m_Blas->Tree[NodeIndex].Bounds, m_Ray); hit && hit.tmin < m_tmax)
        {
            if (m_Blas->Tree[NodeIndex].IsNode())
            {
                m_IterationStack.push(m_Blas->Tree[NodeIndex].LeftIndex());
                m_IterationStack.push(m_Blas->Tree[NodeIndex].RightIndex());
            }
            else // if (m_Blas->Tree[NodeIndex].IsLeaf())
            {
                m_CurrentBVHHit = BVHHit(NodeIndex, hit);
                m_CurrentElementIndex = m_Blas->Tree[NodeIndex].LeftIndex();
                goto trace_leaf;
            }
        }
    }
    
trace_leaf:
    if (m_CurrentBVHHit)
    {
        uint32_t BLASFaceEnd = m_Blas->Tree[m_CurrentBVHHit.NodeIndex].RightIndex();
        Mesh::ConstFaces Faces(*(m_Blas->Meta.MeshRef));
        for (uint32_t ElementIndex = m_CurrentElementIndex; ElementIndex < BLASFaceEnd; ElementIndex++)
        {
            Mesh::ConstFace Face = Faces[m_Blas->Elements[ElementIndex]];
            
            switch (m_Blas->Meta.VertexType)
            {            
            case Mesh::TRIANGLE_STRIP_ADJACENCY:
            case Mesh::TRIANGLES_ADJACENCY:
            case Mesh::TRIANGLE_STRIP:
            case Mesh::TRIANGLE_FAN:
            case Mesh::TRIANGLES:
                if (Hit hit = IntersectTriangle(Face, m_Ray); hit)
                {
                    if (!m_ClosestHit || m_ClosestHit.t > hit.t)
                    {
                        m_ClosestHit = hit;
                        m_tmax = std::min(m_tmax, hit.t);
                        m_CurrentElementIndex = ElementIndex + 1;
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
        
        m_CurrentBVHHit = {};
        m_CurrentElementIndex = std::numeric_limits<uint32_t>::max();
        goto explore_bvh;
    }
    
    return Hit();
}

TraceRayBLAS::iterator::iterator(TraceRayBLAS& TraceRay)
{
    if (Hit hit = TraceRay.Next(); hit)
    {
        m_Hit = hit;
        m_TraceRay = &TraceRay;
    }
    else
    {
        m_Hit = {};
        m_TraceRay = nullptr;
    }
}

TraceRayBLAS::iterator& TraceRayBLAS::iterator::operator++()
{
    if (m_TraceRay == nullptr) return *this;

    if (Hit hit = m_TraceRay->Next(); hit)
    {
        m_Hit = hit;
    }
    else
    {
        m_Hit = {};
        m_TraceRay = nullptr;
    }
    
    return *this;
}

Point3f TLASElement::Center() const
{
    return ModelToWorld * BLAS->Bounds().Center();
}

float TLASElement::Center(size_t Index) const
{
    Point3f c = Center();
    return c[Index];
}

void TLASAddInstance(TLAS& BVH, const BLAS& BLAS, size_t Material, const Transform4f& Transform)
{
    BVH.Elements.emplace_back(&BLAS, Transform, Inverse(Transform), Material);
    
    // Invalidate
    BVH.Tree.clear();
    BVH.Head = std::numeric_limits<uint32_t>::max();
}

static TLAS::Node MakeTLASNode( const Box3f& bounds, const uint32_t left, const uint32_t right )
{
    TLAS::Node node;
    node.Bounds = bounds;
    node.SetLeftNode(left);
    node.SetRightNode(right);
    
    AssertOrError(node.IsNode(), "BLAS Node is not node")
    
    return node;
}

static TLAS::Node MakeTLASLeaf( const Box3f& bounds, const uint32_t element )
{
    TLAS::Node leaf;
    leaf.Bounds = bounds;
    leaf.SetLeafBegin(element);
    leaf.SetLeafEnd(element);
    
    AssertOrError(leaf.IsLeaf(), "BLAS Node is not leaf")
    
    return leaf;
}

static Box3f SafeTransformBounds(Box3f In, const Transform4f& Transform)
{
    Point3f AAA = Transform * In.a;
    Point3f AAB = Transform * Point3f(In.a.x, In.a.y, In.b.z);
    Point3f ABA = Transform * Point3f(In.a.x, In.b.y, In.a.z);
    Point3f ABB = Transform * Point3f(In.a.x, In.b.y, In.b.z);
    Point3f BAA = Transform * Point3f(In.b.x, In.a.y, In.a.z);
    Point3f BAB = Transform * Point3f(In.b.x, In.a.y, In.b.z);
    Point3f BBA = Transform * Point3f(In.b.x, In.b.y, In.a.z);
    Point3f BBB = Transform * In.b;
    
    Box3f out(AAA, AAA);
    out.Insert(AAB);
    out.Insert(ABA);
    out.Insert(ABB);
    out.Insert(BAA);
    out.Insert(BAB);
    out.Insert(BBA);
    out.Insert(BBB);
    
    return out;
}

static uint32_t BuildTLASNode(TLAS& BVH, const uint32_t begin, const uint32_t end)
{
    if(end - begin < 2)
    {
        uint32_t index = BVH.Tree.size();
        BVH.Tree.push_back( MakeTLASLeaf( SafeTransformBounds(BVH.Elements[begin].BLAS->Bounds(), BVH.Elements[begin].ModelToWorld), begin ) );
        return index;
    }
    
    Box3f cbounds(BVH.Elements[begin].Center(), BVH.Elements[begin].Center());
    for (uint32_t i = begin + 1; i < end; i++)
    {
        cbounds.Insert(BVH.Elements[i].Center());
    }
    Vector3f d = Vector3f(cbounds.a, cbounds.b);
    
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

    float cut = cbounds.Center(axis);
    
    TLASElement* pm= std::partition(BVH.Elements.data() + begin, BVH.Elements.data() + end, 
        [axis, cut]( const TLASElement& Instance ) 
        {
            return Instance.Center(axis) < cut; 
        }
    );
    uint32_t m = std::distance(BVH.Elements.data(), pm);
    
    // Ensure partition separated the faces in two groups
    if(m == begin || m == end)
    {
        m = (begin + end) / 2;
    }
    AssertOrError(m != begin && m != end, "Failed to sperate a group of primitive faces into two sub groups")
    
    uint32_t LeftSubGroup = BuildTLASNode(BVH, begin, m);
    
    uint32_t RightSubGroup = BuildTLASNode(BVH, m, end);
    
    uint32_t index = BVH.Tree.size();
    BVH.Tree.push_back( MakeTLASNode( Box3f(BVH.Tree[LeftSubGroup].Bounds, BVH.Tree[RightSubGroup].Bounds), LeftSubGroup, RightSubGroup ) );
    return index;
}

void TLASRebuild(TLAS& BVH)
{
    BVH.Tree.clear();
    BVH.Head = BuildTLASNode(BVH, 0, BVH.Elements.size());
}

BVHHit TraceRayTLAS::Next()
{
    while (!m_IterationStack.empty())
    {
        uint32_t NodeIndex = m_IterationStack.top();
        m_IterationStack.pop();
        
        if (BoxHit hit = IntersectBox(m_Tlas->Tree[NodeIndex].Bounds, m_Ray); hit && hit.tmin < m_tmax)
        {
            if (m_Tlas->Tree[NodeIndex].IsNode())
            {
                m_IterationStack.push(m_Tlas->Tree[NodeIndex].LeftIndex());
                m_IterationStack.push(m_Tlas->Tree[NodeIndex].RightIndex());
            }
            else // if (m_Blas->Tree[NodeIndex].IsLeaf())
            {
                return BVHHit(NodeIndex, hit);
            }
        }
    }
    
    return BVHHit();
}

TraceRayTLAS::iterator::iterator(TraceRayTLAS& TraceRay)
{
    if (BVHHit hit = TraceRay.Next())
    {
        m_Hit = hit;
        m_TraceRay = &TraceRay;
    }
    else
    {
        m_Hit = {};
        m_TraceRay = nullptr;
    }
}

TraceRayTLAS::iterator& TraceRayTLAS::iterator::operator++()
{
    if (m_TraceRay == nullptr) return *this;

    if (BVHHit hit = m_TraceRay->Next())
    {
        m_Hit = hit;
    }
    else
    {
        m_Hit = {};
        m_TraceRay = nullptr;
    }
    
    return *this;
}
