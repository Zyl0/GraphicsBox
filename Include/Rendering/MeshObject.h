#pragma once
#include <vector>
#include <optional>

#include "IndexBuffer.h"
#include "VertexArrayObject.h"
#include "Modeling/Mesh.h"

struct MeshObject
{
    MeshObject(const Mesh& Mesh) {Data(Mesh);}
    ~MeshObject() = delete;

    void Data(const Mesh& Mesh);

    INLINE const VertexArrayObject& GetVAO() const {return m_Vao;}

    INLINE std::span<const Mesh::VertexGroup> GetGroups() const {return m_Groups;}

    INLINE size_t GetVertexBufferCount() const {return m_VertexBuffers.size();}
    INLINE const VertexBuffer& GetVertexBuffer(size_t Index) const {return m_VertexBuffers[Index];}
    INLINE const std::optional<IndexBuffer>& GetIndexBuffer() const  {return m_IndexBuffer;}
    
private:
    VertexArrayObject m_Vao;
    std::optional<IndexBuffer> m_IndexBuffer;
    std::vector<VertexBuffer> m_VertexBuffers;
    std::vector<Mesh::VertexGroup> m_Groups;
};
