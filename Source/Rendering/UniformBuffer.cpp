#include "UniformBuffer.h"

UniformBuffer::UniformBuffer() : m_Size(0)
{
    glGenBuffers(1, &m_UBO);
}

UniformBuffer::UniformBuffer(uint32_t Size, const void* data):
    m_Size(Size)
{
    glGenBuffers(1, &m_UBO);

    Bind(*this);
    glBufferData(GL_UNIFORM_BUFFER, Size, data, GL_STATIC_DRAW);
    UnBind(*this);
}

UniformBuffer::~UniformBuffer()
{
    if (m_UBO == 0) return;
    
    glDeleteBuffers(1, &m_UBO);
}

void UniformBuffer::Data(const void* data, uint32_t size)
{
    m_Size = size;
    
    Bind(*this);
    glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW);
    UnBind(*this);
}

void UniformBuffer::SubData(const void* data, uint32_t size, uint32_t offset)
{
    Bind(*this);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    UnBind(*this);
}

void Bind(const UniformBuffer& UniformBuffer)
{
    glBindBuffer(GL_UNIFORM_BUFFER, UniformBuffer.Handle());
}

void UnBind(const UniformBuffer& UniformBuffer)
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


