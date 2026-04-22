#pragma once

#include <cstdint>

#include "Shared/Annotations.h"
#include "GLHelper.h"

class StorageBuffer
{
public:
    StorageBuffer();
    StorageBuffer(uint32_t Size, const void* data = nullptr);
    ~StorageBuffer();

    void Data(const void* data, uint32_t size);
    void SubData(const  void* data, uint32_t size, uint32_t offset);
    void ExportData(uint32_t size, void* data) const;
    void ExportSubData(uint32_t size, uint32_t offset, void* data) const;
    
    INLINE GLuint Handle() const    { return m_SSBO; }
    INLINE uint32_t Size() const    { return m_Size; }
    
private:
    GLuint m_SSBO;
    uint32_t m_Size;
};

void Bind(const StorageBuffer& UniformBuffer);
void UnBind(const StorageBuffer& UniformBuffer);

void Bind(const StorageBuffer& UniformBuffer, uint32_t BindingPoint);
void UnBind(const StorageBuffer& UniformBuffer, uint32_t BindingPoint);