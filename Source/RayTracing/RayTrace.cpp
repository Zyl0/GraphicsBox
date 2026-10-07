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

HitWave<TriangleWave::kThreadCount> IntersectTriangle(const TriangleWave& Face, const Ray& Ray)
{
    using Vector3 = Simt::Vector3<float, TriangleWave::kThreadCount>;
    using Vector2 = Simt::Vector2<float, TriangleWave::kThreadCount>;
    using Mask = Simt::Scalar<float, TriangleWave::kThreadCount>::MaskType;
    using Float = Simt::Scalar<float, TriangleWave::kThreadCount>;

    HitWave<TriangleWave::kThreadCount> HitWave{};
    Mask mask = Face.Validity;
    [[unlikely]] if (mask.None()) return HitWave;
    
    const Vector3& a = Face.A, &b = Face.B, &c = Face.C;
    Vector3 e1(a, b), e2(a, c);
    const Vector3 direction = Vector3(Ray.direction);
    
    Vector3 pvec = Cross(direction, e2);
    Float det = Dot(e1, pvec);
        
    Float inv_det = Float(1) / det;
    Vector3 tvec(a, Ray.origin);

    Vector2 uv;
    uv.x = Dot(tvec, pvec) * inv_det;
    mask &= (uv.x >= Float(0) && uv.x <= Float(1));
        
    Vector3 qvec = Cross(tvec, e1);
    uv.y = Dot(direction, qvec) * inv_det;
    mask &= (uv.y >= 0) && (uv.x + uv.y <= 1);
        
    Float t = Dot(e2, qvec) * inv_det;
    mask &= (t >= 0) && t <= (Ray.distance);

    HitWave.uv = Select(uv, HitWave.uv, mask);
    HitWave.t = Select(t, HitWave.t, mask);
    HitWave.face = Select(Face.Faces, HitWave.face, mask);

    return HitWave;
}

float VertexInterpolateTriangle(const Hit& Hit, float a, float b, float c)
{
    return (1 - Hit.u - Hit.v) * a + Hit.u * b + Hit.v * c;
}

Math::Vector2f VertexInterpolateTriangle(const Hit& Hit, Math::Vector2f a, Math::Vector2f b, Math::Vector2f c)
{
    return a * (1.f - Hit.u - Hit.v) + b * Hit.u + c * Hit.v;
}

Math::Vector3f VertexInterpolateTriangle(const Hit& Hit, Math::Vector3f a, Math::Vector3f b, Math::Vector3f c)
{
    return a * (1.f - Hit.u - Hit.v) + b * Hit.u + c * Hit.v;
}

Math::Vector4f VertexInterpolateTriangle(const Hit& Hit, Math::Vector4f a, Math::Vector4f b, Math::Vector4f c)
{
    return a * (1.f - Hit.u - Hit.v) + b * Hit.u + c * Hit.v;
}

SurfaceHit HitInterpolateProperties(const Mesh& mesh, const Hit& Hit, const Math::Matrix4f* Transform)
{
    SurfaceHit surface;
    surface.Tangent = {0.0f};
    surface.TextureCoordinates = {0.0f};
    
    switch (mesh.GetMeshType())
    {
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
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported vertex type. Expected triangles")

    case Mesh::TRIANGLE_STRIP_ADJACENCY:
    case Mesh::TRIANGLES_ADJACENCY:
    case Mesh::TRIANGLE_STRIP:
    case Mesh::TRIANGLE_FAN:
    case Mesh::TRIANGLES:
        Mesh::ConstFace face(mesh, Hit.face);
        Mesh::ConstVertex a = face[0];
        Mesh::ConstVertex b = face[1];
        Mesh::ConstVertex c = face[2];
        
        if (Transform != nullptr)
        {
            Vector4f PositionH = ((*Transform) * Vector4f(VertexInterpolateTriangle(Hit, a.Position(), b.Position(), c.Position()), 1.0f));
            surface.Position = PositionH.xyz() / PositionH.w;
            surface.Normal = Normalize(((*Transform) * Vector4f(VertexInterpolateTriangle(Hit, a.Normal(), b.Normal(), c.Normal()), 0.0f)).xyz());
            if (mesh.HasTangents()) surface.Tangent = Normalize(((*Transform) * Vector4f(VertexInterpolateTriangle(Hit, a.Tangent(), b.Tangent(), c.Tangent()), 0.0f)).xyz());
        }
        else
        {
            surface.Position = Normalize(VertexInterpolateTriangle(Hit, a.Position(), b.Position(), c.Position()));
            surface.Normal = Normalize(VertexInterpolateTriangle(Hit, a.Normal(), b.Normal(), c.Normal()));
            if (mesh.HasTangents()) surface.Tangent = Normalize(VertexInterpolateTriangle(Hit, a.Tangent(), b.Tangent(), c.Tangent()));
        }
        if (mesh.HasTextureCoordinates()) surface.TextureCoordinates = VertexInterpolateTriangle(Hit, a.TextureCoordinate(), b.TextureCoordinate(), c.TextureCoordinate());
    }
    
    return surface;
}

SurfaceSampler CalcSurfaceSample(const Mesh& mesh, const Ray& ray, const Hit& Hit, Math::Vector2t<uint32_t> ViewportSize)
{
    if (!mesh.HasNormals() || !mesh.HasTextureCoordinates()) return {.dudx = 0,.dvdx = 0, .dudy = 0,.dvdy = 0}; // Degenerate triangle safety
    
    // TODO hit.face is supposed to return face index but returns face first vertex instead, investigate why.
    // Mesh::ConstFaces faces(mesh);
    // Mesh::ConstFace face = faces[Hit.face];
    
    Mesh::ConstFace face(mesh, Hit.face);
    
    Point3f v0 = face[0].Position(), v1 = face[1].Position(), v2 = face[2].Position();
    Vector2f t0 = face[0].TextureCoordinate(), t1 = face[1].TextureCoordinate(), t2 = face[2].TextureCoordinate();
    
    // --- STEP 1: Calculate Triangle Geometric Properties ---
    Vector3f e1 = v1 - v0;
    Vector3f e2 = v2 - v0;
    Vector3f unnormalized_normal = Cross(e1, e2);
    
    // Get the linear length (2x the Area)
    float len = std::sqrt(Dot(unnormalized_normal, unnormalized_normal));
    
    // Safety check against the linear length (much safer to reason about)
    if (len < 1e-12f) return {0,0, 0,0}; 
    // Normalize it!
    Vector3f normal = unnormalized_normal * (1.0f / len);
    // --- STEP 2: Compute Texture Gradients on the Surface (∇U, ∇V) ---
    float du1 = t1.x - t0.x;
    float dv1 = t1.y - t0.y;
    float du2 = t2.x - t0.x;
    float dv2 = t2.y - t0.y;
    // IMPORTANT DIFFERENCE: Because 'normal' is now normalized, 
    // we only divide by 'len' instead of 'len_sq'
    Vector3f c1 = Cross(e2, normal) * (1.0f / len);
    Vector3f c2 = Cross(normal, e1) * (1.0f / len);
    Vector3f grad_U = c1 * du1 + c2 * du2;
    Vector3f grad_V = c1 * dv1 + c2 * dv2;
    
    // --- STEP 3: Approximate Ray Differentials (dDdx, dDdy) ---
    // Without full camera matrices, we estimate how much the ray direction 
    // changes if we move 1 pixel on the screen. 
    // Assuming a standard ~90 degree FOV, the screen width is roughly 2.0 in tangent space.
    Vector3f cam_up = {0.0f, 1.0f, 0.0f};
    
    // Fallback if looking straight up or down to avoid Cross-Product singularity
    if (std::abs(ray.direction.y) > 0.99f) cam_up = {0.0f, 0.0f, 1.0f}; 
    
    Vector3f cam_right = Normalize(Cross(ray.direction, cam_up));
    Vector3f cam_up_real = Cross(cam_right, ray.direction);
    
    // The derivative of the ray direction with respect to screen pixels X and Y
    Vector3f dDdx = cam_right * (2.0f / (float)ViewportSize.x);
    Vector3f dDdy = cam_up_real * (2.0f / (float)ViewportSize.y);
    
    // --- STEP 4: Compute Positional Surface Derivatives (dPdx, dPdy) ---
    // How much does the actual 3D hit point slide along the triangle if 
    // the ray direction shifts by 1 screen pixel?
    float dot_ND = Dot(normal, ray.direction);
    if (std::abs(dot_ND) < 1e-6f) return {.dudx = 0,.dvdx = 0, .dudy = 0,.dvdy = 0}; // Ray is parallel to surface
    float t = Hit.t;
    
    // Analytical derivation of intersection plane gradients
    float dot_N_dDdx = Dot(normal, dDdx);
    Vector3f dPdx = (dDdx - ray.direction * (dot_N_dDdx / dot_ND)) * t;
    float dot_N_dDdy = Dot(normal, dDdy);
    Vector3f dPdy = (dDdy - ray.direction * (dot_N_dDdy / dot_ND)) * t;
    
    // --- STEP 5: Project Positional Derivatives onto Texture Gradients ---
    // Chain Rule: (Change in U per 3D unit) * (Change in 3D units per screen pixel)
    SurfaceSampler derivs;
    derivs.dudx = Dot(grad_U, dPdx);
    derivs.dudy = Dot(grad_U, dPdy);
    derivs.dvdx = Dot(grad_V, dPdx);
    derivs.dvdy = Dot(grad_V, dPdy);
    return derivs;
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
    m_Ray.origin = xyz(t) / t.w;
    
    t = WorldToModel * Vector4f(end, 1.0f);
    end = xyz(t) / t.w;
    
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
    m_Ray.origin = xyz(t) / t.w;

    t = WorldToModel * Vector4f(end, 1.0f);
    end = xyz(t) / t.w;

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

Box3f BLASElementGetBounds(const BLASElement& Element, const BLASDesc& Tree)
{
    Mesh::ConstFaces Faces(*(Tree.MeshRef));
    Mesh::ConstFace face = Faces[Element];
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

Vector3f BLASElementGetCenter(const BLASElement& Element, const BLASDesc& Tree)
{
    Mesh::ConstFaces Faces(*(Tree.MeshRef));
    Mesh::ConstFace face = Faces[Element];
    
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
    
    return Math::Box3f(min, max).Center();
}

BLAS BuildBLAS(const Mesh& Mesh, uint8_t VertexGroup, uint32_t LeafSize)
{
    Mesh::VertexGroup Group = Mesh.GetVertexGroups()[VertexGroup];
    Mesh::VertexType Type = Mesh.GetMeshType();
    
    BLAS blas{};
    blas.Meta = {.VertexGroup = Group, .VertexType = Type, .MeshRef = &Mesh};
    for (uint32_t i = Group.FirstVertex, iend = i + Group.VertexCount, increment = Mesh::FaceVertexIncrement(Type); i < iend; i+=increment)
    {
        blas.Elements.push_back(Mesh::FaceIndex(Type, i));
    }

    blas.LeafSize = LeafSize;
    blas.Rebuild();

    // enable simd
    // when enabled the tree now points to wave elements instead of regular element list
    blas.Meta.Waves.clear();
    switch (blas.Meta.VertexType)
    {            
    case Mesh::TRIANGLE_STRIP_ADJACENCY:
    case Mesh::TRIANGLES_ADJACENCY:
    case Mesh::TRIANGLE_STRIP:
    case Mesh::TRIANGLE_FAN:
    case Mesh::TRIANGLES:
        if (LeafSize % TriangleWave::kThreadCount == 0)
        {
            size_t WaveCount = LeafSize / TriangleWave::kThreadCount;
            
            Mesh::ConstFaces Faces(Mesh);
            std::stack<uint32_t> IterationStack;
            IterationStack.push(blas.Head);
            
            while (!IterationStack.empty())
            {
                uint32_t NodeIndex = IterationStack.top();
                IterationStack.pop();
                
                if (blas.Tree[NodeIndex].IsNode())
                {
                    IterationStack.push(blas.Tree[NodeIndex].LeftIndex());
                    IterationStack.push(blas.Tree[NodeIndex].RightIndex());
                }
                else // if (m_Blas->Tree[NodeIndex].IsLeaf())
                {
                    // collect triangles in bucket
                    std::span<BLASElement> View = {blas.Elements.begin() + blas.Tree[NodeIndex].LeftIndex(), blas.Elements.begin() + blas.Tree[NodeIndex].RightIndex()};
                    size_t begin = blas.Tree[NodeIndex].LeftIndex();
                    size_t end = blas.Tree[NodeIndex].RightIndex();
                    
                    size_t RequiredWaves = View.size() / TriangleWave::kThreadCount + (View.size() % TriangleWave::kThreadCount ? 1 : 0);
                    AssertOrError(RequiredWaves <= WaveCount, "Wave count is too large")
                    
                    size_t firstWave = blas.Meta.Waves.size();
                    blas.Tree[NodeIndex].SetLeafBegin(static_cast<uint32_t>(firstWave));
                    blas.Tree[NodeIndex].SetLeafEnd(static_cast<uint32_t>(firstWave + RequiredWaves));
                    
                    // A leaf can be made of one or more triangle waves
                    for (size_t waveID = 0; waveID < RequiredWaves; waveID++)
                    {
                        TriangleWave& wave = blas.Meta.Waves.emplace_back();
                    
                        // replace tree with
                        wave.BucketBegin = begin + waveID * TriangleWave::kThreadCount;
                        wave.BucketEnd = std::min(end, wave.BucketBegin + TriangleWave::kThreadCount);
                    
                        // fill bucket data
                        for (size_t i = waveID * TriangleWave::kThreadCount, iend = (waveID + 1) * TriangleWave::kThreadCount; i < std::min(View.size(), iend); i++)
                        {
                            size_t local_i = i - waveID * TriangleWave::kThreadCount;
                            Mesh::ConstFace Face = Faces[View[i]];
                        
                            wave.A.x[local_i] = Face.Position(0).x;
                            wave.A.y[local_i] = Face.Position(0).y;
                            wave.A.z[local_i] = Face.Position(0).z;
                        
                            wave.B.x[local_i] = Face.Position(1).x;
                            wave.B.y[local_i] = Face.Position(1).y;
                            wave.B.z[local_i] = Face.Position(1).z;
                        
                            wave.C.x[local_i] = Face.Position(2).x;
                            wave.C.y[local_i] = Face.Position(2).y;
                            wave.C.z[local_i] = Face.Position(2).z;
                        
                            wave.Faces[local_i] = Face.FirstVertex();
                            wave.Validity[local_i] = true;
                        }
                    }
                }
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
    return blas;
}

TraceRayBLAS::TraceRayBLAS(const BLAS& Mesh, const Ray& Ray, const Math::Transform4f& WorldToModel):
    m_Blas(&Mesh),
    m_IterationStack(),
    m_CurrentBVHHit(),
    m_ClosestHit(),
    m_CurrentElementIndex(std::numeric_limits<uint32_t>::max()),
    m_Ray(Ray),
    ModelToWorld(Inverse(WorldToModel))
{
    Vector3f end =  m_Ray.origin + m_Ray.distance * m_Ray.direction;

    Vector4f t = WorldToModel * Vector4f(m_Ray.origin, 1.0f);
    m_Ray.origin = xyz(t) / t.w;

    t = WorldToModel * Vector4f(end, 1.0f);
    end = xyz(t) / t.w;

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
        
        // if SIMD mode
        if (!m_Blas->Meta.Waves.empty())
        {
            // if (m_SIMDHitIndex == std::numeric_limits<uint32_t>::max())
            switch (m_Blas->Meta.VertexType)
            {            
            case Mesh::TRIANGLE_STRIP_ADJACENCY:
            case Mesh::TRIANGLES_ADJACENCY:
            case Mesh::TRIANGLE_STRIP:
            case Mesh::TRIANGLE_FAN:
            case Mesh::TRIANGLES:
                for (uint32_t ElementIndex = m_CurrentElementIndex; ElementIndex < BLASFaceEnd; ElementIndex++)
                {
                    m_SIMDHits = IntersectTriangle(m_Blas->Meta.Waves[ElementIndex], m_Ray);
                    
                    if (m_SIMDHits.IsValid().Any())
                    {
                        uint32_t SIMDHitIndex = IndexOf(m_SIMDHits.t, Lowest(m_SIMDHits.t));
                        Hit hit = m_SIMDHits.Elt(SIMDHitIndex);
                        
                        if (!m_ClosestHit || m_ClosestHit.t > hit.t)
                        {
                            m_ClosestHit = hit;
                            
                            Vector4f originWorld = ModelToWorld * Vector4f(m_Ray.origin, 1.0f); originWorld.xyz() /= originWorld.w;
                            Vector4f tWorld = ModelToWorld * Vector4f(m_Ray.origin + m_ClosestHit.t * m_Ray.direction, 1.0f); tWorld.xyz() /= tWorld.w;
                            m_ClosestHit.t = Magnitude(tWorld - originWorld);
                            
                            m_tmax = std::min(m_tmax, hit.t);
                            m_CurrentElementIndex = ElementIndex + 1;
                            return m_ClosestHit;
                        }
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
            
            m_CurrentBVHHit = {};
            m_CurrentElementIndex = std::numeric_limits<uint32_t>::max();
            goto explore_bvh;
        }
        else
        {
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
                            
                            Vector4f originWorld = ModelToWorld * Vector4f(m_Ray.origin, 1.0f); originWorld.xyz() /= originWorld.w;
                            Vector4f tWorld = ModelToWorld * Vector4f(m_Ray.origin + m_ClosestHit.t * m_Ray.direction, 1.0f); tWorld.xyz() /= tWorld.w;
                            m_ClosestHit.t = Magnitude(tWorld - originWorld);
                            
                            m_tmax = std::min(m_tmax, hit.t);
                            m_CurrentElementIndex = ElementIndex + 1;
                            return m_ClosestHit;
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

Math::Box3f TLASElementGetBounds(const TLASElement& Element, const TLASDesc& Tree)
{
    return SafeTransformBounds(Element.BLAS->Bounds(), Element.ModelToWorld);
}

Math::Vector3f TLASElementGetCenter(const TLASElement& Element, const TLASDesc& Tree)
{
    return Element.Center();
}

void TLASAddInstance(TLAS& BVH, const BLAS& BLAS, size_t Material, const Transform4f& Transform)
{
    BVH.Elements.emplace_back(&BLAS, Transform, Inverse(Transform), Material);
    
    BVH.Invalidate();
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
