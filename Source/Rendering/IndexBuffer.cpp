#include "IndexBuffer.h"

IndexBuffer::IndexBuffer()
{
    GLCall(glGenBuffers(1, &m_IndexBuffer))
}

IndexBuffer::~IndexBuffer()
{
    GLCall(glDeleteBuffers(1, &m_IndexBuffer))
}

void IndexBuffer::BufferData(const void* data, unsigned int count)
{
    Bind(*this);
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER,  count * sizeof(unsigned int), data, GL_STATIC_DRAW))
    UnBind(*this);
}

void Bind(const IndexBuffer& VertexBuffer)
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VertexBuffer.Handle()))
}

void UnBind(const IndexBuffer& VertexBuffer)
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0))
}