#include "MeshObject.h"

void MeshObject::Data(const Mesh& Mesh)
{
    AssertOrErrorCall(!Mesh.GetPositions().empty(), return;, "Attempted to generate gpu mesh buffers of an empty mesh.");

    m_VertexBuffers.clear();
    VertexArrayObject::Layout layout;
    
    m_VertexBuffers.emplace_back(); m_VertexBuffers.back().Data(&(Mesh.GetPositions().data()->x), Mesh.GetPositions().size() * sizeof(Math::Point3f));
    layout.Push<Math::Point3f>(1);

    if(Mesh.HasNormals())
    {
        m_VertexBuffers.emplace_back(); m_VertexBuffers.back().Data(&(Mesh.GetNormals().data()->x), Mesh.GetNormals().size() * sizeof(Math::Vector3f));
        layout.Push<Math::Vector3f>(1);
    }
    if(Mesh.HasTangents())
    {
        m_VertexBuffers.emplace_back(); m_VertexBuffers.back().Data(&(Mesh.GetTangents().data()->x), Mesh.GetTangents().size() * sizeof(Math::Vector3f));
        layout.Push<Math::Vector3f>(1);
    }
    if(Mesh.HasTextureCoordinates())
    {
        m_VertexBuffers.emplace_back(); m_VertexBuffers.back().Data(&(Mesh.GetTextureCoordinates().data()->x), Mesh.GetTextureCoordinates().size() * sizeof(Math::Vector2f));
        layout.Push<Math::Vector2f>(1);
    }

    m_Vao.BufferData(m_VertexBuffers, layout);

    if(Mesh.IsIndexedMesh())
    {
        m_IndexBuffer = IndexBuffer();
        m_IndexBuffer.value().BufferData(Mesh.GetIndices().data(), Mesh.GetIndices().size());
    }
}
