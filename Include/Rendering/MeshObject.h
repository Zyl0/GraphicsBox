#pragma once
#include <vector>
#include <optional>

#include "IndexBuffer.h"
#include "VertexArrayObject.h"
#include "Modeling/Mesh.h"

struct MeshObject
{
    MeshObject() = default;
    MeshObject(const Mesh& Mesh) {Data(Mesh);}
    ~MeshObject() = default;

    void Data(const Mesh& Mesh);
    
    void BeginMesh(Mesh::VertexType meshType);
    size_t AddVertexBuffer(const VertexArrayObject::Layout& layout, const void* data, size_t size);
    void SetVertexBuffer(size_t index, const void* data, size_t size);
    void SetVertexBuffer(size_t index, const VertexArrayObject::Layout& layout, const void* data, size_t size);
    void PopVertexBuffer();
    void ClearVertexBuffers();
    void SetIndexBuffer(IndexBuffer::IndexType Type, const void* data, unsigned int count);
    void UnsetIndexBuffer();
    size_t AddVertexGroup(const Mesh::VertexGroup& VertexGroup);
    void SetVertexGroup(size_t index, const Mesh::VertexGroup& VertexGroup);
    void EndMesh();

    INLINE const VertexArrayObject& GetVAO() const {return m_Vao;}

    INLINE std::span<const Mesh::VertexGroup> GetGroups() const {return m_Groups;}

    INLINE size_t GetVertexBufferCount() const {return m_VertexBuffers.size();}
    INLINE const VertexBuffer& GetVertexBuffer(size_t Index) const {return m_VertexBuffers[Index];}
    INLINE const std::optional<IndexBuffer>& GetIndexBuffer() const  {return m_IndexBuffer;}
    
    INLINE Mesh::VertexType GetVertexType() const {return m_VertexType;}

    MeshObject(const MeshObject& Other) = delete;

    MeshObject(MeshObject&& Other) noexcept
        : m_Layouts(std::move(Other.m_Layouts)),
          m_Vao(std::move(Other.m_Vao)),
          m_IndexBuffer(std::move(Other.m_IndexBuffer)),
          m_VertexBuffers(std::move(Other.m_VertexBuffers)),
          m_Groups(std::move(Other.m_Groups)),
          m_VertexType(Other.m_VertexType),
          m_EditMode(Other.m_EditMode)
    {
    }

    MeshObject& operator=(const MeshObject& Other) = delete;

    MeshObject& operator=(MeshObject&& Other) noexcept
    {
        if (this == &Other)
            return *this;
        m_Layouts = std::move(Other.m_Layouts);
        m_Vao = std::move(Other.m_Vao);
        m_IndexBuffer = std::move(Other.m_IndexBuffer);
        m_VertexBuffers = std::move(Other.m_VertexBuffers);
        m_Groups = std::move(Other.m_Groups);
        m_VertexType = Other.m_VertexType;
        m_EditMode = Other.m_EditMode;
        return *this;
    }

private:
    std::vector<VertexArrayObject::Layout> m_Layouts;
    VertexArrayObject m_Vao;
    std::optional<IndexBuffer> m_IndexBuffer;
    std::vector<VertexBuffer> m_VertexBuffers;
    std::vector<Mesh::VertexGroup> m_Groups;
    Mesh::VertexType m_VertexType;
    bool m_EditMode = false;
};

GLenum ToGLGeometryType(Mesh::VertexType type);