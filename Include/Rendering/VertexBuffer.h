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

    void Data(const void* data, size_t size);

    void Data(const void** buffers, const size_t* bufferSizes, size_t numBuffers);

private:
    GLuint m_VertexBuffer;
};

void Bind(const VertexBuffer& VertexBuffer);
void UnBind(const VertexBuffer& VertexBuffer);