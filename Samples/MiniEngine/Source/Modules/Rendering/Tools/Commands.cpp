#include "Modules/Rendering/Tools/Commands.h"

#include <cstring>
#include "Rendering/Debug.h"

namespace Rendering::Graph
{
    Location CameraArray::AddCamera()
    {
        Location Index;
        
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

    Location CameraArray::AddCamera(const Camera& camera)
    {
        Location Index = AddCamera();
        UpdateCameraData(m_CPUArray[Index], camera);
        
        return Index;
    }

    Location CameraArray::AddCamera(const CameraData& camera)
    {
        Location Index = AddCamera();
        std::memcpy(&(m_CPUArray[Index]), &camera, sizeof(CameraData));
        
        return Index;
    }

    void CameraArray::UpdateCamera(Location Index, const Camera& camera)
    {
        UpdateCameraData(m_CPUArray[Index], camera);
        m_HasChanged = true;
    }

    void CameraArray::UpdateCamera(Location Index, const CameraData& camera)
    {
        std::memcpy(&(m_CPUArray[Index]), &camera, sizeof(CameraData));
        m_HasChanged = true;
    }

    void CommandContext::UpdateCameras()
    {
        m_CameraArray.Update();
    }

    void CommandContext::ConsumeVariableChanges()
    {
        VariableList<Bool>().Reset();
        VariableList<UInt>().Reset();
        VariableList<Int>().Reset();
        VariableList<Float>().Reset();
        VariableList<Size2D>().Reset();
        VariableList<Rect>().Reset();
    }

    template <>
    Location CommandContext::Add<VertexBuffer>(std::string_view Name)
    {
        return ObjectList<VertexBuffer>().Add(Name);
    }

    template <>
    Location CommandContext::Add<IndexBuffer>(std::string_view Name)
    {
        return ObjectList<IndexBuffer>().Add(Name);
    }

    template <>
    Location CommandContext::Add<VertexArrayObject>(std::string_view Name)
    {
        return ObjectList<VertexArrayObject>().Add(Name);
    }

    template <>
    Location CommandContext::Add<UniformBuffer>(std::string_view Name)
    {
        return ObjectList<UniformBuffer>().Add(Name);
    }

    template <>
    Location CommandContext::Add<UniformBuffer>(std::string_view Name, uint32_t Size)
    {
        return Add<UniformBuffer>(Name, Size, (const void*)nullptr);
    }

    template <>
    Location CommandContext::Add<UniformBuffer>(std::string_view Name, uint32_t Size, const void* data)
    {
        return ObjectList<UniformBuffer>().Add(Name, Size, data);
    }

    template <>
    Location CommandContext::Add<StorageBuffer>(std::string_view Name)
    {
        return ObjectList<StorageBuffer>().Add(Name);
    }

    template <>
    Location CommandContext::Add<StorageBuffer>(std::string_view Name, uint32_t Size)
    {
        return ObjectList<StorageBuffer>().Add(Name, Size);
    }

    template <>
    Location CommandContext::Add<StorageBuffer>(std::string_view Name, uint32_t Size, const void* data)
    {
        return ObjectList<StorageBuffer>().Add(Name, Size, data);
    }

    // template <>
    // Location CommandContext::Add<MeshObject>(std::string_view Name)
    // {
    //     return ObjectList<MeshObject>().Add(Name);
    // }

    // template <>
    // Location CommandContext::Add<MeshObject>(std::string_view Name, const Mesh& Mesh)
    // {
    //     return ObjectList<MeshObject>().Add(Name, Mesh);
    // }

    template <>
    Location CommandContext::Add<Texture2D>(std::string_view Name, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout)
    {
        return ObjectList<Texture2D>().Add(Name, width, height, type, layout, (uint8_t)0);
    }

    template <>
    Location CommandContext::Add<Texture2D>(std::string_view Name, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout, uint8_t SampleCount)
    {
        return ObjectList<Texture2D>().Add(Name, width, height, type, layout, SampleCount);
    }

    template <>
    Location CommandContext::Add<Texture2D>(std::string_view Name, const Image& Image)
    {
        return ObjectList<Texture2D>().Add(Name, Image, true);
    }

    template <>
    Location CommandContext::Add<Texture2D>(std::string_view Name, const Image& Image, bool UseMips)
    {
        return ObjectList<Texture2D>().Add(Name, Image, UseMips);
    }

    template <>
    Location CommandContext::Add<Texture2D>(std::string_view Name, uint32_t width, uint32_t height, Image::Type type, Image::Layout layout, const void* ImageData, size_t ImageSize)
    {
        return ObjectList<Texture2D>().Add(Name, width, height, type, layout, ImageData, ImageSize, true);
    }

    template <>
    Location CommandContext::Add<Texture2D>(std::string_view Name, uint32_t width, uint32_t height, Image::Type type, Image::Layout layout, const void* ImageData, size_t ImageSize, bool UseMips)
    {
        return ObjectList<Texture2D>().Add(Name, width, height, type, layout, ImageData, ImageSize, UseMips);
    }

    template <>
    Location CommandContext::Add<Texture2DArray>(std::string_view Name, uint32_t width, uint32_t height, uint32_t count, Texture::Type type, Texture::Layout layout)
    {
        return ObjectList<Texture2DArray>().Add(Name, width, height, count, type, layout, false);
    }

    template <>
    Location CommandContext::Add<Texture2DArray>(std::string_view Name, uint32_t width, uint32_t height, uint32_t count, Texture::Type type, Texture::Layout layout, bool UseMips)
    {
        return ObjectList<Texture2DArray>().Add(Name, width, height, count, type, layout, UseMips);
    }

    template <>
    Location CommandContext::Add<Texture3D>(std::string_view Name, uint32_t width, uint32_t height, uint32_t depth, Texture::Type type, Texture::Layout layout)
    {
        return ObjectList<Texture3D>().Add(Name, width, height, depth, type, layout);
    }

    template <>
    Location CommandContext::Add<Texture3D, const ImageCube&>(std::string_view Name, const ImageCube& Image)
    {
        return ObjectList<Texture3D>().Add(Name, Image, true);
    }

    template <>
    Location CommandContext::Add<Texture3D, const ImageCube&, bool>(std::string_view Name, const ImageCube& Image, bool UseMips)
    {
        return ObjectList<Texture3D>().Add(Name, Image, UseMips);
    }

    template <>
    Location CommandContext::Add<TextureCube>(std::string_view Name, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout)
    {
        return ObjectList<TextureCube>().Add(Name, width, height, type, layout, false);
    }

    template <>
    Location CommandContext::Add<TextureCube>(std::string_view Name, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout, bool UseMips)
    {
        return ObjectList<TextureCube>().Add(Name, width, height, type, layout, UseMips);
    }

    template <>
    Location CommandContext::Add<TextureCube>(std::string_view Name, std::span<const TextureCube::FacePair> Faces)
    {
        return ObjectList<TextureCube>().Add(Name, Faces, true);
    }

    template <>
    Location CommandContext::Add<TextureCube>(std::string_view Name, std::span<const TextureCube::FacePair> Faces, bool UseMips)
    {
        return ObjectList<TextureCube>().Add(Name, Faces, UseMips);
    }

    // template <>
    // Location CommandContext::Add<TextureCubeView>(std::string_view Name, const TextureCube& texture, uint32_t MipLevel)
    // {
    //     return Add<TextureCubeView, const TextureCube&>(Name, texture, MipLevel, 1u);
    // }
    // template <>
    // Location CommandContext::Add<TextureCubeView>(std::string_view Name, const TextureCube& texture, uint32_t MipLevel, uint32_t MipCount)
    // {
    //     return ObjectList<TextureCubeView>().Add<const TextureCube&>(Name, texture, MipLevel, MipCount);
    // }

    // template <>
    // Location CommandContext::Add<TextureCubeView>(std::string_view Name, std::string_view texture, uint32_t MipLevel)
    // {
    //     return Add<TextureCubeView>(Name, texture, MipLevel, 1u);
    // }

    // template <>
    // Location CommandContext::Add<TextureCubeView>(std::string_view Name, std::string_view texture, uint32_t MipLevel, uint32_t MipCount)
    // {
    //     return ObjectList<TextureCubeView>().Add(Name, ObjectList<TextureCube>().Get(ToName(texture)), MipLevel, MipCount);
    // }

    // template <>
    // Location CommandContext::Add<TextureCubeView>(std::string_view Name, Location texture, uint32_t MipLevel)
    // {
    //     return Add<TextureCubeView>(Name, texture, MipLevel, 1u);
    // }

    // template <>
    // Location CommandContext::Add<TextureCubeView>(std::string_view Name, Location texture, uint32_t MipLevel, uint32_t MipCount)
    // {
    //     return ObjectList<TextureCubeView>().Add(Name, ObjectList<TextureCube>()[texture], MipLevel, MipCount);
    // }
    
    template<> 
    Location CommandContext::AddVariable<Bool>(std::string_view Name, const Bool& BaseValue) 
    {
        return VariableList<Bool>().Add(Name, BaseValue);
    }
    
    template<> 
    Location CommandContext::AddVariable<UInt>(std::string_view Name, const UInt& BaseValue) 
    {
        return VariableList<UInt>().Add(Name, BaseValue);
    }
    
    template<> 
    Location CommandContext::AddVariable<Int>(std::string_view Name, const Int& BaseValue) 
    {
        return VariableList<Int>().Add(Name, BaseValue);
    }
    
    template<> 
    Location CommandContext::AddVariable<Float>(std::string_view Name, const Float& BaseValue) 
    {
        return VariableList<Float>().Add(Name, BaseValue);
    }
    
    template<> 
    Location CommandContext::AddVariable<Size2D>(std::string_view Name, const Size2D& BaseValue) 
    {
        return VariableList<Size2D>().Add(Name, BaseValue);
    }
    
    template<> 
    Location CommandContext::AddVariable<Rect>(std::string_view Name, const Rect& BaseValue) 
    {
        return VariableList<Rect>().Add(Name, BaseValue);
    }
    
    template<> 
    Location CommandContext::AddVariable<Math::Vector3f>(std::string_view Name, const Math::Vector3f& BaseValue) 
    {
        return VariableList<Math::Vector3f>().Add(Name, BaseValue);
    }

    void CommandList::ReloadShaders()
    {
        for (auto& command : m_Commands)
        {
            command.node->OnReloadShaders(m_Context);
        }
    }

    void CommandList::Update(double DeltaTime)
    {
        for (auto& command : m_Commands)
        {
            command.node->OnUpdate(m_Context, DeltaTime);
        }
        
        m_Context.UpdateCameras();
        m_Context.ConsumeVariableChanges();
    }

    void CommandList::Render() const
    {
        for (const auto& command : m_Commands)
        {
            DebugScopeMarker scope(std::string_view(command.name.begin(), command.name.end()));
            
            command.node->OnExecute(m_Context);
        }
    }
}
