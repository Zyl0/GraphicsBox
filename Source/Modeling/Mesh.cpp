#include <cmath>

#include "Modeling/Mesh.h"

#include "Shared/Logger.h"
#include "Shared/Assertion.h"

unsigned int Mesh::GetFaceCount() const
{
    switch (GetMeshType())
    {
    case POINTS:
    case LINE_STRIP:
    case LINE_LOOP:
    case LINES:
    case LINE_STRIP_ADJACENCY:
    case LINES_ADJACENCY:
        AssertOrWarnCall(false, return 0, "This geometry type is not made of faces.")
        
    case PATCHES:
    case TRIANGLE_STRIP_ADJACENCY:
    case TRIANGLES_ADJACENCY:
        AssertOrWarnCall(false, return 0,  "PATCHES, TRIANGLE_STRIP_ADJACENCY and TRIANGLES_ADJACENCY geometry types is not supported, unimplemented.")
        
    case TRIANGLE_STRIP:
    case TRIANGLE_FAN:
        return static_cast<unsigned>( m_positions.size() > 2ul ? m_positions.size() - 2ull : 0);
    case QUAD_STRIP:
        return static_cast<unsigned>( m_positions.size() > 3ul ? m_positions.size() - 3ull : 0);
    case TRIANGLES:
        return static_cast<unsigned>(m_positions.size() / 3ul);
    case QUADS:
        return static_cast<unsigned>(m_positions.size() / 4ul);
        
    case _Count:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported face type")
    }
}

unsigned int Mesh::GetVertexCount() const
{
    if (IsIndexedMesh())
        return m_indexes.size();
    return m_positions.size();
}

void Mesh::BeginMesh(VertexType meshType)
{
    AssertOrError(meshType != 0, "Invalid GL Mesh Type")
    m_mesh_type = meshType;
    
    m_positions.clear();
    m_normals.clear();
    m_texture_coordinates.clear();
    m_indexes.clear();
    m_vertex_group.clear();
    
    bIsInEditMode = true;
}

void Mesh::EditMesh()
{
    AssertOrWarnCall(!bIsInEditMode, return, "Attempted to edit a mesh that is already in edit mode.")
    bIsInEditMode = true;
}

void Mesh::AddVertexPosition(const Math::Point3f& p)
{
    AssertOrError(bIsInEditMode, "Attempted to edit a mesh that is not in edit mode.")

    m_positions.push_back(p);
}

void Mesh::AddVertexNormal(const Math::Vector3f& n)
{
    AssertOrError(bIsInEditMode, "Attempted to edit a mesh that is not in edit mode.")

    m_normals.push_back(n);
}

void Mesh::AddVertexTextureCoordinate(const Math::Vector2f& uv)
{
    AssertOrError(bIsInEditMode, "Attempted to edit a mesh that is not in edit mode.")

    m_texture_coordinates.push_back(uv);
}

void Mesh::AddVertexGroup(unsigned first, unsigned count)
{
    AssertOrError(bIsInEditMode, "Attempted to edit a mesh that is not in edit mode.")

    m_vertex_group.push_back({first, count});
}

void Mesh::AddVertexPolygonIndex(unsigned index)
{
    AssertOrError(bIsInEditMode, "Attempted to edit a mesh that is not in edit mode.")

    m_indexes.push_back(index);
}

void Mesh::SetVertexPosition(unsigned i, const Math::Point3f& p)
{
    AssertOrError(bIsInEditMode, "Attempted to edit a mesh that is not in edit mode.")
    AssertOrError(i < m_positions.size(), "Vertex index out of range.")
    
    m_positions[i] = p;
}

void Mesh::SetVertexNormal(unsigned i, const Math::Vector3f& n)
{
    AssertOrError(bIsInEditMode, "Attempted to edit a mesh that is not in edit mode.")
    AssertOrError(i < m_normals.size(), "Vertex index out of range.")

    m_normals[i] = n;
}

void Mesh::SetVertexTextureCoordinate(unsigned i, const Math::Vector2f& uv)
{
    AssertOrError(bIsInEditMode, "Attempted to edit a mesh that is not in edit mode.")
    AssertOrError(i < m_texture_coordinates.size(), "Vertex index out of range.")

    m_texture_coordinates[i] = uv;
}

void Mesh::SetVertexPolygonIndex(unsigned i, unsigned index)
{
    AssertOrError(bIsInEditMode, "Attempted to edit a mesh that is not in edit mode.")
    AssertOrError(i < m_indexes.size(), "Index out of range.")

    m_indexes[i] = index;
}

void Mesh::SetVertexGroup(unsigned i, unsigned first, unsigned count)
{
    AssertOrError(bIsInEditMode, "Attempted to edit a mesh that is not in edit mode.")
    AssertOrError(i < m_vertex_group.size(), "Vertex group index out of range.")

    m_vertex_group[i].FirstVertex = first;
    m_vertex_group[i].VertexCount = count;
}

void Mesh::GenerateNormals()
{
    AssertOrError(bIsInEditMode, "Attempted to edit a mesh that is not in edit mode.");

    //TODO complete normal generation implementation
    
    switch (m_mesh_type)
    {
    case TRIANGLES:
    case TRIANGLE_STRIP_ADJACENCY:
    case TRIANGLES_ADJACENCY:
    case TRIANGLE_STRIP:
    case TRIANGLE_FAN:
        {
            // Ensure the size is right
            m_normals.resize(m_positions.size());
            
            Faces faces(*this);
            
            for (Face face : faces)
            {
                Vertex va = face[0];
                Vertex vb = face[1];
                Vertex vc = face[1];
                
                Math::Point3f a = va.Position();
                Math::Point3f b = vb.Position();
                Math::Point3f c = vc.Position();

                //TODO check triangle position order regarding cross product
                Math::Vector3f ab = b - a;
                Math::Vector3f ac = c - a;
                va.Normal() = Math::Normalize(Math::Cross(ab, ac));
            
                Math::Vector3f ba = a - b;
                Math::Vector3f bc = c - b;
                vb.Normal() = Math::Normalize(Math::Cross(ba, bc));

                Math::Vector3f cb = b - c;
                Math::Vector3f ca = a - c;
                vc.Normal() = Math::Normalize(Math::Cross(cb, ca));

                //todo implement soft normal calculation from laplacian
        
            }
        }
        break;
        
    case POINTS:
    case LINE_STRIP:
    case LINE_LOOP:
    case LINES:
    case LINE_STRIP_ADJACENCY:
    case LINES_ADJACENCY:
    case QUADS:
    case QUAD_STRIP:
    case PATCHES:
    case _Count:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported geometry type for normal generation");
    }
}

Math::Vector3f GenerateVertexTangentForTriangle(
    const Math::Vector3f &A,
    const Math::Vector3f &B,
    const Math::Vector3f &C,
    const Math::Vector2f &UVA,
    const Math::Vector2f &UVB,
    const Math::Vector2f &UVC
    )
{
    Math::Vector3f ab = B - A;
    Math::Vector3f ac = C - A;

    Math::Vector2f deltaUV1 = UVB - UVA;
    Math::Vector2f deltaUV2 = UVC - UVA;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
    
    Math::Vector3f t;
    t.x = f * (deltaUV2.y * ab.x - deltaUV1.y * ac.x);
    t.y = f * (deltaUV2.y * ab.y - deltaUV1.y * ac.y);
    t.z = f * (deltaUV2.y * ab.z - deltaUV1.y * ac.z);
    
    t = Normalize(t);
    return t;
}

void Mesh::GenerateTangents()
{
    if(m_normals.empty()) return;
    
    AssertOrError(bIsInEditMode, "Attempted to edit a mesh that is not in edit mode.");

    AssertOrWarnCall(m_tangents.empty(), return;, "Tangents already exists, clear tangents before trying to generate them.")

    switch (GetMeshType())
    {
    case POINTS:
    case LINE_STRIP:
    case LINE_LOOP:
    case LINES:
    case LINE_STRIP_ADJACENCY:
    case LINES_ADJACENCY:
        EngineLoggerError("Cannot generate tangents for mesh as this surface type is not made of faces.");
        m_tangents.resize(m_normals.size());
        for (size_t i = 0; i < m_normals.size(); ++i)
        {
            m_tangents[i] = {0,0,0};
        }
        break;
        
    case PATCHES:
    case TRIANGLE_STRIP_ADJACENCY:
    case TRIANGLES_ADJACENCY:
        EngineLoggerError("Cannot generate tangents for mesh, TRIANGLE_STRIP_ADJACENCY and TRIANGLES_ADJACENCY geometry types is not supported for tangent generation.");
        m_tangents.resize(m_normals.size());
        for (size_t i = 0; i < m_normals.size(); ++i)
        {
            m_tangents[i] = {0,0,0};
        }
        break;
        
    case TRIANGLE_FAN:
    case TRIANGLE_STRIP:
    case TRIANGLES:
        m_tangents.resize(m_normals.size());
        
        {
            Faces faces(*this);
            for (auto face : faces)
            {
                Math::Vector3f a = face[0].Position();
                Math::Vector3f b = face[1].Position();
                Math::Vector3f c = face[2].Position();

                Math::Vector2f UVA = face[0].TextureCoordinate();
                Math::Vector2f UVB = face[1].TextureCoordinate();
                Math::Vector2f UVC = face[2].TextureCoordinate();

                face[0].Tangent() = GenerateVertexTangentForTriangle(a, b, c, UVA, UVB, UVC);
                face[1].Tangent() = GenerateVertexTangentForTriangle(b, c, a, UVB, UVC, UVA);
                face[2].Tangent() = GenerateVertexTangentForTriangle(c, a, b, UVC, UVA, UVB);
            }
        }
        break;
        
    case QUADS:
    case QUAD_STRIP:
        m_tangents.resize(m_normals.size());
        
        {
            Faces faces(*this);
            for (auto face : faces)
            {
                Math::Vector3f a = face[0].Position();
                Math::Vector3f b = face[1].Position();
                Math::Vector3f c = face[2].Position();
                Math::Vector3f d = face[3].Position();

                Math::Vector2f UVA = face[0].TextureCoordinate();
                Math::Vector2f UVB = face[1].TextureCoordinate();
                Math::Vector2f UVC = face[2].TextureCoordinate();
                Math::Vector2f UVD = face[3].TextureCoordinate();

                //First triangle
                face[0].Tangent() = GenerateVertexTangentForTriangle(a, b, c, UVA, UVB, UVC);
                face[1].Tangent() = GenerateVertexTangentForTriangle(b, c, a, UVB, UVC, UVA);
                face[2].Tangent() = GenerateVertexTangentForTriangle(c, a, b, UVC, UVA, UVB);

                //Second triangle
                face[3].Tangent() = GenerateVertexTangentForTriangle(d, b, a, UVD, UVB, UVA);
            }
        }
        break;
        
    default: UNREACHABLE;
    }
}

void Mesh::ClearTangents()
{
    AssertOrError(bIsInEditMode, "Attempted to edit a mesh that is not in edit mode.");
    m_tangents.clear();
}

Math::Point3f& Mesh::Vertex::Position()
{
    AssertOrError(m_MeshReference.bIsInEditMode, "Attempted to edit a mesh that is not in edit mode.");
    if (m_MeshReference.IsIndexedMesh())
        return m_MeshReference.m_positions[m_MeshReference.m_indexes[m_Vertex]];
    return m_MeshReference.m_positions[m_Vertex];
}

const Math::Point3f& Mesh::Vertex::Position() const
{
    if (m_MeshReference.IsIndexedMesh())
        return m_MeshReference.m_positions[m_MeshReference.m_indexes[m_Vertex]];
    return m_MeshReference.m_positions[m_Vertex];
}

Math::Vector3f& Mesh::Vertex::Normal()
{
    AssertOrError(m_MeshReference.bIsInEditMode, "Attempted to edit a mesh that is not in edit mode.");
    if (m_MeshReference.IsIndexedMesh())
        return m_MeshReference.m_normals[m_MeshReference.m_indexes[m_Vertex]];
    return m_MeshReference.m_normals[m_Vertex];
}

const Math::Vector3f& Mesh::Vertex::Normal() const
{
    if (m_MeshReference.IsIndexedMesh())
        return m_MeshReference.m_normals[m_MeshReference.m_indexes[m_Vertex]];
    return m_MeshReference.m_normals[m_Vertex];
}

Math::Vector3f& Mesh::Vertex::Tangent()
{
    AssertOrError(m_MeshReference.bIsInEditMode, "Attempted to edit a mesh that is not in edit mode.");
    if (m_MeshReference.IsIndexedMesh())
        return m_MeshReference.m_tangents[m_MeshReference.m_indexes[m_Vertex]];
    return m_MeshReference.m_tangents[m_Vertex];
}

const Math::Vector3f& Mesh::Vertex::Tangent() const
{
    if (m_MeshReference.IsIndexedMesh())
        return m_MeshReference.m_tangents[m_MeshReference.m_indexes[m_Vertex]];
    return m_MeshReference.m_tangents[m_Vertex];
}

Math::Vector2f& Mesh::Vertex::TextureCoordinate()
{
    AssertOrError(m_MeshReference.bIsInEditMode, "Attempted to edit a mesh that is not in edit mode.");
    if (m_MeshReference.IsIndexedMesh())
        return m_MeshReference.m_texture_coordinates[m_MeshReference.m_indexes[m_Vertex]];
    return m_MeshReference.m_texture_coordinates[m_Vertex];
}

const Math::Vector2f& Mesh::Vertex::TextureCoordinate() const
{
    if (m_MeshReference.IsIndexedMesh())
        return m_MeshReference.m_texture_coordinates[m_MeshReference.m_indexes[m_Vertex]];
    return m_MeshReference.m_texture_coordinates[m_Vertex];
}

Mesh::Vertex Mesh::Face::GetVertex(uint8_t index)
{
    AssertFaceReadable(index);
    if(m_MeshReference.GetMeshType() == TRIANGLE_FAN && index == 0)
        return Vertex(m_MeshReference, 0);
    return Vertex(m_MeshReference, m_FirstVertex + index);
}

const Mesh::Vertex Mesh::Face::GetVertex(uint8_t index) const
{
    AssertFaceReadable(index);
    if(m_MeshReference.GetMeshType() == TRIANGLE_FAN && index == 0)
        return Vertex(m_MeshReference, 0);
    return Vertex(m_MeshReference, m_FirstVertex + index);
}

Mesh::Vertex::iterator Mesh::Face::begin()
{
    switch (m_MeshReference.GetMeshType())
    {
    case POINTS:
    case LINE_STRIP:
    case LINE_LOOP:
    case LINES:
    case LINE_STRIP_ADJACENCY:
    case LINES_ADJACENCY:
        AssertOrError(false, "Cannot iterate on faces. This geometry type is not made of faces.");
        
    case PATCHES:
    case TRIANGLE_STRIP_ADJACENCY:
    case TRIANGLES_ADJACENCY:
        AssertOrError(false, "Cannot iterate on faces. PATCHES, TRIANGLE_STRIP_ADJACENCY and TRIANGLES_ADJACENCY geometry types is not supported for face iteration.");
        
    case TRIANGLE_STRIP:
    case TRIANGLE_FAN:
    case QUAD_STRIP:
    case TRIANGLES:
    case QUADS:
        return Vertex::iterator(m_MeshReference, m_FirstVertex);
        
    default: UNREACHABLE;
    }

    AssertOrError(false, "Cannot iterate on faces. Unimplemented geometry type.")
}

Mesh::Vertex::iterator Mesh::Face::end()
{
    switch (m_MeshReference.GetMeshType())
    {
    case POINTS:
    case LINE_STRIP:
    case LINE_LOOP:
    case LINES:
    case LINE_STRIP_ADJACENCY:
    case LINES_ADJACENCY:
        AssertOrError(false, "Cannot iterate on faces. This geometry type is not made of faces.");
        
    case PATCHES:
    case TRIANGLE_STRIP_ADJACENCY:
    case TRIANGLES_ADJACENCY:
        AssertOrError(false, "Cannot iterate on faces. PATCHES, TRIANGLE_STRIP_ADJACENCY and TRIANGLES_ADJACENCY geometry types is not supported for face iteration.")
        
    case TRIANGLES:
    case TRIANGLE_STRIP:
    case TRIANGLE_FAN:
        return Vertex::iterator(m_MeshReference, m_FirstVertex + 3);
        
    case QUADS:
    case QUAD_STRIP:
        return Vertex::iterator(m_MeshReference, m_FirstVertex + 4);
        
    default: UNREACHABLE;
    }

    AssertOrError(false, "Cannot iterate on faces. Unimplemented geometry type.")
}

Mesh::Vertex::iterator Mesh::Face::last()
{
    switch (m_MeshReference.GetMeshType())
    {
    case POINTS:
    case LINE_STRIP:
    case LINE_LOOP:
    case LINES:
    case LINE_STRIP_ADJACENCY:
    case LINES_ADJACENCY:
        AssertOrError(false, "Cannot iterate on faces. This geometry type is not made of faces.");
        
    case PATCHES:
    case TRIANGLE_STRIP_ADJACENCY:
    case TRIANGLES_ADJACENCY:
        AssertOrError(false, "Cannot iterate on faces. PATCHES, TRIANGLE_STRIP_ADJACENCY and TRIANGLES_ADJACENCY geometry types is not supported for face iteration.");
        
    case TRIANGLES:
    case TRIANGLE_STRIP:
    case TRIANGLE_FAN:
        return Vertex::iterator(m_MeshReference, m_FirstVertex + 2);

    case QUADS:
    case QUAD_STRIP:
        return Vertex::iterator(m_MeshReference, m_FirstVertex + 3);
        
    default: UNREACHABLE;
    }

    AssertOrError(false, "Cannot iterate on faces. Unimplemented geometry type.")
}

void Mesh::Face::AssertFaceReadable(uint8_t vertex) const
{
    switch (m_MeshReference.GetMeshType())
    {
    case POINTS:
    case LINE_STRIP:
    case LINE_LOOP:
    case LINES:
    case LINE_STRIP_ADJACENCY:
    case LINES_ADJACENCY:
        AssertOrError(false, "Cannot access face vertex. This geometry type is not made of faces");
        
    case PATCHES:
    case TRIANGLE_STRIP_ADJACENCY:
    case TRIANGLES_ADJACENCY:
        AssertOrError(false, "Cannot access face vertex. PATCHES, TRIANGLE_STRIP_ADJACENCY and TRIANGLES_ADJACENCY geometry types is not supported for face iteration");
        
    case TRIANGLE_STRIP:
    case TRIANGLE_FAN:
    case TRIANGLES:
        AssertOrErrorF(vertex < 3, "Cannot access triangle vertex number %u. A quad is only made of 4 vertices", vertex)
        break;

    case QUAD_STRIP:
    case QUADS:
        AssertOrErrorF(vertex < 4, "Cannot access quad vertex number %u. A quad is only made of 4 vertices", vertex)
        break;
        
    default:
        AssertOrError(false, "Cannot access face vertex. Unimplemented geometry type.");
    }
}

Mesh::Face::FaceIterator& Mesh::Face::FaceIterator::operator++()
{
    switch (m_MeshReference.GetMeshType())
    {
    case POINTS:
    case LINE_STRIP:
    case LINE_LOOP:
    case LINES:
    case LINE_STRIP_ADJACENCY:
    case LINES_ADJACENCY:
        AssertOrError(false, "Cannot iterate on faces. This geometry type is not made of faces.");
        
    case PATCHES:
    case TRIANGLE_STRIP_ADJACENCY:
    case TRIANGLES_ADJACENCY:
        AssertOrError(false, "Cannot iterate on faces. PATCHES, TRIANGLE_STRIP_ADJACENCY and TRIANGLES_ADJACENCY geometry types is not supported for face iteration.");
        
    case TRIANGLE_STRIP:
    case TRIANGLE_FAN:
    case QUAD_STRIP:
        m_FirstVertex++; return *this;
        
    case TRIANGLES:
        m_FirstVertex+=3; return *this;

    case QUADS:
        m_FirstVertex+=4; return *this;
        
    default: UNREACHABLE;
    }

    AssertOrError(false, "Cannot iterate on faces. Unimplemented geometry type.")
}

void Mesh::CommitMesh()
{
    AssertOrWarnCall(bIsInEditMode, return, "Committed mesh that was not in edit mode.");
    AssertOrWarnCall(!m_positions.empty(), bIsInEditMode = false; return;, "Attempted to generate gpu mesh buffers of an empty mesh.");
    AssertOrError(m_normals.empty()             || m_positions.size() == m_normals.size(), "Impossible to commit mesh. Mesh vertex buffers missmatch.");
    AssertOrError(m_texture_coordinates.empty() || m_positions.size() == m_texture_coordinates.size(), "Impossible to commit mesh. Mesh vertex buffers missmatch.");

    if(!m_texture_coordinates.empty() && m_tangents.empty())
        GenerateTangents();
    
    bIsInEditMode = false;
    
    if(m_vertex_group.empty())
        m_vertex_group.push_back({0,  IsIndexedMesh() ? static_cast<unsigned int>(m_indexes.size()) : static_cast<unsigned int>(m_positions.size())});

    GenerateBounds();    
}

void Mesh::GenerateBounds()
{
    for (auto & Group : m_vertex_group)
    {
        if(Group.VertexCount == 0)
        {
            Group.BoundsMin = Math::Point3f();
            Group.BoundsMax = Math::Point3f();
            continue;
        }

        Group.BoundsMin = m_positions[Group.FirstVertex];
        Group.BoundsMax = m_positions[Group.FirstVertex];

        for(size_t i = Group.FirstVertex + 1, iend = Group.FirstVertex + Group.VertexCount; i < iend; i++)
        {
            Math::Point3f p = m_positions[i];

            Group.BoundsMin.x = std::min(Group.BoundsMin.x, p.x);
            Group.BoundsMin.y = std::min(Group.BoundsMin.y, p.y);
            Group.BoundsMin.z = std::min(Group.BoundsMin.z, p.z);

            Group.BoundsMax.x = std::max(Group.BoundsMax.x, p.x);
            Group.BoundsMax.y = std::max(Group.BoundsMax.y, p.y);
            Group.BoundsMax.z = std::max(Group.BoundsMax.z, p.z);
        }
    }
}