#include "MeshObject.h"

void MeshObject::Data(const Mesh& Mesh)
{
    AssertOrErrorCall(!Mesh.GetPositions().empty(), return;, "Attempted to generate gpu mesh buffers of an empty mesh.");

    BeginMesh(Mesh.GetMeshType());
    ClearVertexBuffers();
    {
        VertexArrayObject::Layout layout{}; layout.Push<Math::Point3f>(1);
        AddVertexBuffer(layout, &(Mesh.GetPositions().data()->x), Mesh.GetPositions().size() * sizeof(Math::Point3f));
    }
    if(Mesh.HasNormals())
    {
        VertexArrayObject::Layout layout{}; layout.Push<Math::Vector3f>(1);
        AddVertexBuffer(layout, &(Mesh.GetNormals().data()->x), Mesh.GetNormals().size() * sizeof(Math::Vector3f));
    }
    if(Mesh.HasTangents())
    {
        VertexArrayObject::Layout layout{}; layout.Push<Math::Vector3f>(1);
        AddVertexBuffer(layout, &(Mesh.GetTangents().data()->x), Mesh.GetTangents().size() * sizeof(Math::Vector3f));
    }
    if(Mesh.HasTextureCoordinates())
    {
        VertexArrayObject::Layout layout{}; layout.Push<Math::Vector2f>(1);
        AddVertexBuffer(layout, &(Mesh.GetTextureCoordinates().data()->x), Mesh.GetTextureCoordinates().size() * sizeof(Math::Vector2f));
    }
    if(Mesh.IsIndexedMesh())
    {
        SetIndexBuffer(IndexBuffer::UnsignedInt, Mesh.GetIndices().data(), Mesh.GetIndices().size());
    }
    else
    {
        UnsetIndexBuffer();
    }
    EndMesh();
}

void MeshObject::BeginMesh(Mesh::VertexType meshType)
{
    m_EditMode = true;
    m_VertexType = meshType;
}

size_t MeshObject::AddVertexBuffer(const VertexArrayObject::Layout& layout, const void* data, size_t size)
{
    AssertOrErrorCall(m_EditMode, return std::numeric_limits<size_t>::max();, "Could not perform AddVertexBuffer, Mesh Data object needs to be in edit mode to perform this operation")

    size_t index = m_VertexBuffers.size();
    m_VertexBuffers.emplace_back();
    m_VertexBuffers.back().Data(data, size);
    m_Layouts.push_back(layout);
    
    return index;
}

size_t MeshObject::AddVertexBuffer(const VertexArrayObject::Layout& layout, size_t size)
{
    AssertOrErrorCall(m_EditMode, return std::numeric_limits<size_t>::max();, "Could not perform AddVertexBuffer, Mesh Data object needs to be in edit mode to perform this operation")
    
    size_t index = m_VertexBuffers.size();
    m_VertexBuffers.emplace_back();
    m_VertexBuffers.back().Data(nullptr, size);
    m_Layouts.push_back(layout);
    
    return index;
}

void MeshObject::SetVertexBuffer(size_t index, const void* data, size_t size)
{
    AssertOrErrorCall(m_EditMode, return;, "Could not perform SetVertexBuffer, Mesh Data object needs to be in edit mode to perform this operation")
    AssertOrErrorCall(index < m_VertexBuffers.size(), return;, "Index out of bounds")
    
    m_VertexBuffers[index].Data(data, size);
}

void MeshObject::SetVertexBuffer(size_t index, const VertexArrayObject::Layout& layout, const void* data, size_t size)
{
    AssertOrErrorCall(m_EditMode, return;, "Could not perform SetVertexBuffer, Mesh Data object needs to be in edit mode to perform this operation")
    AssertOrErrorCall(index < m_VertexBuffers.size(), return;, "Index out of bounds")
    
    m_VertexBuffers[index].Data(data, size);
    m_Layouts[index] = layout;
}

void MeshObject::SetVertexSubBuffer(size_t index, const void* data, size_t offset, size_t size)
{
    AssertOrErrorCall(m_EditMode, return;, "Could not perform SetVertexBuffer, Mesh Data object needs to be in edit mode to perform this operation")
    AssertOrErrorCall(index < m_VertexBuffers.size(), return;, "Index out of bounds")
    
    m_VertexBuffers[index].SubData(data, offset, size);
}

void MeshObject::PopVertexBuffer()
{
    AssertOrErrorCall(m_EditMode, return;, "Could not perform PopVertexBuffer, Mesh Data object needs to be in edit mode to perform this operation")
    AssertOrWarnCall(!m_VertexBuffers.empty(), return, "Attempted to pop an empty vertex buffer")
    
    m_Layouts.pop_back();
    m_VertexBuffers.pop_back();
}

void MeshObject::ClearVertexBuffers()
{
    AssertOrErrorCall(m_EditMode, return;, "Could not perform ClearVertexBuffers, Mesh Data object needs to be in edit mode to perform this operation")
    AssertOrWarnCall(!m_VertexBuffers.empty(), return, "Attempted to pop an empty vertex buffer")
    
    m_Layouts.clear();
    m_VertexBuffers.clear();
}

void MeshObject::SetIndexBuffer(IndexBuffer::IndexType Type, const void* data, unsigned int count)
{
    AssertOrErrorCall(m_EditMode, return;, "Could not perform SetIndexBuffer, Mesh Data object needs to be in edit mode to perform this operation")
    
    if (!m_IndexBuffer.has_value())
    {
        m_IndexBuffer.emplace();
    }
    m_IndexBuffer.value().BufferData(Type, data, count);
}

void MeshObject::SetIndexBuffer(IndexBuffer::IndexType Type, unsigned int count)
{
    AssertOrErrorCall(m_EditMode, return;, "Could not perform SetIndexBuffer, Mesh Data object needs to be in edit mode to perform this operation")
    
    if (!m_IndexBuffer.has_value())
    {
        m_IndexBuffer.emplace();
    }
    m_IndexBuffer.value().BufferData(Type, count);
}

void MeshObject::SetIndexSubBuffer(const void* data, size_t offset, size_t size)
{
    AssertOrErrorCall(m_EditMode, return;, "Could not perform SetIndexBuffer, Mesh Data object needs to be in edit mode to perform this operation")
    AssertOrErrorCall(m_IndexBuffer.has_value(), return;, "Could not perform SetIndexSubBuffer, index buffer is invalid")
    
    m_IndexBuffer.value().BufferSubData(data, offset, size);
}

void MeshObject::UnsetIndexBuffer()
{
    AssertOrErrorCall(m_EditMode, return;, "Could not perform SetIndexBuffer, Mesh Data object needs to be in edit mode to perform this operation")
    
    if (m_IndexBuffer.has_value())
    {
        m_IndexBuffer.reset();
    }
}

size_t MeshObject::AddVertexGroup(const Mesh::VertexGroup& VertexGroup)
{
    AssertOrErrorCall(m_EditMode, return std::numeric_limits<size_t>::max();, "Could not perform AddVertexGroup, Mesh Data object needs to be in edit mode to perform this operation")
    
    size_t index = m_Groups.size();
    m_Groups.emplace_back(VertexGroup);
    
    return index;
}

void MeshObject::SetVertexGroup(size_t index, const Mesh::VertexGroup& VertexGroup)
{
    AssertOrErrorCall(m_EditMode, return;, "Could not perform SetVertexGroup, Mesh Data object needs to be in edit mode to perform this operation")
    AssertOrErrorCall(index < m_Groups.size(), return;, "Index out of bounds")
    
    m_Groups[index] = VertexGroup;
}

void MeshObject::EndMesh()
{
    AssertOrErrorCall(m_EditMode, return;, "Could not perform EndMesh, Mesh Data object needs to be in edit mode to perform this operation")
    
    // Update VAO
    m_Vao.BufferData(m_VertexBuffers, m_Layouts);
}

GLenum ToGLGeometryType(Mesh::VertexType type)
{
    switch (type)
    {
    case Mesh::POINTS:                      return GL_POINTS;
    case Mesh::LINE_STRIP:                  return GL_LINE_STRIP;
    case Mesh::LINE_LOOP:                   return GL_LINE_LOOP;
    case Mesh::LINES:                       return GL_LINES;
    case Mesh::LINE_STRIP_ADJACENCY:        return GL_LINE_STRIP_ADJACENCY;
    case Mesh::LINES_ADJACENCY:             return GL_LINES_ADJACENCY;
    case Mesh::PATCHES:                     return GL_PATCHES;
    case Mesh::TRIANGLE_STRIP_ADJACENCY:    return GL_TRIANGLE_STRIP_ADJACENCY;
    case Mesh::TRIANGLES_ADJACENCY:         return GL_TRIANGLES_ADJACENCY;
    case Mesh::TRIANGLE_STRIP:              return GL_TRIANGLE_STRIP;
    case Mesh::TRIANGLE_FAN:                return GL_TRIANGLE_FAN;
    case Mesh::QUAD_STRIP:                  return GL_QUAD_STRIP;
    case Mesh::TRIANGLES:                   return GL_TRIANGLES;
    case Mesh::QUADS:                       return GL_QUADS;

    case Mesh::_Count:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported geometry type")
    }
}
