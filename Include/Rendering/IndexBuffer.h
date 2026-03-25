#pragma once

#include <cstdint>

#include "Shared/Annotations.h"
#include "GLHelper.h"

class IndexBuffer
{
public:
    enum IndexType : uint8_t
    {
        Byte = 0,
        UnsignedByte,
        Short,
        UnsignedShort,
        Int,
        UnsignedInt,
        _Count
    };
    
    IndexBuffer();
    ~IndexBuffer();

    INLINE IndexBuffer(const IndexBuffer& Other) = delete;

    INLINE IndexBuffer(IndexBuffer&& Other) noexcept: m_IndexBuffer(Other.m_IndexBuffer) {Other.m_IndexBuffer = 0;}

    INLINE IndexBuffer& operator=(const IndexBuffer& Other) = delete;

    INLINE IndexBuffer& operator=(IndexBuffer&& Other) noexcept
    {
        if (this == &Other)
            return *this;
        m_IndexBuffer = Other.m_IndexBuffer;
        Other.m_IndexBuffer = 0;
        return *this;
    }

    INLINE GLuint Handle() const
    {
        return m_IndexBuffer;
    }

    void BufferData(IndexType type, const void* data, unsigned int count);
    
    INLINE IndexType GetIndexType() const {return m_IndexType;}
    
private:
    GLuint m_IndexBuffer;
    IndexType m_IndexType;
};

GLenum ToGLIndexType(IndexBuffer::IndexType Type);
GLsizei ToGLIndexSize(IndexBuffer::IndexType Type);

void Bind(const IndexBuffer& VertexBuffer);
void UnBind(const IndexBuffer& VertexBuffer);