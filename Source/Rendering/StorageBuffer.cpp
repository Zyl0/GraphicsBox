#include "StorageBuffer.h"

StorageBuffer::StorageBuffer()
{
    GLCall(glGenBuffers(1, &m_SSBO))
    m_Size = 0;
}

StorageBuffer::StorageBuffer(uint32_t Size, const void* data):
    StorageBuffer()
{
}

StorageBuffer::~StorageBuffer()
{
    GLCall(glDeleteBuffers(1, &m_SSBO))
}

void StorageBuffer::Data(const void* data, uint32_t size)
{
    m_Size = size;
    
    Bind(*this);
    glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW);
    UnBind(*this);
}

void StorageBuffer::SubData(const void* data, uint32_t size, uint32_t offset)
{
    Bind(*this);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    UnBind(*this);
}

void StorageBuffer::ExportData(uint32_t size, void* data) const
{
    Bind(*this);
    GLCall(glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, data))
    UnBind(*this);
}

void StorageBuffer::ExportSubData(uint32_t size, uint32_t offset, void* data) const
{
    Bind(*this);
    GLCall(glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data))
    UnBind(*this);
}

void Bind(const StorageBuffer& UniformBuffer)
{
    GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, UniformBuffer.Handle()))
}

void UnBind(const StorageBuffer& UniformBuffer)
{
    GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0))
}

void Bind(const StorageBuffer& UniformBuffer, uint32_t BindingPoint)
{
    GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BindingPoint, UniformBuffer.Handle()))
}

void UnBind(const StorageBuffer& UniformBuffer, uint32_t BindingPoint)
{
    GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BindingPoint, 0))
}
