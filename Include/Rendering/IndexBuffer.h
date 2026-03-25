#pragma once

#include "Shared/Annotations.h"
#include "GLHelper.h"

class IndexBuffer
{
public:
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

    void BufferData(const void* data, unsigned int count);
    
private:
    GLuint m_IndexBuffer;
};

void Bind(const IndexBuffer& VertexBuffer);
void UnBind(const IndexBuffer& VertexBuffer);