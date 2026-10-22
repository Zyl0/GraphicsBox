#include "RayTracing/RayTrace.h"

#include "Modeling/Mesh.h"
#include "Shared/Assertion.h"

using namespace Math;

#define SORT_BLAS_NODE_ITERATION_INSERTION
#define SORT_TLAS_NODE_ITERATION_INSERTION
#define FORCE_STATIC_STACK
#define FORCE_DYNAMIC_STACK

#ifndef CONFIG_RELEASE
#define FORCE_STATIC_STACK_SECURITY_CHECK
#endif // CONFIG_DEBUG

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

SurfaceSampler CalcSurfaceSample(const Mesh& mesh, const Ray& ray, const Hit& Hit, Math::Vector2t<uint32_t> ViewportSize, const Math::Matrix4f* Transform)
{
    if (!mesh.HasNormals() || !mesh.HasTextureCoordinates()) return {.dudx = 0,.dvdx = 0, .dudy = 0,.dvdy = 0}; // Degenerate triangle safety
    
    // TODO hit.face is supposed to return face index but returns face first vertex instead, investigate why.
    // Mesh::ConstFaces faces(mesh);
    // Mesh::ConstFace face = faces[Hit.face];
    
    Mesh::ConstFace face(mesh, Hit.face);
    
    Point3f v0 = face[0].Position(), v1 = face[1].Position(), v2 = face[2].Position();
    if (Transform)
    {
        v0 = ((*Transform) * Vector4f(v0, 1.0f)).xyz();
        v1 = ((*Transform) * Vector4f(v1, 1.0f)).xyz();
        v2 = ((*Transform) * Vector4f(v2, 1.0f)).xyz();
    }
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
    
    float t;
    if (Transform) {
        Vector3f hit_pos = VertexInterpolateTriangle(Hit, v0, v1, v2);
        t = Dot(hit_pos - ray.origin, ray.direction);
    } else {
        t = Hit.t;
    }
    
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
    AssertOrError(FirstVertex < Mesh.GetVertexCount(), "Vertex index out of range")
    AssertOrError(FirstVertex + VertexCount <= Mesh.GetVertexCount(), "Vertex count out of range")

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

TraceRayBLAS::TraceRayBLAS(const BLAS& Mesh, const Ray& Ray, const Hit& ClosestHit, const Math::Transform4f& WorldToModel) :
    m_Ray(Ray),
    m_Blas(&Mesh),
    m_IterationStack(), m_StaticIterationStack(), m_StaticIterationStackElement(std::numeric_limits<uint32_t>::max()),
    m_CurrentBVHHit(), m_ClosestHit(ClosestHit),
    m_PreviousClosestHit(ClosestHit),
    m_CurrentElementIndex(std::numeric_limits<uint32_t>::max()),
    ModelToWorld(Inverse(WorldToModel)),
    RequestInvalidateHit(false)
{
#ifdef FORCE_STATIC_STACK
#ifdef FORCE_STATIC_STACK_SECURITY_CHECK
    AssertOrError(!m_StaticIterationStack.empty(), "Invalid statick stack required when FORCE_STATIC_STACK is enabled")
#endif
#endif // FORCE_STATIC_STACK
    
    Vector3f end = m_Ray.origin + m_Ray.distance * m_Ray.direction;

    Vector4f t = WorldToModel * Vector4f(m_Ray.origin, 1.0f);
    m_Ray.origin = xyz(t) / t.w;

    t = WorldToModel * Vector4f(end, 1.0f);
    end = xyz(t) / t.w;

    if (m_ClosestHit)
    {
        t = WorldToModel * Vector4f(Ray.origin + ClosestHit.t * Ray.distance, 1.0f);
        m_ClosestHit.t = Magnitude((t.xyz() / t.w) - m_Ray.origin);
        m_PreviousClosestHit.t = m_ClosestHit.t;
    }

    m_Ray.distance = Magnitude(end - m_Ray.origin);
    m_Ray.direction = Normalize(end - m_Ray.origin);

    m_tmax = m_ClosestHit ? m_ClosestHit.t : m_Ray.distance;

    m_IterationStack.push(m_Blas->Head);
}

TraceRayBLAS::TraceRayBLAS(const BLAS& Mesh, const Ray& Ray, const Hit& ClosestHit, std::span<uint32_t> IterationStack, const Math::Transform4f& WorldToModel):
    m_Ray(Ray),
    m_Blas(&Mesh),
    m_IterationStack(), m_StaticIterationStack(IterationStack), m_StaticIterationStackElement(0),
    m_CurrentBVHHit(), m_ClosestHit(ClosestHit),
    m_PreviousClosestHit(ClosestHit),
    m_CurrentElementIndex(std::numeric_limits<uint32_t>::max()),
    ModelToWorld(Inverse(WorldToModel)),
    RequestInvalidateHit(false)
{
#ifdef FORCE_STATIC_STACK
#ifdef FORCE_STATIC_STACK_SECURITY_CHECK
    AssertOrError(!IterationStack.empty(), "Invalid statick stack required when FORCE_STATIC_STACK is enabled")
#endif
#endif // FORCE_STATIC_STACK
    
    m_Ray = WorldToModel * Ray;

    if (m_ClosestHit)
    {
        Vector4f t = WorldToModel * Vector4f(Ray.origin + ClosestHit.t * Ray.distance, 1.0f);
        m_ClosestHit.t = Magnitude((t.xyz() / t.w) - m_Ray.origin);
        m_PreviousClosestHit.t = m_ClosestHit.t;
    }

    m_tmax = m_ClosestHit ? m_ClosestHit.t : m_Ray.distance;

    PushStack(m_Blas->Head);
}

Hit TraceRayBLAS::Next()
{
explore_bvh:
    if (!m_CurrentBVHHit)
    while (!StackEmpty()) // TODO reorder iteration to stackless that prioritize branches close to the origin of the tree
    {
        uint32_t NodeIndex = StackTop();
        PopStack();
        
        if (BoxHit hit = IntersectBox(m_Blas->Tree[NodeIndex].Bounds, m_Ray); hit && hit.tmin < m_tmax)
        {
            if (m_Blas->Tree[NodeIndex].IsNode())
            {
#ifdef SORT_BLAS_NODE_ITERATION_INSERTION
                const BLAS::Node& node = m_Blas->Tree[NodeIndex];
                const BLAS::Node& leftNode = m_Blas->Tree[node.LeftIndex()];
                const BLAS::Node& rightNode = m_Blas->Tree[node.RightIndex()];
                
                Vector3f nodeCenter = node.Bounds.Center();
                float dl = SquareDistance(leftNode.Bounds.Center(), nodeCenter);
                float dr = SquareDistance(rightNode.Bounds.Center(), nodeCenter);
                
                if (dl < dr)
                {
                    PushStack(m_Blas->Tree[NodeIndex].RightIndex());
                    PushStack(m_Blas->Tree[NodeIndex].LeftIndex());
                }
                else
                {
                    PushStack(m_Blas->Tree[NodeIndex].LeftIndex());
                    PushStack(m_Blas->Tree[NodeIndex].RightIndex());
                }
#else
                PushStack(m_Blas->Tree[NodeIndex].LeftIndex());
                PushStack(m_Blas->Tree[NodeIndex].RightIndex());
#endif
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
    bool HasInvalidatedHit = false;
    if (RequestInvalidateHit)
    {
        m_ClosestHit = m_PreviousClosestHit;
        m_tmax = m_ClosestHit ? m_ClosestHit.t : m_Ray.distance;
        RequestInvalidateHit = false;
        HasInvalidatedHit = true;
    }
    
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
                // if (HasInvalidatedHit) --m_CurrentElementIndex; // TODO hit regection
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
                            
                            m_tmax = std::min(m_tmax, hit.t);
                            m_CurrentElementIndex = ElementIndex + 1;
                            return ClosestHit();
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
            if (!m_ClosestHit) m_ClosestHit = m_PreviousClosestHit;
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
                            
                            return ClosestHit();
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

Hit TraceRayBLAS::ClosestHit() const
{
    Hit hit = m_ClosestHit;

    if (hit)
    {
        Vector4f originWorld = ModelToWorld * Vector4f(m_Ray.origin, 1.0f); originWorld.xyz() /= originWorld.w;
        Vector4f tWorld = ModelToWorld * Vector4f(m_Ray.origin + m_ClosestHit.t * m_Ray.direction, 1.0f); tWorld.xyz() /= tWorld.w;
        hit.t = Magnitude(tWorld - originWorld);
    }
    
    return hit;
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

void TraceRayBLAS::RejectHit()
{
    RequestInvalidateHit = true;
    m_ClosestHit = {};
}

void TraceRayBLAS::PushStack(uint32_t value)
{
#ifdef FORCE_STATIC_STACK
#ifdef FORCE_STATIC_STACK_SECURITY_CHECK
    AssertOrError(m_StaticIterationStackElement + 1 < m_StaticIterationStack.size(), "Index out of range") // TODO introduce assert tests that only exists in debug builds
#endif // FORCE_STATIC_STACK_SECURITY_CHECK
    
    m_StaticIterationStack[m_StaticIterationStackElement++] = value;
#elif defined(FORCE_DYNAMIC_STACK) 
    m_IterationStack.push(value);
#else
    if (m_StaticIterationStack.empty())
    {
        m_IterationStack.push(value);
    }
    else
    {
        AssertOrError(m_StaticIterationStackElement + 1 < m_StaticIterationStack.size(), "Index out of range") // TODO introduce assert tests that only exists in debug builds
        
        m_StaticIterationStack[m_StaticIterationStackElement++] = value;
    }
#endif
}

uint32_t TraceRayBLAS::StackTop()
{
#ifdef FORCE_STATIC_STACK
#ifdef FORCE_STATIC_STACK_SECURITY_CHECK
    AssertOrError(m_StaticIterationStackElement > 0, "Index out of range")
#endif
    
    return m_StaticIterationStack[m_StaticIterationStackElement - 1];
#elif defined(FORCE_DYNAMIC_STACK)
    return m_IterationStack.top();
#else
    if (m_StaticIterationStack.empty())
    {
        return m_IterationStack.top();
    }
    else
    {
        return m_StaticIterationStack[m_StaticIterationStackElement - 1];
    }
#endif
}

void TraceRayBLAS::PopStack()
{
#ifdef FORCE_STATIC_STACK
#ifdef FORCE_STATIC_STACK_SECURITY_CHECK
    AssertOrError(m_StaticIterationStackElement > 0, "Index out of range")
#endif
    --m_StaticIterationStackElement;
#elif defined(FORCE_DYNAMIC_STACK)
    m_IterationStack.pop();
#else
    if (m_StaticIterationStack.empty())
    {
        m_IterationStack.pop();
    }
    else
    {
        AssertOrError(m_StaticIterationStackElement > 0, "Index out of range")
        --m_StaticIterationStackElement;
    } 
#endif
}

bool TraceRayBLAS::StackEmpty()
{
#ifdef FORCE_STATIC_STACK
    return m_StaticIterationStackElement == 0;
#elif defined(FORCE_DYNAMIC_STACK)
    return m_IterationStack.empty();
#else
    if (m_StaticIterationStack.empty())
    {
        return m_IterationStack.empty();
    }
    else
    {
        return m_StaticIterationStackElement == 0;
    } 
#endif
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

void TLASRebuild(TLAS& BVH)
{
    BVH.Rebuild();
    
    BVH.Meta.HighestBlass = 0;
    for (const TLASElement& element : BVH.Elements)
    {
        BVH.Meta.HighestBlass = std::max(BVH.Meta.HighestBlass, element.BLAS->Height);
    }
}

TraceRayTLAS::TraceRayTLAS(const TLAS& Scene, const Ray& Ray, std::span<uint32_t> IterationStack):
    m_Tlas(&Scene),
    m_IterationStack(), m_StaticIterationStack(IterationStack), m_StaticIterationStackElement(0),
    m_Ray(Ray), m_BlasHit(nullptr), m_ClosestHit()
{
#ifdef FORCE_STATIC_STACK
#ifdef FORCE_STATIC_STACK_SECURITY_CHECK
    AssertOrError(!IterationStack.empty(), "Invalid statick stack required when FORCE_STATIC_STACK is enabled")
#endif
#endif // FORCE_STATIC_STACK
    
    PushStack(m_Tlas->Head);
}

BVHHit TraceRayTLAS::Next()
{
    m_CurrentBVHHit = {};
    
    // TODO reorder iteration to stackless that prioritize branches close to the origin of the tree
    while (!StackEmpty())
    {
        uint32_t NodeIndex = StackTop();
        PopStack();
        
        if (BoxHit hit = IntersectBox(m_Tlas->Tree[NodeIndex].Bounds, m_Ray))
        {
            if (m_ClosestHit && hit.tmin >= m_ClosestHit.t) continue;
            
            if (m_Tlas->Tree[NodeIndex].IsNode())
            {
#ifdef SORT_TLAS_NODE_ITERATION_INSERTION
                const TLAS::Node& node = m_Tlas->Tree[NodeIndex];
                const TLAS::Node& leftNode = m_Tlas->Tree[node.LeftIndex()];
                const TLAS::Node& rightNode = m_Tlas->Tree[node.RightIndex()];
                
                Vector3f nodeCenter = node.Bounds.Center();
                float dl = SquareDistance(leftNode.Bounds.Center(), nodeCenter);
                float dr = SquareDistance(rightNode.Bounds.Center(), nodeCenter);
                
                if (dl < dr)
                {
                    PushStack(m_Tlas->Tree[NodeIndex].RightIndex());
                    PushStack(m_Tlas->Tree[NodeIndex].LeftIndex());
                }
                else
                {
                    PushStack(m_Tlas->Tree[NodeIndex].LeftIndex());
                    PushStack(m_Tlas->Tree[NodeIndex].RightIndex());
                }
#else
                PushStack(m_Tlas->Tree[NodeIndex].LeftIndex());
                PushStack(m_Tlas->Tree[NodeIndex].RightIndex());
#endif
            }
            else // if (m_Blas->Tree[NodeIndex].IsLeaf())
            {
                m_CurrentBVHHit = BVHHit(NodeIndex, hit);
                return m_CurrentBVHHit;
            }
        }
    }
    
    return BVHHit();
}

const TLASElement* TraceRayTLAS::Element() const
{
    if (!m_CurrentBVHHit) return nullptr;

    return &(m_Tlas->Elements[m_Tlas->Tree[m_CurrentBVHHit.NodeIndex].LeftIndex()]);
}

void TraceRayTLAS::AcceptHit(Hit hit)
{
    if (!hit || !m_CurrentBVHHit) return;

    if (!m_ClosestHit || hit.t < m_ClosestHit.t)
    {
        m_ClosestHit = hit;
        m_BlasHit = Element();
    }
}

void TraceRayTLAS::PushStack(uint32_t value)
{
#ifdef FORCE_STATIC_STACK
#ifdef FORCE_STATIC_STACK_SECURITY_CHECK
        AssertOrError(m_StaticIterationStackElement + 1 < m_StaticIterationStack.size(), "Index out of range")
#endif
    m_StaticIterationStack[m_StaticIterationStackElement++] = value;
#elif defined(FORCE_DYNAMIC_STACK)
    m_IterationStack.push(value);
#else
    if (m_StaticIterationStack.empty())
    {
        m_IterationStack.push(value);
    }
    else
    {
        AssertOrError(m_StaticIterationStackElement + 1 < m_StaticIterationStack.size(), "Index out of range")
        
        m_StaticIterationStack[m_StaticIterationStackElement++] = value;
    }
#endif
}

uint32_t TraceRayTLAS::StackTop()
{
#ifdef FORCE_STATIC_STACK
#ifdef FORCE_STATIC_STACK_SECURITY_CHECK
    AssertOrError(m_StaticIterationStackElement > 0, "Index out of range")
#endif
    
    return m_StaticIterationStack[m_StaticIterationStackElement - 1];
#elif defined(FORCE_DYNAMIC_STACK)
    return m_IterationStack.top();
#else
    if (m_StaticIterationStack.empty())
    {
        return m_IterationStack.top();
    }
    else
    {
        return m_StaticIterationStack[m_StaticIterationStackElement - 1];
    }
#endif
}

void TraceRayTLAS::PopStack()
{
#ifdef FORCE_STATIC_STACK
#ifdef FORCE_STATIC_STACK_SECURITY_CHECK
    AssertOrError(m_StaticIterationStackElement > 0, "Index out of range")
#endif
    --m_StaticIterationStackElement;
#elif defined(FORCE_DYNAMIC_STACK)
    m_IterationStack.pop();
#else
    if (m_StaticIterationStack.empty())
    {
        m_IterationStack.pop();
    }
    else
    {
        AssertOrError(m_StaticIterationStackElement > 0, "Index out of range")
        --m_StaticIterationStackElement;
    } 
#endif
}

bool TraceRayTLAS::StackEmpty()
{
#ifdef FORCE_STATIC_STACK
    return m_StaticIterationStackElement == 0;
#elif defined(FORCE_DYNAMIC_STACK)
    return m_IterationStack.empty();
#else
    if (m_StaticIterationStack.empty())
    {
        return m_IterationStack.empty();
    }
    else
    {
        return m_StaticIterationStackElement == 0;
    } 
#endif
}

BVHIterationStack::BVHIterationStack(uint32_t StackCount, uint32_t TLASHeight, uint32_t BLASHeight): 
    m_Data(), 
    m_StackCount(StackCount), 
    m_StackTLASHeight(TLASHeight),
    m_StackBLASHeight(BLASHeight)
{
    // PLATFORM_ALIGNED_MALLOC(uint32_t, m_Data, alignof(uint32_t), static_cast<size_t>(m_StackCount) * (TLASHeight + BLASHeight))
    m_Data = (uint32_t*)malloc(static_cast<size_t>(m_StackCount) * (TLASHeight + BLASHeight) * sizeof(uint32_t));
}

BVHIterationStack::~BVHIterationStack()
{
    // PLATFORM_ALIGNED_FREE(m_Data)
    free(m_Data);
}

std::span<uint32_t> BVHIterationStack::BLAS(uint32_t StackId)
{
    AssertOrErrorCall(StackId < m_StackCount, return std::span<uint32_t>{}, "Stack index out of range")
    
    size_t blases_offset = static_cast<size_t>(m_StackCount) * m_StackTLASHeight;
    size_t begin = blases_offset + StackId * m_StackBLASHeight;
    size_t end = blases_offset + (StackId + 1) * m_StackBLASHeight;
    
    AssertOrError(end <= static_cast<size_t>(m_StackCount) * (m_StackTLASHeight + m_StackBLASHeight), "Stack memory overflow")
    
    return std::span<uint32_t>{m_Data + begin, end - begin}; 
}

std::span<uint32_t> BVHIterationStack::TLAS(uint32_t StackId)
{
    AssertOrErrorCall(StackId < m_StackCount, return std::span<uint32_t>{}, "Stack index out of range")

    size_t begin = StackId * m_StackTLASHeight;
    size_t end = (StackId + 1) * m_StackTLASHeight;
    
    AssertOrError(end <= static_cast<size_t>(m_StackCount) * (m_StackTLASHeight + m_StackBLASHeight), "Stack memory overflow")
    
    return std::span<uint32_t>{m_Data + begin, end - begin}; 
}

TraceRayBVH::TraceRayBVH(const TLAS& Scene, const Ray& Ray, BVHIterationStack* IterationStack, uint32_t StackIndex) : 
    m_Tlas(nullptr),
    m_TraceRayTLAS(Scene, Ray, IterationStack ? IterationStack->TLAS(StackIndex) : std::span<uint32_t>{}),
    m_TLASCurrent(m_TraceRayTLAS.begin()), m_TLASEnd(m_TraceRayTLAS.end()),
    m_IterationStackBlas(IterationStack ? IterationStack->BLAS(StackIndex) : std::span<uint32_t>{})
{
}

Hit TraceRayBVH::Next()
{
    for (TraceRayTLAS::iterator& it = m_TLASCurrent; it != m_TLASEnd; ++it)
    {
        BVHHit hit = *it;

        // TODO BLAS early test
        
        const TLASElement& Elt = *m_TraceRayTLAS.Element();
        
        if (!m_TraceRayBLAS.has_value())
        {
            m_TraceRayBLAS.emplace(*(Elt.BLAS), m_TraceRayTLAS.GetRay(), m_TraceRayTLAS.ClosestHit(), m_IterationStackBlas, Elt.WorldToModel);
            m_BLASCurrent = m_TraceRayBLAS->begin();
            m_BLASEnd = m_TraceRayBLAS->end();
        }
        else
        {
            ++m_BLASCurrent;
        }
        
        if (m_BLASCurrent != m_BLASEnd)
        {
            return *m_BLASCurrent;
        }
        
        m_TraceRayTLAS.AcceptHit(m_TraceRayBLAS->ClosestHit());
        
        m_TraceRayBLAS.reset();
        m_BLASCurrent = {};
        m_BLASEnd = {};
    }
    
    return Hit();
}

TraceRayBVH::iterator::iterator(TraceRayBVH& TraceRay)
{
    if (Hit hit = TraceRay.Next())
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

TraceRayBVH::iterator& TraceRayBVH::iterator::operator++()
{
    if (m_TraceRay == nullptr) return *this;

    if (Hit hit = m_TraceRay->Next())
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

