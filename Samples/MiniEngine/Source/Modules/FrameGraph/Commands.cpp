#include "Modules/FrameGraph/Commands.h"

#include <cstring>
#include "Rendering/Debug.h"

#include "Core/Module.h"

#define ALWAYS_REFLECT_RENDERING_OBJECTS
#define ALWAYS_REFLECT_VARIABLES

namespace FrameGraph
{
    FrameGraph::Location CameraArray::AddCamera()
    {
        FrameGraph::Location Index;
        
        if (m_FreeCount > 0)
        {
            Index = std::find(m_Validity.begin(), m_Validity.end(), false) - m_Validity.begin();
            m_Validity[Index] = true;
            m_FreeCount--;
        }
        else
        {
            Index = m_Validity.size();
            
            m_Validity.push_back(true);
            m_CPUArray.emplace_back();
            
            if (Index == m_GPUArray.Size())
            {
                m_GPUArray.Resize(m_CPUArray.capacity());
            }
        }
        
        m_HasChanged = true;
        
        return Index;
    }

    FrameGraph::Location CameraArray::AddCamera(const Camera& camera)
    {
        Location Index = AddCamera();
        UpdateCameraData(m_CPUArray[Index], camera);
        
        return Index;
    }

    FrameGraph::Location CameraArray::AddCamera(const Rendering::CameraData& camera)
    {
        Location Index = AddCamera();
        std::memcpy(&(m_CPUArray[Index]), &camera, sizeof(Rendering::CameraData));
        
        return Index;
    }

    void CameraArray::UpdateCamera(FrameGraph::Location Index, const Camera& camera)
    {
        UpdateCameraData(m_CPUArray[Index], camera);
        m_HasChanged = true;
    }

    void CameraArray::UpdateCamera(FrameGraph::Location Index, const Rendering::CameraData& camera)
    {
        std::memcpy(&(m_CPUArray[Index]), &camera, sizeof(Rendering::CameraData));
        m_HasChanged = true;
    }

    CommandContext::CommandContext(bool RequireReflectionData )
    {
        if (RequireReflectionData)
        {
            m_ReflectionData.emplace();
        }
    }

    void CommandContext::UpdateCameras()
    {
        m_CameraArray.Update();
    }

    void CommandContext::ConsumeVariableChanges()
    {
        VariableList<FrameGraph::Bool>().Reset();
        VariableList<FrameGraph::UInt>().Reset();
        VariableList<FrameGraph::Int>().Reset();
        VariableList<FrameGraph::Float>().Reset();
        VariableList<FrameGraph::Size2D>().Reset();
        VariableList<FrameGraph::Rect>().Reset();
        VariableList<Math::Vector3f>().Reset();
    }

    template <>
    FrameGraph::Location CommandContext::Add<VertexBuffer>(std::string_view Name)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<VertexBuffer>().Has(Name))
            {
                return GetLocation<VertexBuffer>(Name);
            }
            
            Location At = ObjectList<VertexBuffer>().Add(Name);

            _Graph::ReflectedObject<VertexBuffer> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_VertexBuffers.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<VertexBuffer>().Add(Name);
    }

    template<>
    FrameGraph::Location CommandContext::Add<VertexBuffer>(std::string_view Name, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<VertexBuffer>().Has(Name))
            {
                return GetLocation<VertexBuffer>(Name);
            }
            
            Location At = ObjectList<VertexBuffer>().Add(Name);

            _Graph::ReflectedObject<VertexBuffer> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_VertexBuffers.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<VertexBuffer>().Add(Name);
    }

    template <>
    FrameGraph::Location CommandContext::Add<IndexBuffer>(std::string_view Name)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<IndexBuffer>().Has(Name))
            {
                return GetLocation<IndexBuffer>(Name);
            }
            
            Location At = ObjectList<IndexBuffer>().Add(Name);

            _Graph::ReflectedObject<IndexBuffer> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_IndexBuffers.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<IndexBuffer>().Add(Name);
    }

    template<>
    FrameGraph::Location CommandContext::Add<IndexBuffer>(std::string_view Name, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<IndexBuffer>().Has(Name))
            {
                return GetLocation<IndexBuffer>(Name);
            }
            
            Location At = ObjectList<IndexBuffer>().Add(Name);

            _Graph::ReflectedObject<IndexBuffer> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_IndexBuffers.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<IndexBuffer>().Add(Name);
    }

    template <>
    FrameGraph::Location CommandContext::Add<VertexArrayObject>(std::string_view Name)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<VertexArrayObject>().Has(Name))
            {
                return GetLocation<VertexArrayObject>(Name);
            }
            
            Location At = ObjectList<VertexArrayObject>().Add(Name);

            _Graph::ReflectedObject<VertexArrayObject> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_VAOs.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<VertexArrayObject>().Add(Name);
    }

    template<>
    FrameGraph::Location CommandContext::Add<VertexArrayObject>(std::string_view Name, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<VertexArrayObject>().Has(Name))
            {
                return GetLocation<VertexArrayObject>(Name);
            }
            
            Location At = ObjectList<VertexArrayObject>().Add(Name);

            _Graph::ReflectedObject<VertexArrayObject> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_VAOs.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<VertexArrayObject>().Add(Name);
    }

    template <>
    FrameGraph::Location CommandContext::Add<UniformBuffer>(std::string_view Name)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<UniformBuffer>().Has(Name))
            {
                return GetLocation<UniformBuffer>(Name);
            }
            
            Location At = ObjectList<UniformBuffer>().Add(Name);

            _Graph::ReflectedObject<UniformBuffer> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_UniformBuffers.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<UniformBuffer>().Add(Name);
    }

    template <>
    FrameGraph::Location CommandContext::Add<UniformBuffer>(std::string_view Name, uint32_t Size)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<UniformBuffer>().Has(Name))
            {
                return GetLocation<UniformBuffer>(Name);
            }
            
            Location At = ObjectList<UniformBuffer>().Add(Name, Size, (const void*)nullptr);

            _Graph::ReflectedObject<UniformBuffer> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_UniformBuffers.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return Add<UniformBuffer>(Name, Size, (const void*)nullptr);
    }

    template <>
    FrameGraph::Location CommandContext::Add<UniformBuffer>(std::string_view Name, uint32_t Size, const void* data)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<UniformBuffer>().Has(Name))
            {
                return GetLocation<UniformBuffer>(Name);
            }
            
            Location At = ObjectList<UniformBuffer>().Add(Name, Size, data);

            _Graph::ReflectedObject<UniformBuffer> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_UniformBuffers.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<UniformBuffer>().Add(Name, Size, data);
    }

    template<> 
    FrameGraph::Location CommandContext::Add<UniformBuffer>(std::string_view Name, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<UniformBuffer>().Has(Name))
            {
                return GetLocation<UniformBuffer>(Name);
            }
            
            Location At = ObjectList<UniformBuffer>().Add(Name);

            _Graph::ReflectedObject<UniformBuffer> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_UniformBuffers.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<UniformBuffer>().Add(Name);
    }
    
    template<> 
    FrameGraph::Location CommandContext::Add<UniformBuffer>(std::string_view Name, uint32_t Size, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<UniformBuffer>().Has(Name))
            {
                return GetLocation<UniformBuffer>(Name);
            }
            
            Location At = ObjectList<UniformBuffer>().Add(Name, Size, (const void*)nullptr);

            _Graph::ReflectedObject<UniformBuffer> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_UniformBuffers.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<UniformBuffer>().Add(Name, Size, (const void*)nullptr);
    }
    
    template<> 
    FrameGraph::Location CommandContext::Add<UniformBuffer>(std::string_view Name, uint32_t Size, const void* data, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<UniformBuffer>().Has(Name))
            {
                return GetLocation<UniformBuffer>(Name);
            }
            
            Location At = ObjectList<UniformBuffer>().Add(Name, Size, data);

            _Graph::ReflectedObject<UniformBuffer> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_UniformBuffers.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<UniformBuffer>().Add(Name, Size, data);
    }
    
    template <>
    FrameGraph::Location CommandContext::Add<StorageBuffer>(std::string_view Name)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<StorageBuffer>().Has(Name))
            {
                return GetLocation<StorageBuffer>(Name);
            }
            
            Location At = ObjectList<StorageBuffer>().Add(Name);

            _Graph::ReflectedObject<StorageBuffer> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_StorageBuffers.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<StorageBuffer>().Add(Name);
    }

    template <>
    FrameGraph::Location CommandContext::Add<StorageBuffer>(std::string_view Name, uint32_t Size)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<StorageBuffer>().Has(Name))
            {
                return GetLocation<StorageBuffer>(Name);
            }
            
            Location At = ObjectList<StorageBuffer>().Add(Name, Size);

            _Graph::ReflectedObject<StorageBuffer> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_StorageBuffers.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<StorageBuffer>().Add(Name, Size);
    }

    template <>
    FrameGraph::Location CommandContext::Add<StorageBuffer>(std::string_view Name, uint32_t Size, const void* data)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<StorageBuffer>().Has(Name))
            {
                return GetLocation<StorageBuffer>(Name);
            }
            
            Location At = ObjectList<StorageBuffer>().Add(Name, Size, data);

            _Graph::ReflectedObject<StorageBuffer> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_StorageBuffers.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<StorageBuffer>().Add(Name, Size, data);
    }

    template<> 
    FrameGraph::Location CommandContext::Add<StorageBuffer>(std::string_view Name, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<StorageBuffer>().Has(Name))
            {
                return GetLocation<StorageBuffer>(Name);
            }
            
            Location At = ObjectList<StorageBuffer>().Add(Name);

            _Graph::ReflectedObject<StorageBuffer> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_StorageBuffers.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<StorageBuffer>().Add(Name);
    }
    
    template<> 
    FrameGraph::Location CommandContext::Add<StorageBuffer>(std::string_view Name, uint32_t Size, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<StorageBuffer>().Has(Name))
            {
                return GetLocation<StorageBuffer>(Name);
            }
            
            Location At = ObjectList<StorageBuffer>().Add(Name, Size);

            _Graph::ReflectedObject<StorageBuffer> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_StorageBuffers.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<StorageBuffer>().Add(Name, Size);
    }
    
    template<> 
    FrameGraph::Location CommandContext::Add<StorageBuffer>(std::string_view Name, uint32_t Size, const void* data, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<StorageBuffer>().Has(Name))
            {
                return GetLocation<StorageBuffer>(Name);
            }
            
            Location At = ObjectList<StorageBuffer>().Add(Name, Size, data);

            _Graph::ReflectedObject<StorageBuffer> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_StorageBuffers.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<StorageBuffer>().Add(Name);
    }

    // template <>
    // FrameGraph::Location CommandContext::Add<MeshObject>(std::string_view Name)
    // {
    //     return ObjectList<MeshObject>().Add(Name);
    // }

    // template <>
    // FrameGraph::Location CommandContext::Add<MeshObject>(std::string_view Name, const Mesh& Mesh)
    // {
    //     return ObjectList<MeshObject>().Add(Name, Mesh);
    // }

    template <>
    FrameGraph::Location CommandContext::Add<Texture2D>(std::string_view Name, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture2D>().Has(Name))
            {
                return GetLocation<Texture2D>(Name);
            }
            
            Location At = ObjectList<Texture2D>().Add(Name, width, height, type, layout, (uint8_t)0);

            _Graph::ReflectedObject<Texture2D> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_Textures2D.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<Texture2D>().Add(Name, width, height, type, layout, (uint8_t)0);
    }

    template <>
    FrameGraph::Location CommandContext::Add<Texture2D>(std::string_view Name, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout, uint8_t SampleCount)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture2D>().Has(Name))
            {
                return GetLocation<Texture2D>(Name);
            }
            
            Location At = ObjectList<Texture2D>().Add(Name, width, height, type, layout, SampleCount);

            _Graph::ReflectedObject<Texture2D> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_Textures2D.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<Texture2D>().Add(Name, width, height, type, layout, SampleCount);
    }

    template <>
    FrameGraph::Location CommandContext::Add<Texture2D>(std::string_view Name, const Image& Image)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture2D>().Has(Name))
            {
                return GetLocation<Texture2D>(Name);
            }
            
            Location At = ObjectList<Texture2D>().Add(Name, Image, true);

            _Graph::ReflectedObject<Texture2D> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_Textures2D.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<Texture2D>().Add(Name, Image, true);
    }

    template <>
    FrameGraph::Location CommandContext::Add<Texture2D>(std::string_view Name, const Image& Image, bool UseMips)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture2D>().Has(Name))
            {
                return GetLocation<Texture2D>(Name);
            }
            
            Location At = ObjectList<Texture2D>().Add(Name, Image, UseMips);

            _Graph::ReflectedObject<Texture2D> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_Textures2D.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<Texture2D>().Add(Name, Image, UseMips);
    }

    template <>
    FrameGraph::Location CommandContext::Add<Texture2D>(std::string_view Name, uint32_t width, uint32_t height, Image::Type type, Image::Layout layout, const void* ImageData, size_t ImageSize)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture2D>().Has(Name))
            {
                return GetLocation<Texture2D>(Name);
            }
            
            Location At = ObjectList<Texture2D>().Add(Name, width, height, type, layout, ImageData, ImageSize, true);

            _Graph::ReflectedObject<Texture2D> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_Textures2D.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<Texture2D>().Add(Name, width, height, type, layout, ImageData, ImageSize, true);
    }

    template <>
    FrameGraph::Location CommandContext::Add<Texture2D>(std::string_view Name, uint32_t width, uint32_t height, Image::Type type, Image::Layout layout, const void* ImageData, size_t ImageSize, bool UseMips)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture2D>().Has(Name))
            {
                return GetLocation<Texture2D>(Name);
            }
            
            Location At = ObjectList<Texture2D>().Add(Name, width, height, type, layout, ImageData, ImageSize, UseMips);

            _Graph::ReflectedObject<Texture2D> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_Textures2D.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<Texture2D>().Add(Name, width, height, type, layout, ImageData, ImageSize, UseMips);
    }

    template<> 
    FrameGraph::Location CommandContext::Add<Texture2D>(std::string_view Name, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture2D>().Has(Name))
            {
                return GetLocation<Texture2D>(Name);
            }
            
            Location At = ObjectList<Texture2D>().Add(Name, width, height, type, layout, (uint8_t)0);

            _Graph::ReflectedObject<Texture2D> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_Textures2D.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<Texture2D>().Add(Name, width, height, type, layout, (uint8_t)0);
    }
    
    template<> 
    FrameGraph::Location CommandContext::Add<Texture2D>(std::string_view Name, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout, uint8_t SampleCount, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture2D>().Has(Name))
            {
                return GetLocation<Texture2D>(Name);
            }
            
            Location At = ObjectList<Texture2D>().Add(Name, width, height, type, layout, SampleCount);

            _Graph::ReflectedObject<Texture2D> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_Textures2D.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<Texture2D>().Add(Name, width, height, type, layout, SampleCount);
    }
    
    template<> 
    FrameGraph::Location CommandContext::Add<Texture2D>(std::string_view Name, const Image& Image, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture2D>().Has(Name))
            {
                return GetLocation<Texture2D>(Name);
            }
            
            Location At = ObjectList<Texture2D>().Add(Name, Image, true);

            _Graph::ReflectedObject<Texture2D> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_Textures2D.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<Texture2D>().Add(Name, Image, true);
    }
    
    template<> 
    FrameGraph::Location CommandContext::Add<Texture2D>(std::string_view Name, const Image& Image, bool UseMips, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture2D>().Has(Name))
            {
                return GetLocation<Texture2D>(Name);
            }
            
            Location At = ObjectList<Texture2D>().Add(Name, Image, UseMips);

            _Graph::ReflectedObject<Texture2D> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_Textures2D.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<Texture2D>().Add(Name, Image, UseMips);
    }
    
    template<> 
    FrameGraph::Location CommandContext::Add<Texture2D>(std::string_view Name, uint32_t width, uint32_t height, Image::Type type, Image::Layout layout, const void* ImageData, size_t ImageSize, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture2D>().Has(Name))
            {
                return GetLocation<Texture2D>(Name);
            }
            
            Location At = ObjectList<Texture2D>().Add(Name, width, height, type, layout, ImageData, ImageSize, true);

            _Graph::ReflectedObject<Texture2D> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_Textures2D.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<Texture2D>().Add(Name, width, height, type, layout, ImageData, ImageSize, true);
    }
    
    template<> 
    FrameGraph::Location CommandContext::Add<Texture2D>(std::string_view Name, uint32_t width, uint32_t height, Image::Type type, Image::Layout layout, const void* ImageData, size_t ImageSize, bool UseMips, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture2D>().Has(Name))
            {
                return GetLocation<Texture2D>(Name);
            }
            
            Location At = ObjectList<Texture2D>().Add(Name, width, height, type, layout, ImageData, ImageSize, UseMips);

            _Graph::ReflectedObject<Texture2D> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_Textures2D.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<Texture2D>().Add(Name, width, height, type, layout, ImageData, ImageSize, UseMips);
    }
    
    template <>
    FrameGraph::Location CommandContext::Add<Texture2DArray>(std::string_view Name, uint32_t width, uint32_t height, uint32_t count, Texture::Type type, Texture::Layout layout)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture2DArray>().Has(Name))
            {
                return GetLocation<Texture2DArray>(Name);
            }
            
            Location At = ObjectList<Texture2DArray>().Add(Name, width, height, count, type, layout, false);

            _Graph::ReflectedObject<Texture2DArray> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_Textures2DArray.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<Texture2DArray>().Add(Name, width, height, count, type, layout, false);
    }

    template <>
    FrameGraph::Location CommandContext::Add<Texture2DArray>(std::string_view Name, uint32_t width, uint32_t height, uint32_t count, Texture::Type type, Texture::Layout layout, bool UseMips)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture2DArray>().Has(Name))
            {
                return GetLocation<Texture2DArray>(Name);
            }
            
            Location At = ObjectList<Texture2DArray>().Add(Name, width, height, count, type, layout, UseMips);

            _Graph::ReflectedObject<Texture2DArray> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_Textures2DArray.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<Texture2DArray>().Add(Name, width, height, count, type, layout, UseMips);
    }
    
    template<> 
    FrameGraph::Location CommandContext::Add<Texture2DArray>(std::string_view Name, uint32_t width, uint32_t height, uint32_t count, Texture::Type type, Texture::Layout layout, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture2DArray>().Has(Name))
            {
                return GetLocation<Texture2DArray>(Name);
            }
            
            Location At = ObjectList<Texture2DArray>().Add(Name, width, height, count, type, layout, false);

            _Graph::ReflectedObject<Texture2DArray> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_Textures2DArray.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<Texture2DArray>().Add(Name, width, height, count, type, layout, false);
    }
    
    template<> 
    FrameGraph::Location CommandContext::Add<Texture2DArray>(std::string_view Name, uint32_t width, uint32_t height, uint32_t count, Texture::Type type, Texture::Layout layout, bool UseMips, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture2DArray>().Has(Name))
            {
                return GetLocation<Texture2DArray>(Name);
            }
            
            Location At = ObjectList<Texture2DArray>().Add(Name, width, height, count, type, layout, UseMips);

            _Graph::ReflectedObject<Texture2DArray> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_Textures2DArray.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<Texture2DArray>().Add(Name, width, height, count, type, layout, UseMips);
    }
    
    template <>
    FrameGraph::Location CommandContext::Add<Texture3D>(std::string_view Name, uint32_t width, uint32_t height, uint32_t depth, Texture::Type type, Texture::Layout layout)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture3D>().Has(Name))
            {
                return GetLocation<Texture3D>(Name);
            }
            
            Location At = ObjectList<Texture3D>().Add(Name, width, height, depth, type, layout);

            _Graph::ReflectedObject<Texture3D> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_Textures3D.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<Texture3D>().Add(Name, width, height, depth, type, layout);
    }

    template <>
    FrameGraph::Location CommandContext::Add<Texture3D, const ImageCube&>(std::string_view Name, const ImageCube& Image)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture3D>().Has(Name))
            {
                return GetLocation<Texture3D>(Name);
            }
            
            Location At = ObjectList<Texture3D>().Add(Name, Image, true);

            _Graph::ReflectedObject<Texture3D> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_Textures3D.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<Texture3D>().Add(Name, Image, true);
    }

    template <>
    FrameGraph::Location CommandContext::Add<Texture3D, const ImageCube&, bool>(std::string_view Name, const ImageCube& Image, bool UseMips)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture3D>().Has(Name))
            {
                return GetLocation<Texture3D>(Name);
            }
            
            Location At = ObjectList<Texture3D>().Add(Name, Image, UseMips);

            _Graph::ReflectedObject<Texture3D> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_Textures3D.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<Texture3D>().Add(Name, Image, UseMips);
    }

    template<> 
    FrameGraph::Location CommandContext::Add<Texture3D>(std::string_view Name, uint32_t width, uint32_t height, uint32_t depth, Texture::Type type, Texture::Layout layout, std::string_view Description)
    {        
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture3D>().Has(Name))
            {
                return GetLocation<Texture3D>(Name);
            }
            
            Location At = ObjectList<Texture3D>().Add(Name, width, height, depth, type, layout);

            _Graph::ReflectedObject<Texture3D> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_Textures3D.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<Texture3D>().Add(Name, width, height, depth, type, layout);
    }
    
    template<> 
    FrameGraph::Location CommandContext::Add<Texture3D>(std::string_view Name, const ImageCube& Image, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture3D>().Has(Name))
            {
                return GetLocation<Texture3D>(Name);
            }
            
            Location At = ObjectList<Texture3D>().Add(Name, Image, true);

            _Graph::ReflectedObject<Texture3D> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_Textures3D.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<Texture3D>().Add(Name, Image, true);
    }
    
    template<> 
    FrameGraph::Location CommandContext::Add<Texture3D>(std::string_view Name, const ImageCube& Image, bool UseMips, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<Texture3D>().Has(Name))
            {
                return GetLocation<Texture3D>(Name);
            }
            
            Location At = ObjectList<Texture3D>().Add(Name, Image, UseMips);

            _Graph::ReflectedObject<Texture3D> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_Textures3D.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<Texture3D>().Add(Name, Image, UseMips);
    }
    
    template <>
    FrameGraph::Location CommandContext::Add<TextureCube>(std::string_view Name, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<TextureCube>().Has(Name))
            {
                return GetLocation<TextureCube>(Name);
            }
            
            Location At = ObjectList<TextureCube>().Add(Name, width, height, type, layout, false);

            _Graph::ReflectedObject<TextureCube> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_TexturesCube.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<TextureCube>().Add(Name, width, height, type, layout, false);
    }

    template <>
    FrameGraph::Location CommandContext::Add<TextureCube>(std::string_view Name, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout, bool UseMips)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<TextureCube>().Has(Name))
            {
                return GetLocation<TextureCube>(Name);
            }
            
            Location At = ObjectList<TextureCube>().Add(Name, width, height, type, layout, UseMips);

            _Graph::ReflectedObject<TextureCube> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_TexturesCube.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<TextureCube>().Add(Name, width, height, type, layout, UseMips);
    }

    template <>
    FrameGraph::Location CommandContext::Add<TextureCube>(std::string_view Name, std::span<const TextureCube::FacePair> Faces)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<TextureCube>().Has(Name))
            {
                return GetLocation<TextureCube>(Name);
            }
            
            Location At = ObjectList<TextureCube>().Add(Name, Faces, true);

            _Graph::ReflectedObject<TextureCube> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_TexturesCube.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<TextureCube>().Add(Name, Faces, true);
    }

    template <>
    FrameGraph::Location CommandContext::Add<TextureCube>(std::string_view Name, std::span<const TextureCube::FacePair> Faces, bool UseMips)
    {
#ifdef ALWAYS_REFLECT_RENDERING_OBJECTS
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<TextureCube>().Has(Name))
            {
                return GetLocation<TextureCube>(Name);
            }
            
            Location At = ObjectList<TextureCube>().Add(Name, Faces, UseMips);

            _Graph::ReflectedObject<TextureCube> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            
            m_ReflectionData->m_TexturesCube.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_RENDERING_OBJECTS
        
        return ObjectList<TextureCube>().Add(Name, Faces, UseMips);
    }

    template<> 
    FrameGraph::Location CommandContext::Add<TextureCube>(std::string_view Name, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<TextureCube>().Has(Name))
            {
                return GetLocation<TextureCube>(Name);
            }
            
            Location At = ObjectList<TextureCube>().Add(Name, width, height, type, layout, false);

            _Graph::ReflectedObject<TextureCube> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_TexturesCube.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<TextureCube>().Add(Name, width, height, type, layout, false);
    }
    
    template<> 
    FrameGraph::Location CommandContext::Add<TextureCube>(std::string_view Name, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout, bool UseMips, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<TextureCube>().Has(Name))
            {
                return GetLocation<TextureCube>(Name);
            }
            
            Location At = ObjectList<TextureCube>().Add(Name, width, height, type, layout, UseMips);

            _Graph::ReflectedObject<TextureCube> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_TexturesCube.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<TextureCube>().Add(Name, width, height, type, layout, UseMips);
    }
    
    template<> 
    FrameGraph::Location CommandContext::Add<TextureCube>(std::string_view Name, std::span<const TextureCube::FacePair> Faces, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<TextureCube>().Has(Name))
            {
                return GetLocation<TextureCube>(Name);
            }
            
            Location At = ObjectList<TextureCube>().Add(Name, Faces, true);

            _Graph::ReflectedObject<TextureCube> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_TexturesCube.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<TextureCube>().Add(Name, Faces, true);
    }
    
    template<> 
    FrameGraph::Location CommandContext::Add<TextureCube>(std::string_view Name, std::span<const TextureCube::FacePair> Faces, bool UseMips, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            if (ObjectList<TextureCube>().Has(Name))
            {
                return GetLocation<TextureCube>(Name);
            }
            
            Location At = ObjectList<TextureCube>().Add(Name, Faces, UseMips);

            _Graph::ReflectedObject<TextureCube> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            
            m_ReflectionData->m_TexturesCube.push_back(ReflectionData);

            return At;
        }
        
        return ObjectList<TextureCube>().Add(Name, Faces, UseMips);
    }
    
    // template <>
    // FrameGraph::Location CommandContext::Add<TextureCubeView>(std::string_view Name, const TextureCube& texture, uint32_t MipLevel)
    // {
    //     return Add<TextureCubeView, const TextureCube&>(Name, texture, MipLevel, 1u);
    // }
    // template <>
    // FrameGraph::Location CommandContext::Add<TextureCubeView>(std::string_view Name, const TextureCube& texture, uint32_t MipLevel, uint32_t MipCount)
    // {
    //     return ObjectList<TextureCubeView>().Add<const TextureCube&>(Name, texture, MipLevel, MipCount);
    // }

    // template <>
    // FrameGraph::Location CommandContext::Add<TextureCubeView>(std::string_view Name, std::string_view texture, uint32_t MipLevel)
    // {
    //     return Add<TextureCubeView>(Name, texture, MipLevel, 1u);
    // }

    // template <>
    // FrameGraph::Location CommandContext::Add<TextureCubeView>(std::string_view Name, std::string_view texture, uint32_t MipLevel, uint32_t MipCount)
    // {
    //     return ObjectList<TextureCubeView>().Add(Name, ObjectList<TextureCube>().Get(ToName(texture)), MipLevel, MipCount);
    // }

    // template <>
    // FrameGraph::Location CommandContext::Add<TextureCubeView>(std::string_view Name, FrameGraph::Location texture, uint32_t MipLevel)
    // {
    //     return Add<TextureCubeView>(Name, texture, MipLevel, 1u);
    // }

    // template <>
    // FrameGraph::Location CommandContext::Add<TextureCubeView>(std::string_view Name, FrameGraph::Location texture, uint32_t MipLevel, uint32_t MipCount)
    // {
    //     return ObjectList<TextureCubeView>().Add(Name, ObjectList<TextureCube>()[texture], MipLevel, MipCount);
    // }

    template<> 
    FrameGraph::Location CommandContext::AddVariable<FrameGraph::Bool>(std::string_view Name, const FrameGraph::Bool& BaseValue) 
    {
#ifdef ALWAYS_REFLECT_VARIABLES
        if (m_ReflectionData.has_value())
        {
            Location At = VariableList<FrameGraph::Bool>().Add(Name, BaseValue);

            _Graph::ReflectedVariable<FrameGraph::Bool> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            ReflectionData.BaseValue = BaseValue;
            
            m_ReflectionData->m_BoolVariables.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_VARIABLES
        
        return VariableList<FrameGraph::Bool>().Add(Name, BaseValue);
    }
    
    template<>
    FrameGraph::Location CommandContext::AddVariable<FrameGraph::Bool>(std::string_view Name, const FrameGraph::Bool& BaseValue, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            Location At = VariableList<FrameGraph::Bool>().Add(Name, BaseValue);

            _Graph::ReflectedVariable<FrameGraph::Bool> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            ReflectionData.BaseValue = BaseValue;
            
            m_ReflectionData->m_BoolVariables.push_back(ReflectionData);

            return At;
        }
        
        return VariableList<FrameGraph::Bool>().Add(Name, BaseValue);
    }

    template<> 
    FrameGraph::Location CommandContext::AddVariable<FrameGraph::UInt>(std::string_view Name, const FrameGraph::UInt& BaseValue) 
    {
#ifdef ALWAYS_REFLECT_VARIABLES
        if (m_ReflectionData.has_value())
        {
            Location At = VariableList<FrameGraph::UInt>().Add(Name, BaseValue);

            _Graph::ReflectedVariable<FrameGraph::UInt> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            ReflectionData.BaseValue = BaseValue;
            ReflectionData.MinValue = 0;
            ReflectionData.MaxValue = 0;
            
            m_ReflectionData->m_UIntVariables.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_VARIABLES
        
        return VariableList<FrameGraph::UInt>().Add(Name, BaseValue);
    }
    
    template<> 
    FrameGraph::Location CommandContext::AddVariable<FrameGraph::UInt>(std::string_view Name, const FrameGraph::UInt& BaseValue, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            Location At = VariableList<FrameGraph::UInt>().Add(Name, BaseValue);

            _Graph::ReflectedVariable<FrameGraph::UInt> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            ReflectionData.BaseValue = BaseValue;
            ReflectionData.MinValue = 0;
            ReflectionData.MaxValue = 0;
            
            m_ReflectionData->m_UIntVariables.push_back(ReflectionData);

            return At;
        }
        
        return VariableList<FrameGraph::UInt>().Add(Name, BaseValue);
    }
    
    template<> 
    FrameGraph::Location CommandContext::AddVariable<FrameGraph::UInt>(std::string_view Name, const FrameGraph::UInt& BaseValue, FrameGraph::UInt Min, FrameGraph::UInt Max, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            Location At = VariableList<FrameGraph::UInt>().Add(Name, BaseValue);

            _Graph::ReflectedVariable<FrameGraph::UInt> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            ReflectionData.BaseValue = BaseValue;
            ReflectionData.MinValue = Min;
            ReflectionData.MaxValue = Max;
            
            m_ReflectionData->m_UIntVariables.push_back(ReflectionData);

            return At;
        }
        
        return VariableList<FrameGraph::UInt>().Add(Name, BaseValue);
    }

    template<> 
    FrameGraph::Location CommandContext::AddVariable<FrameGraph::Int>(std::string_view Name, const FrameGraph::Int& BaseValue) 
    {
#ifdef ALWAYS_REFLECT_VARIABLES
        if (m_ReflectionData.has_value())
        {
            Location At = VariableList<FrameGraph::Int>().Add(Name, BaseValue);

            _Graph::ReflectedVariable<FrameGraph::Int> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            ReflectionData.BaseValue = BaseValue;
            ReflectionData.MinValue = 0;
            ReflectionData.MaxValue = 0;
            
            m_ReflectionData->m_IntVariables.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_VARIABLES
        
        return VariableList<FrameGraph::Int>().Add(Name, BaseValue);
    }

    template<> 
    FrameGraph::Location CommandContext::AddVariable<FrameGraph::Int>(std::string_view Name, const FrameGraph::Int& BaseValue, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            Location At = VariableList<FrameGraph::Int>().Add(Name, BaseValue);

            _Graph::ReflectedVariable<FrameGraph::Int> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            ReflectionData.BaseValue = BaseValue;
            ReflectionData.MinValue = 0;
            ReflectionData.MaxValue = 0;
            
            m_ReflectionData->m_IntVariables.push_back(ReflectionData);

            return At;
        }
        
        return VariableList<FrameGraph::Int>().Add(Name, BaseValue);
    }
    
    template<> 
    FrameGraph::Location CommandContext::AddVariable<FrameGraph::Int>(std::string_view Name, const FrameGraph::Int& BaseValue, FrameGraph::Int Min, FrameGraph::Int Max, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            Location At = VariableList<FrameGraph::Int>().Add(Name, BaseValue);

            _Graph::ReflectedVariable<FrameGraph::Int> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            ReflectionData.BaseValue = BaseValue;
            ReflectionData.MinValue = Min;
            ReflectionData.MaxValue = Max;
            
            m_ReflectionData->m_IntVariables.push_back(ReflectionData);

            return At;
        }
        
        return VariableList<FrameGraph::Int>().Add(Name, BaseValue);
    }
    
    template<> 
    FrameGraph::Location CommandContext::AddVariable<FrameGraph::Float>(std::string_view Name, const FrameGraph::Float& BaseValue) 
    {
#ifdef ALWAYS_REFLECT_VARIABLES
        if (m_ReflectionData.has_value())
        {
            Location At = VariableList<FrameGraph::Float>().Add(Name, BaseValue);

            _Graph::ReflectedVariable<FrameGraph::Float> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            ReflectionData.BaseValue = BaseValue;
            ReflectionData.MinValue = 0;
            ReflectionData.MaxValue = 0;
            
            m_ReflectionData->m_FloatVariables.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_VARIABLES
        
        return VariableList<FrameGraph::Float>().Add(Name, BaseValue);
    }

    template<> 
    FrameGraph::Location CommandContext::AddVariable<FrameGraph::Float>(std::string_view Name, const FrameGraph::Float& BaseValue, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            Location At = VariableList<FrameGraph::Float>().Add(Name, BaseValue);

            _Graph::ReflectedVariable<FrameGraph::Float> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            ReflectionData.BaseValue = BaseValue;
            ReflectionData.MinValue = 0;
            ReflectionData.MaxValue = 0;
            
            m_ReflectionData->m_FloatVariables.push_back(ReflectionData);

            return At;
        }
        
        return VariableList<FrameGraph::Float>().Add(Name, BaseValue);
    }
    
    template<> 
    FrameGraph::Location CommandContext::AddVariable<FrameGraph::Float>(std::string_view Name, const FrameGraph::Float& BaseValue, FrameGraph::Float Min, FrameGraph::Float Max, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            Location At = VariableList<FrameGraph::Float>().Add(Name, BaseValue);

            _Graph::ReflectedVariable<FrameGraph::Float> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            ReflectionData.BaseValue = BaseValue;
            ReflectionData.MinValue = Min;
            ReflectionData.MaxValue = Max;
            
            m_ReflectionData->m_FloatVariables.push_back(ReflectionData);

            return At;
        }
        
        return VariableList<FrameGraph::Float>().Add(Name, BaseValue);
    }

    template<> 
    FrameGraph::Location CommandContext::AddVariable<FrameGraph::Size2D>(std::string_view Name, const FrameGraph::Size2D& BaseValue) 
    {
#ifdef ALWAYS_REFLECT_VARIABLES
        if (m_ReflectionData.has_value())
        {
            Location At = VariableList<FrameGraph::Size2D>().Add(Name, BaseValue);

            _Graph::ReflectedVariable<FrameGraph::Size2D> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            ReflectionData.BaseValue = BaseValue;
            
            m_ReflectionData->m_Size2DVariables.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_VARIABLES
        
        return VariableList<FrameGraph::Size2D>().Add(Name, BaseValue);
    }

    template<>
    FrameGraph::Location CommandContext::AddVariable<FrameGraph::Size2D>(std::string_view Name, const FrameGraph::Size2D& BaseValue, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            Location At = VariableList<FrameGraph::Size2D>().Add(Name, BaseValue);

            _Graph::ReflectedVariable<FrameGraph::Size2D> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            ReflectionData.BaseValue = BaseValue;
            
            m_ReflectionData->m_Size2DVariables.push_back(ReflectionData);

            return At;
        }
        
        return VariableList<FrameGraph::Size2D>().Add(Name, BaseValue);
    }

    template<> 
    FrameGraph::Location CommandContext::AddVariable<FrameGraph::Rect>(std::string_view Name, const FrameGraph::Rect& BaseValue) 
    {
#ifdef ALWAYS_REFLECT_VARIABLES
        if (m_ReflectionData.has_value())
        {
            Location At = VariableList<FrameGraph::Rect>().Add(Name, BaseValue);

            _Graph::ReflectedVariable<FrameGraph::Rect> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            ReflectionData.BaseValue = BaseValue;
            
            m_ReflectionData->m_RectVariables.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_VARIABLES
        
        return VariableList<FrameGraph::Rect>().Add(Name, BaseValue);
    }

    template<>
    FrameGraph::Location CommandContext::AddVariable<FrameGraph::Rect>(std::string_view Name, const FrameGraph::Rect& BaseValue, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            Location At = VariableList<FrameGraph::Rect>().Add(Name, BaseValue);

            _Graph::ReflectedVariable<FrameGraph::Rect> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            ReflectionData.BaseValue = BaseValue;
            
            m_ReflectionData->m_RectVariables.push_back(ReflectionData);

            return At;
        }
        
        return VariableList<FrameGraph::Rect>().Add(Name, BaseValue);
    }

    template<> 
    FrameGraph::Location CommandContext::AddVariable<Math::Vector3f>(std::string_view Name, const Math::Vector3f& BaseValue) 
    {
#ifdef ALWAYS_REFLECT_VARIABLES
        if (m_ReflectionData.has_value())
        {
            Location At = VariableList<Math::Vector3f>().Add(Name, BaseValue);

            _Graph::ReflectedVariable<Math::Vector3f> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = {};
            ReflectionData.BaseValue = BaseValue;
            
            m_ReflectionData->m_Vec3Variables.push_back(ReflectionData);

            return At;
        }
#endif // ALWAYS_REFLECT_VARIABLES
        
        return VariableList<Math::Vector3f>().Add(Name, BaseValue);
    }

    template<>
    FrameGraph::Location CommandContext::AddVariable<Math::Vector3f>(std::string_view Name, const Math::Vector3f& BaseValue, std::string_view Description)
    {
        if (m_ReflectionData.has_value())
        {
            Location At = VariableList<Math::Vector3f>().Add(Name, BaseValue);

            _Graph::ReflectedVariable<Math::Vector3f> ReflectionData;
            ReflectionData.FieldName = Name;
            ReflectionData.HashedName = FrameGraph::ToName(Name);
            ReflectionData.Location = At;
            ReflectionData.Description = Description;
            ReflectionData.BaseValue = BaseValue;
            
            m_ReflectionData->m_Vec3Variables.push_back(ReflectionData);

            return At;
        }
        
        return VariableList<Math::Vector3f>().Add(Name, BaseValue);
    }

    template <> 
    std::span<const _Graph::ReflectedObject<VertexBuffer>> CommandContext::ReflectedObjects<VertexBuffer>() const
    {
        if (!m_ReflectionData.has_value()) return {};

        return m_ReflectionData->m_VertexBuffers;
    }
    
    template <> 
    std::span<const _Graph::ReflectedObject<IndexBuffer>> CommandContext::ReflectedObjects<IndexBuffer>() const
    {
        if (!m_ReflectionData.has_value()) return {};
        
        return m_ReflectionData->m_IndexBuffers;
    }
    
    template <> 
    std::span<const _Graph::ReflectedObject<VertexArrayObject>> CommandContext::ReflectedObjects<VertexArrayObject>() const
    {
        if (!m_ReflectionData.has_value()) return {};
        
        return m_ReflectionData->m_VAOs;
    }
    
    template <> 
    std::span<const _Graph::ReflectedObject<UniformBuffer>> CommandContext::ReflectedObjects<UniformBuffer>() const
    {
        if (!m_ReflectionData.has_value()) return {};
        
        return m_ReflectionData->m_UniformBuffers;
    }
    
    template <> 
    std::span<const _Graph::ReflectedObject<StorageBuffer>> CommandContext::ReflectedObjects<StorageBuffer>() const
    {
        if (!m_ReflectionData.has_value()) return {};
        
        return m_ReflectionData->m_StorageBuffers;
    }
    
    template <> 
    std::span<const _Graph::ReflectedObject<Texture2D>> CommandContext::ReflectedObjects<Texture2D>() const
    {
        if (!m_ReflectionData.has_value()) return {};
        
        return m_ReflectionData->m_Textures2D;
    }
    
    template <> 
    std::span<const _Graph::ReflectedObject<Texture2DArray>> CommandContext::ReflectedObjects<Texture2DArray>() const
    {
        if (!m_ReflectionData.has_value()) return {};
        
        return m_ReflectionData->m_Textures2DArray;
    }
    
    template <> 
    std::span<const _Graph::ReflectedObject<Texture3D>> CommandContext::ReflectedObjects<Texture3D>() const
    {
        if (!m_ReflectionData.has_value()) return {};
        
        return m_ReflectionData->m_Textures3D;
    }
    
    template <> 
    std::span<const _Graph::ReflectedObject<TextureCube>> CommandContext::ReflectedObjects<TextureCube>() const
    {
        if (!m_ReflectionData.has_value()) return {};
        
        return m_ReflectionData->m_TexturesCube;
    }
    
    template <> 
    std::span<const _Graph::ReflectedVariable<Bool>> CommandContext::ReflectedVariables<Bool>() const
    {
        if (!m_ReflectionData.has_value()) return {};
        
        return m_ReflectionData->m_BoolVariables;
    }
    
    template <> 
    std::span<const _Graph::ReflectedVariable<UInt>> CommandContext::ReflectedVariables<UInt>() const
    {
        if (!m_ReflectionData.has_value()) return {};
        
        return m_ReflectionData->m_UIntVariables;
    }
    
    template <> 
    std::span<const _Graph::ReflectedVariable<Int>> CommandContext::ReflectedVariables<Int>() const
    {
        if (!m_ReflectionData.has_value()) return {};
        
        return m_ReflectionData->m_IntVariables;
    }
    
    template <> 
    std::span<const _Graph::ReflectedVariable<Float>> CommandContext::ReflectedVariables<Float>() const
    {
        if (!m_ReflectionData.has_value()) return {};
        
        return m_ReflectionData->m_FloatVariables;
    }
    
    template <> 
    std::span<const _Graph::ReflectedVariable<Size2D>> CommandContext::ReflectedVariables<Size2D>() const
    {
        if (!m_ReflectionData.has_value()) return {};
        
        return m_ReflectionData->m_Size2DVariables;
    }
    
    template <> 
    std::span<const _Graph::ReflectedVariable<Rect>> CommandContext::ReflectedVariables<Rect>() const
    {
        if (!m_ReflectionData.has_value()) return {};
        
        return m_ReflectionData->m_RectVariables;
    }
    
    template <> 
    std::span<const _Graph::ReflectedVariable<Math::Vector3f>> CommandContext::ReflectedVariables<Math::Vector3f>() const
    {
        if (!m_ReflectionData.has_value()) return {};
        
        return m_ReflectionData->m_Vec3Variables;
    }

    bool CommandDebugViewList::IsDebugViewContextValid(Location Caller, Location DebugView)
    {
        for (const auto& link : m_DebugViewLinks)
        {
            if (Caller == link.Command && DebugView == link.DebugView) return true;
        }
        return false;
    }

    void CommandDebugViewList::Update(CommandContext& Resources, double DeltaTime)
    {
        for (auto& View : m_DebugViews)
        {
            View->OnUpdate(Resources, DeltaTime);
        }
    }

    void CommandDebugViewList::PrepareDebugView(CommandContext& Resources, double deltaTime, const ICommand& Caller, Location DebugView)
    {
        auto& View = m_DebugViews[DebugView];
        View->OnPrepare(Resources, Caller, deltaTime);
    }

    void CommandDebugViewList::RunDebugView(const CommandContext& Resources, const ICommand& Caller, std::string_view CallerName, Location DebugView)
    {
        auto& View = m_DebugViews[DebugView];
        auto& Name = m_DebugViewsNames[DebugView];
        DebugScopeMarker scope(std::string(Name.cppstring()).append(" on ").append(CallerName));

        View->OnExecute(Resources, Caller);
    }

    void CommandDebugViewList::ReloadShaders(CommandContext& Resources)
    {
        for (auto& view : m_DebugViews)
        {
            view->OnReloadShaders(Resources);
        }
    }

    void CommandDebugViewList::EditorUI(Location DebugView)
    {
        auto& View = m_DebugViews[DebugView];
        View->EditorUI();
    }

    void GraphDebugViewList::Update(CommandContext& Resources, double deltaTime)
    {
        for (auto& View : m_DebugViews)
        {
            View->OnUpdate(Resources, deltaTime);
        }
    }

    void GraphDebugViewList::RunDebugView(const CommandContext& Resources, Location DebugView)
    {
        auto& View = m_DebugViews[DebugView];
        auto& Name = m_DebugViewsNames[DebugView];
        DebugScopeMarker scope(Name.cppstring());

        View->OnExecute(Resources);
    }

    void GraphDebugViewList::ReloadShaders(CommandContext& Resources)
    {
        for (auto& view : m_DebugViews)
        {
            view->OnReloadShaders(Resources);
        }
    }

    void GraphDebugViewList::EditorUI(Location DebugView)
    {
        auto& View = m_DebugViews[DebugView];
        View->EditorUI();
    }

    CommandPool::CommandPool(bool EnableReflectionData, bool EnableDebugViews): m_Context(EnableReflectionData)
    {
        if (EnableDebugViews)
        {
            m_CommandDebugViews.emplace();
            m_GraphDebugViews.emplace();
        }
    }

    void CommandPool::ReloadShaders()
    {
        for (auto& command : m_Commands)
        {
            command->OnReloadShaders(m_Context);
        }
        
        if (m_CommandDebugViews.has_value())
        {
            m_CommandDebugViews->ReloadShaders(m_Context);
        }
    }

    void CommandPool::Update(double DeltaTime)
    {
        for (auto& command : m_Commands)
        {
            command->OnUpdate(m_Context, DeltaTime);
        }

        if (HasCommandDebugViews()) m_CommandDebugViews->Update(m_Context, DeltaTime);
        if (HasGraphDebugViews()) m_GraphDebugViews->Update(m_Context, DeltaTime);
        
        m_Context.UpdateCameras();
        m_Context.ConsumeVariableChanges();
    }

    void CommandPool::Render(std::span<const Location> CommandList) const
    {
        for (Location commandIndex : CommandList)
        {
            const auto& command = m_Commands[commandIndex];
            const auto& commandName = m_CommandNames[commandIndex];

            DebugScopeMarker scope(std::string_view(commandName.begin(), commandName.end()));
            
            command->OnExecute(m_Context);
        }
    }

    void CommandPool::RenderToCommandDebugView(double DeltaTime, std::span<const Location> CommandList, Location Command, Location DebugView)
    {
        if (!m_CommandDebugViews.has_value()) return;
        if (!m_CommandDebugViews->IsDebugViewContextValid(Command, DebugView)) return;
        ICommand& CommandInst = *(m_Commands[Command]);
        const auto& CommandName = m_CommandNames[Command];
        
        m_CommandDebugViews->PrepareDebugView(m_Context, DeltaTime, CommandInst, DebugView);

        for (Location commandIndex : CommandList)
        {
            const auto& command = m_Commands[commandIndex];
            const auto& commandName = m_CommandNames[commandIndex];

            DebugScopeMarker scope(std::string_view(commandName.begin(), commandName.end()));
            
            command->OnExecute(m_Context);

            if (Command == commandIndex)
            {
                m_CommandDebugViews->RunDebugView(m_Context, CommandInst, std::string_view(CommandName.begin(), CommandName.end()), DebugView);

                return;
            }
        }
    }

    void CommandPool::RenderToGraphDebugView(std::span<const Location> CommandList,
        Location Command, Location DebugView)
    {
        if (!m_GraphDebugViews.has_value()) return;
        ICommand& CommandInst = *(m_Commands[Command]);
        const auto& CommandName = m_CommandNames[Command];

        for (Location commandIndex : CommandList)
        {
            const auto& command = m_Commands[commandIndex];
            const auto& commandName = m_CommandNames[commandIndex];

            DebugScopeMarker scope(std::string_view(commandName.begin(), commandName.end()));
            
            command->OnExecute(m_Context);

            if (Command == commandIndex)
            {
                m_GraphDebugViews->RunDebugView(m_Context, DebugView);

                return;
            }
        }
    }

    IGraphDebugView* CommandPool::GetDebugView(Location DebugView) const
    {
        if (!HasGraphDebugViews()) return nullptr;

        return m_GraphDebugViews->GetDebugView(DebugView);
    }

    void CommandPool::CommandDebugViewEditorUI(Location DebugView)
    {
        if (!HasCommandDebugViews()) return;

        m_CommandDebugViews->EditorUI(DebugView);
    }

    void CommandPool::GraphDebugViewEditorUI(Location DebugView)
    {
        if (!HasGraphDebugViews()) return;

        m_GraphDebugViews->EditorUI(DebugView);
    }
}
