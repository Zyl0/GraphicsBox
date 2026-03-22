#pragma once

#include <vector>

#include "Modeling/Mesh.h"
#include "Rendering/VertexArrayObject.h"
#include "Rendering/IndexBuffer.h"

#include "Core/Types.h"
#include "Rendering/Textures.h"

namespace Rendering::Resources
{    
    class MeshResourcePool
    {
    public:
        Engine::Handle AddResource(const Mesh& Mesh);
        
        void UpdateResource(Engine::Handle, const Mesh& Mesh);
        void UpdateResource(Engine::Handle, const Mesh& Mesh, size_t VertexGroup);
        
        void RemoveResource(Engine::Handle);
        
        bool IsValid(Engine::Handle Mesh);
        
        VertexArrayObject& GetVAO(Engine::Handle Mesh);
        
        VertexBuffer& GetVB(Engine::Handle Mesh, size_t Index);
        size_t GetVBCount(Engine::Handle Mesh);
        
        bool HasIB(Engine::Handle Mesh);
        IndexBuffer& GetIB(Engine::Handle Mesh);
        
    private:        
        struct MeshResource
        {
            VertexArrayObject VAO;
            std::vector<Engine::Handle> VertexBuffers;
            Engine::Handle IndexBuffer;
            bool IsValid;
        };
        
        std::vector<std::optional<VertexBuffer>> VertexBuffers;
        std::vector<std::optional<IndexBuffer>> IndexBuffers;
        std::vector<MeshResource> MeshResources;
    };
    
    extern MeshResourcePool g_MeshResources;
    
    class Texture2DResourcePool
    {
    public:
        Engine::Handle AddResource(uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout);
        Engine::Handle AddResource(const Image& Image, bool UseMips = true);
        
        void UpdateResource(Engine::Handle, uint32_t width, uint32_t height);
        void UpdateResource(Engine::Handle, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout);
        void UpdateResource(Engine::Handle, const Image& Image, bool UseMips = true);
        
        void ExportResource(Engine::Handle, Image& Export);
        
        void RemoveResource(Engine::Handle);
        
        bool IsTexture(Engine::Handle Mesh);
        
        Texture2D& GetTexture2D(Engine::Handle Mesh);
    
    private:
        std::vector<std::optional<Texture2D>> Texture2Ds;
    };
    
    extern Texture2DResourcePool g_Texture2DResources;
}
