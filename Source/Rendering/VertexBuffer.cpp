#include "VertexBuffer.h"

VertexBuffer::VertexBuffer()
{
    GLCall(glGenBuffers(1, &m_VertexBuffer))
}

VertexBuffer::~VertexBuffer()
{
    if (m_VertexBuffer != 0)
    {
        GLCall(glDeleteBuffers(1, &m_VertexBuffer))
    }
}

void VertexBuffer::Data(const void* data, size_t size)
{
    Bind(*this);
    GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW))
    UnBind(*this);
}

void VertexBuffer::Data(const void** buffers, const size_t* bufferSizes, size_t numBuffers)
{
    Bind(*this);
    unsigned totalSize = 0;
    for(size_t i = 0; i < numBuffers;i++)
        totalSize += bufferSizes[i];
    GLCall(glBufferData(GL_ARRAY_BUFFER, totalSize, 0, GL_STATIC_DRAW))

    unsigned offset = 0;
    for(size_t i = 0; i < numBuffers;i++)
    {
        GLCall(glBufferSubData(GL_ARRAY_BUFFER, offset, bufferSizes[i], buffers[i]))
        offset += bufferSizes[i];
    }
    UnBind(*this);
}

void Bind(const VertexBuffer& VertexBuffer)
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer.Handle()))
}

void UnBind(const VertexBuffer& VertexBuffer)
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0))
}
