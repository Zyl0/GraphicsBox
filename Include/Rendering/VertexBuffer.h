#pragma once

#include <cstdint>

#include "Shared/Annotations.h"
#include "GLHelper.h"

class VertexBuffer
{
public:
    VertexBuffer();
    ~VertexBuffer();

    INLINE GLuint Handle() const
    {
        return m_VertexBuffer;
    }

    INLINE VertexBuffer(const VertexBuffer& Other) = delete;

    INLINE VertexBuffer(VertexBuffer&& Other) noexcept: m_VertexBuffer(Other.m_VertexBuffer) { Other.m_VertexBuffer = 0; }

    INLINE VertexBuffer& operator=(const VertexBuffer& Other) = delete;

    INLINE VertexBuffer& operator=(VertexBuffer&& Other) noexcept
    {
        if (this == &Other)
            return *this;
        m_VertexBuffer = Other.m_VertexBuffer;
        Other.m_VertexBuffer = 0;
        return *this;
    }

    void Data(const void* data, size_t size);

    void Data(const void** buffers, const size_t* bufferSizes, size_t numBuffers);
    
    void SubData(const void* data, size_t offset, size_t size);

private:
    GLuint m_VertexBuffer;
};

void Bind(const VertexBuffer& VertexBuffer);
void UnBind(const VertexBuffer& VertexBuffer);