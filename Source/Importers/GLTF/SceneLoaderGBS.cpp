
#include "Importers/GLTF/SceneLoader.h"

#include <fstream>

// #include "Files/Files.h"

#include <stack>

namespace GBS
{
    static struct FileHeaderT
    {
        const char Text[34] = 
            "Graphics Box Scene file.\n"
            "Version ";
        
        const size_t Version = 1;
        
        const char ClosingText[2] = "\n";
    } FileHeader = {};
    
    static enum GeneralRegion : uint32_t
    {
        Materials = 0,
        Meshes,
        Textures,
        Transforms,
        Instances,
    };
    
    static struct TextureHeader
    {
        enum Type :uint8_t
        {
            Texture2D,
            Texture2DArray,
            // Texture3D,
        } type = Texture2D;
        Image::Type ImageType;
        Image::Layout Layout;
        Image::Encoding Encoding;
        
        uint32_t Width, Height, Depth;
    };
    
    static struct BufferHeader
    {
        size_t SizeInBytes;
    };
    
    static struct ArrayHeader
    {
        size_t SizeInBytes;
        size_t Count;
    };
    
    static struct ListHeader
    {
        size_t Count;
    };
    
    bool LoadGPUScene(const std::filesystem::path& path, GLTF::GPUScene& scene)
    {
        AssertOrErrorCallF(exists(path), return false, "No such file or directory \"%s\"", path.generic_string().c_str())
        if(!exists(path)) return false;
        
        std::string extension = path.extension().generic_string();
            
        if (extension == ".gbs") {}
        else
        {
            EngineLoggerErrorF("Unsupported file extension (%s) for gltf file", extension.c_str());
            return false;
        }
        
        // Read the entire file
        // std::string File = FileToString(path, true);
        
        // Destroy all previous objects
        scene.Clear();
        
        std::ifstream ifs(path, std::ios::binary);
        
        FileHeaderT header;
        ifs.read(reinterpret_cast<char*>(&FileHeader), sizeof(FileHeader));
        ifs.read(reinterpret_cast<char*>(&(scene.Extension)), sizeof(GLTF::GPUScene::Extension));
        
        ExportSettings exportSettings;
        ifs.read(reinterpret_cast<char*>(&(exportSettings.flags)), sizeof(ExportSettings::Flags));
        
        std::vector<uint8_t> ReadBuffer;
        
        // Textures
        {
            GeneralRegion region;
            ifs.read(reinterpret_cast<char*>(&region), sizeof(GeneralRegion));
            AssertOrError(region == Textures)
            
            std::vector<uint8_t> ReadCompressedBuffer;
            
            // Texture2Ds
            {
                ListHeader textureListHeader;
                ifs.read(reinterpret_cast<char*>(&textureListHeader), sizeof(ListHeader));
            
                scene.textures.reserve(textureListHeader.Count);
                for (size_t i = 0; i < textureListHeader.Count; ++i)
                {
                    TextureHeader textureHeader;
                    ifs.read(reinterpret_cast<char*>(&textureHeader), sizeof(TextureHeader));
                
                    Image Image 
                    {
                        textureHeader.Width, 
                        textureHeader.Height, 
                        textureHeader.ImageType,
                        textureHeader.Layout,
                    };
                
                    if (exportSettings.flags & ExportSettings::ImageCompression)
                    {
                        switch (textureHeader.ImageType) 
                        {
                        case Image::UnsignedByte:
                        case Image::Byte:
                            {
                                BufferHeader textureBufferHeader;
                                ifs.read(reinterpret_cast<char*>(&textureBufferHeader), sizeof(BufferHeader));
                                ReadCompressedBuffer.resize(textureBufferHeader.SizeInBytes);
                                ifs.read(reinterpret_cast<char*>(ReadCompressedBuffer.data()), textureBufferHeader.SizeInBytes);
                        
                                Image::FileType fileType;
                                fileType = (exportSettings.flags & ExportSettings::ImageCompressionJPG) ? Image::JPG : Image::PNG;
                        
                                ImageLoadFromMemory(ReadCompressedBuffer.data(), textureBufferHeader.SizeInBytes, fileType, Image);
                            }
                        break;

                        case Image::Float:
                            {
                                BufferHeader textureBufferHeader;
                                ifs.read(reinterpret_cast<char*>(&textureBufferHeader), sizeof(BufferHeader));
                                ReadCompressedBuffer.resize(textureBufferHeader.SizeInBytes);
                                ifs.read(reinterpret_cast<char*>(ReadCompressedBuffer.data()), textureBufferHeader.SizeInBytes);
                            
                                Image::FileType fileType;
                                fileType = Image::HDR;
                                if ((exportSettings.flags & ExportSettings::ImageCompressionEXR) && (textureHeader.Layout == Image::RGBA || textureHeader.Layout == Image::ABGR || textureHeader.Layout == Image::ARGB))
                                    fileType = Image::EXR;
                            
                                ImageLoadFromMemory(ReadCompressedBuffer.data(), textureBufferHeader.SizeInBytes, fileType, Image);
                            }
                        break;
                    
                        case Image::UnsignedShort:
                        case Image::Short:
                        case Image::UnsignedInt:
                        case Image::Int:
                        case Image::Double:
                            {                            
                                BufferHeader textureBufferHeader;
                                ifs.read(reinterpret_cast<char*>(&textureBufferHeader), sizeof(BufferHeader));
                                ifs.read(reinterpret_cast<char*>(Image.Data()), textureBufferHeader.SizeInBytes);
                            }
                        break;
                        }
                    }
                    else
                    {
                        BufferHeader textureBufferHeader;
                        ifs.read(reinterpret_cast<char*>(&textureBufferHeader), sizeof(BufferHeader));
                        ifs.read(reinterpret_cast<char*>(Image.Data()), textureBufferHeader.SizeInBytes);
                    }
                
                    scene.textures.emplace_back(Image);
                }
            }
            
            // Texture2DArrays
            {
                ListHeader texturesArraysListHeader;
                ifs.read(reinterpret_cast<char*>(&texturesArraysListHeader), sizeof(ListHeader));
                
                scene.texturesArrays.reserve(texturesArraysListHeader.Count);
                for (size_t i = 0; i < texturesArraysListHeader.Count; ++i)
                {                    
                    TextureHeader textureHeader;
                    ifs.read(reinterpret_cast<char*>(&textureHeader), sizeof(TextureHeader));
                    
                    scene.texturesArrays.emplace_back(textureHeader.Width, textureHeader.Height, textureHeader.Depth, Texture::ToTextureType(textureHeader.ImageType), Texture::ToTextureLayout(textureHeader.Layout), true);
                    auto& texture2DArray = scene.texturesArrays.back();
                    
                    Image Image {
                        textureHeader.Width, 
                        textureHeader.Height, 
                        textureHeader.ImageType,
                        textureHeader.Layout,
                    };
                
                    for (size_t layer = 0; layer < textureHeader.Depth; ++layer)
                    {                
                        if (exportSettings.flags & ExportSettings::ImageCompression)
                        {
                            switch (textureHeader.ImageType) 
                            {
                            case Image::UnsignedByte:
                            case Image::Byte:
                                {
                                    BufferHeader textureBufferHeader;
                                    ifs.read(reinterpret_cast<char*>(&textureBufferHeader), sizeof(BufferHeader));
                                    ReadCompressedBuffer.resize(textureBufferHeader.SizeInBytes);
                                    ifs.read(reinterpret_cast<char*>(ReadCompressedBuffer.data()), textureBufferHeader.SizeInBytes);
                        
                                    Image::FileType fileType;
                                    fileType = (exportSettings.flags & ExportSettings::ImageCompressionJPG) ? Image::JPG : Image::PNG;
                        
                                    ImageLoadFromMemory(ReadCompressedBuffer.data(), textureBufferHeader.SizeInBytes, fileType, Image);
                                }
                            break;

                            case Image::Float:
                                {
                                    BufferHeader textureBufferHeader;
                                    ifs.read(reinterpret_cast<char*>(&textureBufferHeader), sizeof(BufferHeader));
                                    ReadCompressedBuffer.resize(textureBufferHeader.SizeInBytes);
                                    ifs.read(reinterpret_cast<char*>(ReadCompressedBuffer.data()), textureBufferHeader.SizeInBytes);
                            
                                    Image::FileType fileType;
                                    fileType = Image::HDR;
                                    if ((exportSettings.flags & ExportSettings::ImageCompressionEXR) && (textureHeader.Layout == Image::RGBA || textureHeader.Layout == Image::ABGR || textureHeader.Layout == Image::ARGB))
                                        fileType = Image::EXR;
                            
                                    ImageLoadFromMemory(ReadCompressedBuffer.data(), textureBufferHeader.SizeInBytes, fileType, Image);
                                }
                            break;
                    
                            case Image::UnsignedShort:
                            case Image::Short:
                            case Image::UnsignedInt:
                            case Image::Int:
                            case Image::Double:
                                {
                                    BufferHeader textureBufferHeader;
                                    ifs.read(reinterpret_cast<char*>(&textureBufferHeader), sizeof(BufferHeader));
                                    ifs.read(reinterpret_cast<char*>(Image.Data()), textureBufferHeader.SizeInBytes);
                                }
                            }
                        }
                        else
                        {
                            BufferHeader textureBufferHeader;
                            ifs.read(reinterpret_cast<char*>(&textureBufferHeader), sizeof(BufferHeader));
                            ifs.read(reinterpret_cast<char*>(Image.Data()), textureBufferHeader.SizeInBytes);
                        }
                        
                        texture2DArray.SubData(layer, Image);
                    }
                }
            }
        }
        
        // Meshes
        {
            GeneralRegion region;
            ifs.read(reinterpret_cast<char*>(&region), sizeof(GeneralRegion));
            AssertOrError(region == Meshes)
            
            ListHeader meshListHeader;
            ifs.read(reinterpret_cast<char*>(&meshListHeader), sizeof(ListHeader));
            
            std::vector<VertexArrayObject::Layout::Element> ReadBufferLayout;
            std::vector<Mesh::VertexGroup> ReadVertexGroups;
            
            scene.meshes.resize(meshListHeader.Count);
            for (size_t i = 0; i < meshListHeader.Count; ++i)
            {
                Mesh::VertexType Type;
                ifs.read(reinterpret_cast<char*>(&Type), sizeof(Mesh::VertexType));
                
                MeshObject& mesh = scene.meshes[i];
                
                mesh.BeginMesh(Type);
                
                ListHeader layoutListHeader;
                ifs.read(reinterpret_cast<char*>(&layoutListHeader), sizeof(ListHeader));
                
                for (size_t vb = 0; vb < layoutListHeader.Count; ++vb)
                {
                    ArrayHeader layoutArrayHeader;
                    ifs.read(reinterpret_cast<char*>(&layoutArrayHeader), sizeof(ArrayHeader));
                    
                    ReadBufferLayout.resize(layoutArrayHeader.Count);
                    ifs.read(reinterpret_cast<char*>(ReadBufferLayout.data()), layoutArrayHeader.SizeInBytes);
                    
                    uint32_t Stride;
                    ifs.read(reinterpret_cast<char*>(&Stride), sizeof(uint32_t));
                    
                    BufferHeader vertexBufferHeader;
                    ifs.read(reinterpret_cast<char*>(&vertexBufferHeader), sizeof(BufferHeader));
                    
                    ReadBuffer.resize(vertexBufferHeader.SizeInBytes);
                    ifs.read(reinterpret_cast<char*>(ReadBuffer.data()),  vertexBufferHeader.SizeInBytes);
                    
                    VertexArrayObject::Layout layout;
                    layout.Replace(ReadBufferLayout, Stride);
                    
                    mesh.AddVertexBuffer(layout, ReadBuffer.data(), vertexBufferHeader.SizeInBytes);
                }
                
                IndexBuffer::IndexType IndexType;
                ifs.read(reinterpret_cast<char*>(&IndexType), sizeof(IndexBuffer::IndexType));
                    
                BufferHeader indexBufferHeader;
                ifs.read(reinterpret_cast<char*>(&indexBufferHeader), sizeof(BufferHeader));
                
                if (IndexType != IndexBuffer::IndexType::_Count)
                {
                    ReadBuffer.resize(indexBufferHeader.SizeInBytes);
                    ifs.read(reinterpret_cast<char*>(ReadBuffer.data()),  indexBufferHeader.SizeInBytes);
                    
                    mesh.SetIndexBuffer(IndexType, ReadBuffer.data(), indexBufferHeader.SizeInBytes / ToGLIndexSize(IndexType));
                }
                
                ArrayHeader vertexGroupsArrayHeader;
                ifs.read(reinterpret_cast<char*>(&vertexGroupsArrayHeader), sizeof(ArrayHeader));
                
                ReadVertexGroups.resize(vertexGroupsArrayHeader.Count);
                ifs.read(reinterpret_cast<char*>(ReadVertexGroups.data()), vertexGroupsArrayHeader.SizeInBytes);
                
                for (const auto & group : ReadVertexGroups) 
                    mesh.AddVertexGroup(group);
                
                mesh.EndMesh();
            }
        }
        
        // Materials
        {
            GeneralRegion region;
            ifs.read(reinterpret_cast<char*>(&region), sizeof(GeneralRegion));
            AssertOrError(region == Materials)
            
            // Unified Uniform buffer of materials
            if (scene.Extension & GLTF::GPUScene::MaterialsAsUnifiedBuffer)
            {
                BufferHeader materialBufferHeader;
                ifs.read(reinterpret_cast<char*>(&materialBufferHeader), sizeof(BufferHeader));
                
                std::vector<uint8_t> bufferData(materialBufferHeader.SizeInBytes);
                ifs.read(reinterpret_cast<char*>(bufferData.data()), materialBufferHeader.SizeInBytes);
                scene.unifiedMaterialBuffer.emplace(materialBufferHeader.SizeInBytes, bufferData.data());
                
                ArrayHeader materialViewArray;
                ifs.read(reinterpret_cast<char*>(&materialViewArray), sizeof(ArrayHeader));
                scene.unifiedMaterialOffsets.resize(materialViewArray.Count);
                ifs.read(reinterpret_cast<char*>(scene.unifiedMaterialOffsets.data()), materialViewArray.SizeInBytes);
            }

            // Separate Uniform buffer of materials
            {
                ArrayHeader materialsHeader;
                ifs.read(reinterpret_cast<char*>(&materialsHeader), sizeof(ArrayHeader));
                scene.materials.resize(materialsHeader.Count);
                ifs.read(reinterpret_cast<char*>(scene.materials.data()), materialsHeader.SizeInBytes);
                
                // Upload material buffer data to GPU
                if (scene.Extension & GLTF::GPUScene::MaterialsAsBuffers)
                {
                    scene.materialUniformBuffers.resize(scene.materials.size());
                    
                    for (size_t i = 0; i < scene.materialUniformBuffers.size(); ++i)
                    {
                        auto & materialUniformBuffer = scene.materialUniformBuffers[i];
                        materialUniformBuffer.Data(&(scene.materials[i]), sizeof(GLTF::Material));
                    }
                }
            }
        }

        // Transforms
        {
            GeneralRegion region;
            ifs.read(reinterpret_cast<char*>(&region), sizeof(GeneralRegion));
            AssertOrError(region == Transforms)
            
            ArrayHeader transformsArrayHeader;
            ifs.read(reinterpret_cast<char*>(&transformsArrayHeader), sizeof(ArrayHeader));
            
            scene.transforms.resize(transformsArrayHeader.Count);
            ifs.read(reinterpret_cast<char*>(scene.transforms.data()), transformsArrayHeader.SizeInBytes);
        }
        
        // Instances
        {
            GeneralRegion region;
            ifs.read(reinterpret_cast<char*>(&region), sizeof(GeneralRegion));
            AssertOrError(region == Instances)
            
            ArrayHeader instancesHeader;
            ifs.read(reinterpret_cast<char*>(&instancesHeader), sizeof(ArrayHeader));
            
            scene.instances.resize(instancesHeader.Count);
            ifs.read(reinterpret_cast<char*>(scene.instances.data()), instancesHeader.SizeInBytes);
        }
        
        return true;
    }

    bool SaveGPUScene(const std::filesystem::path& path, const GLTF::GPUScene& scene, ExportSettings settings)
    {
        // Ensure folder exist
        std::stack<std::filesystem::path> folderStack;
        std::filesystem::path folder = path;
        while (folder.has_parent_path() && !exists(folder.parent_path()))
        {
            folderStack.push(folder.parent_path());
            folder = folder.parent_path();
        }
        while (!folderStack.empty())
        {
            if (!exists(folderStack.top()))
            {
                std::filesystem::create_directory(folderStack.top());
            }
            folderStack.pop();
        }
                
        std::ofstream output(path, std::ios::binary);
        output.clear();
        
        // Header
        output.write(reinterpret_cast<const char*>(&FileHeader), sizeof(FileHeaderT));
        output.write(reinterpret_cast<const char*>(&(scene.Extension)), sizeof(GLTF::GPUScene::Extension));
        output.write(reinterpret_cast<const char*>(&(settings.flags)), sizeof(ExportSettings::Flags));
        
        // Textures
        {
            GeneralRegion region = Textures;
            output.write(reinterpret_cast<const char*>(&region), sizeof(GeneralRegion));
            
            // Texture2Ds
            {
                ListHeader textureListHeader;
                textureListHeader.Count = scene.textures.size();
                output.write(reinterpret_cast<const char*>(&textureListHeader), sizeof(ListHeader));
            
                for (size_t i = 0; i < textureListHeader.Count; ++i)
                {
                    const auto& texture = scene.textures[i];
                
                    Image Image {
                        texture.Width(), 
                        texture.Height(), 
                        Texture::ToImageType(texture.ComponentType()),
                        Texture::ToImageLayout(texture.ComponentLayout()),
                    };
                    texture.Export(Image);
                
                    TextureHeader textureHeader;
                    textureHeader.type = TextureHeader::Texture2D;
                    textureHeader.Width = Image.Width();
                    textureHeader.Height = Image.Height();
                    textureHeader.Depth = 1;
                    textureHeader.ImageType = Image.ComponentType();
                    textureHeader.Layout = Image.ComponentLayout();
                    textureHeader.Encoding = Image.ComponentEncoding();
                    output.write(reinterpret_cast<const char*>(&textureHeader), sizeof(TextureHeader));
                
                    if (settings.flags & ExportSettings::ImageCompression)
                    {
                        switch (textureHeader.ImageType) 
                        {
                        case Image::UnsignedByte:
                        case Image::Byte:
                            {
                                Image::FileType fileType;
                                fileType = (settings.flags & ExportSettings::ImageCompressionJPG) ? Image::JPG : Image::PNG;
                            
                                uint8_t* writeBuffer = nullptr;
                                size_t writeBufferSize = 0;
                                ImageStoreToMemory(Image, fileType, &writeBuffer, &writeBufferSize);
                    
                                BufferHeader textureBufferHeader;
                                textureBufferHeader.SizeInBytes = writeBufferSize;
                                output.write(reinterpret_cast<const char*>(&textureBufferHeader), sizeof(BufferHeader));
                                output.write(reinterpret_cast<const char*>(writeBuffer), textureBufferHeader.SizeInBytes);
                    
                                free(writeBuffer);
                            }
                        break;

                        case Image::Float:
                            {
                                Image::FileType fileType;
                                fileType = Image::HDR;
                                if ((settings.flags & ExportSettings::ImageCompressionEXR) && (textureHeader.Layout == Image::RGBA || textureHeader.Layout == Image::ABGR || textureHeader.Layout == Image::ARGB))
                                    fileType = Image::EXR;
                            
                                uint8_t* writeBuffer = nullptr;
                                size_t writeBufferSize = 0;
                                ImageStoreToMemory(Image, fileType, &writeBuffer, &writeBufferSize);
                    
                                BufferHeader textureBufferHeader;
                                textureBufferHeader.SizeInBytes = writeBufferSize;
                                output.write(reinterpret_cast<const char*>(&textureBufferHeader), sizeof(BufferHeader));
                                output.write(reinterpret_cast<const char*>(writeBuffer), textureBufferHeader.SizeInBytes);
                    
                                free(writeBuffer);
                            }
                        break;
                    
                        case Image::UnsignedShort:
                        case Image::Short:
                        case Image::UnsignedInt:
                        case Image::Int:
                        case Image::Double:
                            {
                                EngineLoggerWarn("Unsupported texture type for compression");
                            
                                BufferHeader textureBufferHeader;
                                textureBufferHeader.SizeInBytes = textureHeader.Width * textureHeader.Height * Image::PixelSize(textureHeader.ImageType, textureHeader.Layout) ;
                                output.write(reinterpret_cast<const char*>(&textureBufferHeader), sizeof(BufferHeader));
                                output.write(reinterpret_cast<const char*>(Image.Data()), textureBufferHeader.SizeInBytes);
                            }
                        break;
                        }
                    }
                    else
                    {
                        BufferHeader textureBufferHeader;
                        textureBufferHeader.SizeInBytes = textureHeader.Width * textureHeader.Height * Image::PixelSize(textureHeader.ImageType, textureHeader.Layout) ;
                        output.write(reinterpret_cast<const char*>(&textureBufferHeader), sizeof(BufferHeader));
                        output.write(reinterpret_cast<const char*>(Image.Data()), textureBufferHeader.SizeInBytes);
                    }
                }
            }
            
            // Texture2DArrays
            {
                ListHeader texturesArraysListHeader;
                texturesArraysListHeader.Count = scene.texturesArrays.size();
                output.write(reinterpret_cast<const char*>(&texturesArraysListHeader), sizeof(ListHeader));
            
                for (size_t i = 0; i < texturesArraysListHeader.Count; ++i)
                {
                    const auto& texture = scene.texturesArrays[i];
                
                    Image Image {
                        texture.Width(), 
                        texture.Height(), 
                        Texture::ToImageType(texture.ComponentType()),
                        Texture::ToImageLayout(texture.ComponentLayout()),
                    };
                
                    TextureHeader textureHeader;
                    textureHeader.type = TextureHeader::Texture2DArray;
                    textureHeader.Width = Image.Width();
                    textureHeader.Height = Image.Height();
                    textureHeader.Depth = texture.Count();
                    textureHeader.ImageType = Image.ComponentType();
                    textureHeader.Layout = Image.ComponentLayout();
                    textureHeader.Encoding = Image.ComponentEncoding();
                    output.write(reinterpret_cast<const char*>(&textureHeader), sizeof(TextureHeader));
                
                    for (size_t layer = 0; layer < textureHeader.Depth; ++layer)
                    {
                        texture.ExportSub(layer, Image);
                
                        if (settings.flags & ExportSettings::ImageCompression)
                        {
                            switch (textureHeader.ImageType) 
                            {
                            case Image::UnsignedByte:
                            case Image::Byte:
                                {
                                    Image::FileType fileType;
                                    fileType = (settings.flags & ExportSettings::ImageCompressionJPG) ? Image::JPG : Image::PNG;
                            
                                    uint8_t* writeBuffer = nullptr;
                                    size_t writeBufferSize = 0;
                                    ImageStoreToMemory(Image, fileType, &writeBuffer, &writeBufferSize);
                    
                                    BufferHeader textureBufferHeader;
                                    textureBufferHeader.SizeInBytes = writeBufferSize;
                                    output.write(reinterpret_cast<const char*>(&textureBufferHeader), sizeof(BufferHeader));
                                    output.write(reinterpret_cast<const char*>(writeBuffer), textureBufferHeader.SizeInBytes);
                    
                                    free(writeBuffer);
                                }
                            break;

                            case Image::Float:
                                {
                                    Image::FileType fileType;
                                    fileType = (settings.flags & ExportSettings::ImageCompressionEXR) ? Image::EXR : Image::HDR;
                            
                                    uint8_t* writeBuffer = nullptr;
                                    size_t writeBufferSize = 0;
                                    ImageStoreToMemory(Image, fileType, &writeBuffer, &writeBufferSize);
                    
                                    BufferHeader textureBufferHeader;
                                    textureBufferHeader.SizeInBytes = writeBufferSize;
                                    output.write(reinterpret_cast<const char*>(&textureBufferHeader), sizeof(BufferHeader));
                                    output.write(reinterpret_cast<const char*>(writeBuffer), textureBufferHeader.SizeInBytes);
                    
                                    free(writeBuffer);
                                }
                            break;
                    
                            case Image::UnsignedShort:
                            case Image::Short:
                            case Image::UnsignedInt:
                            case Image::Int:
                            case Image::Double:
                                {
                                    EngineLoggerWarn("Unsupported texture type for compression");
                            
                                    BufferHeader textureBufferHeader;
                                    textureBufferHeader.SizeInBytes = textureHeader.Width * textureHeader.Height * Image::PixelSize(textureHeader.ImageType, textureHeader.Layout) ;
                                    output.write(reinterpret_cast<const char*>(&textureBufferHeader), sizeof(BufferHeader));
                                    output.write(reinterpret_cast<const char*>(Image.Data()), textureBufferHeader.SizeInBytes);
                                }
                            }
                        }
                        else
                        {
                            BufferHeader textureBufferHeader;
                            textureBufferHeader.SizeInBytes = textureHeader.Width * textureHeader.Height * Image::PixelSize(textureHeader.ImageType, textureHeader.Layout) ;
                            output.write(reinterpret_cast<const char*>(&textureBufferHeader), sizeof(BufferHeader));
                            output.write(reinterpret_cast<const char*>(Image.Data()), textureBufferHeader.SizeInBytes);
                        }
                    }
                }
            }
        }
        
        // Meshes
        {
            GeneralRegion region = Meshes;
            output.write(reinterpret_cast<const char*>(&region), sizeof(GeneralRegion));
            
            ListHeader meshListHeader;
            meshListHeader.Count = scene.meshes.size();
            output.write(reinterpret_cast<const char*>(&meshListHeader), sizeof(ListHeader));
            
            std::vector<uint8_t> WriteBuffer;
            
            for (size_t i = 0; i < meshListHeader.Count; ++i)
            {
                const MeshObject& mesh = scene.meshes[i];
                auto vertexBuffersLayouts = mesh.GetLayouts();
                auto groups = mesh.GetGroups();
                auto vertexBuffers = mesh.GetVertexBuffers();
                const auto& indexBuffer = mesh.GetIndexBuffer();
                size_t vertexBufferCount = mesh.GetVertexBufferCount();
                
                Mesh::VertexType Type = mesh.GetVertexType();
                output.write(reinterpret_cast<const char*>(&Type), sizeof(Mesh::VertexType));
                
                ListHeader layoutListHeader;
                layoutListHeader.Count = vertexBufferCount;
                output.write(reinterpret_cast<const char*>(&layoutListHeader), sizeof(ListHeader));
                
                for (size_t vb = 0; vb < layoutListHeader.Count; ++vb)
                {
                    WriteBuffer.clear();
                    
                    const auto& layout = vertexBuffersLayouts[vb].GetElements();
                    
                    ArrayHeader layoutArrayHeader;
                    layoutArrayHeader.SizeInBytes = layout.size() * sizeof(VertexArrayObject::Layout::Element);
                    layoutArrayHeader.Count = layout.size();
                    output.write(reinterpret_cast<const char*>(&layoutArrayHeader), sizeof(ArrayHeader));
                    output.write(reinterpret_cast<const char*>(layout.data()), layoutArrayHeader.SizeInBytes);
                    
                    uint32_t Stride = vertexBuffersLayouts[vb].GetStride();
                    output.write(reinterpret_cast<const char*>(&Stride), sizeof(uint32_t));
                    
                    BufferHeader vertexBufferHeader;
                    vertexBufferHeader.SizeInBytes = vertexBuffers[vb].ExportSize();
                    output.write(reinterpret_cast<const char*>(&vertexBufferHeader), sizeof(BufferHeader));
                    
                    WriteBuffer.resize(vertexBufferHeader.SizeInBytes);
                    vertexBuffers[vb].Export(WriteBuffer.data(), vertexBufferHeader.SizeInBytes);
                    output.write(reinterpret_cast<const char*>(WriteBuffer.data()),  vertexBufferHeader.SizeInBytes);
                }
                
                if (mesh.GetIndexBuffer().has_value())
                {
                    IndexBuffer::IndexType Type = mesh.GetIndexBuffer()->GetIndexType();
                    output.write(reinterpret_cast<const char*>(&Type), sizeof(IndexBuffer::IndexType));
                    
                    BufferHeader indexBufferHeader;
                    indexBufferHeader.SizeInBytes = indexBuffer->ExportSize();
                    output.write(reinterpret_cast<const char*>(&indexBufferHeader), sizeof(BufferHeader));
                    
                    WriteBuffer.resize(indexBufferHeader.SizeInBytes);
                    indexBuffer->Export(WriteBuffer.data(), indexBufferHeader.SizeInBytes);
                    output.write(reinterpret_cast<const char*>(WriteBuffer.data()),  indexBufferHeader.SizeInBytes);
                }
                else
                {
                    IndexBuffer::IndexType Type = IndexBuffer::_Count;
                    output.write(reinterpret_cast<const char*>(&Type), sizeof(IndexBuffer::IndexType));
                    
                    BufferHeader indexBufferHeader;
                    indexBufferHeader.SizeInBytes = 0;
                    output.write(reinterpret_cast<const char*>(&indexBufferHeader), sizeof(BufferHeader));
                }
                
                ArrayHeader vertexGroupsArrayHeader;
                vertexGroupsArrayHeader.SizeInBytes = groups.size() * sizeof(Mesh::VertexGroup);
                vertexGroupsArrayHeader.Count = groups.size();
                output.write(reinterpret_cast<const char*>(&vertexGroupsArrayHeader), sizeof(ArrayHeader));
                output.write(reinterpret_cast<const char*>(groups.data()), vertexGroupsArrayHeader.SizeInBytes);
            }
        }
        
        // Materials
        {
            GeneralRegion region = Materials;
            output.write(reinterpret_cast<const char*>(&region), sizeof(GeneralRegion));
            
            // Unified Uniform buffer of materials
            if (scene.Extension & GLTF::GPUScene::MaterialsAsUnifiedBuffer)
            {
                size_t materialCount = (scene.materials.size() != scene.materialUniformBuffers.size() ? scene.materialUniformBuffers.size() : scene.materials.size());
                
                BufferHeader materialBufferHeader;
                materialBufferHeader.SizeInBytes = sizeof(GLTF::Material) * materialCount;
                output.write(reinterpret_cast<const char*>(&materialBufferHeader), sizeof(BufferHeader));
                
                std::vector<uint8_t> bufferData(materialBufferHeader.SizeInBytes);
                scene.unifiedMaterialBuffer->Export(bufferData.data(), materialBufferHeader.SizeInBytes);
                output.write(reinterpret_cast<const char*>(bufferData.data()), materialBufferHeader.SizeInBytes);
                
                ArrayHeader materialViewArray;
                materialViewArray.SizeInBytes = sizeof(size_t) * materialCount;
                materialViewArray.Count = materialCount;
                output.write(reinterpret_cast<const char*>(&materialViewArray), sizeof(ArrayHeader));
                output.write(reinterpret_cast<const char*>(scene.unifiedMaterialOffsets.data()), materialViewArray.SizeInBytes);
            }

            // Separate Uniform buffer of materials
            {
                std::vector<GLTF::Material> materialsExportBuffer{};
                std::span<const GLTF::Material> materialsToExport;
                
                // Retrieve material buffer data from GPU if out of date from GPU
                if (scene.Extension & GLTF::GPUScene::MaterialsAsBuffers && scene.materials.size() != scene.materialUniformBuffers.size())
                {
                    materialsExportBuffer.resize(scene.materialUniformBuffers.size());
                    materialsToExport = materialsExportBuffer;
                    
                    for (size_t i = 0; i < scene.materialUniformBuffers.size(); ++i)
                    {
                        const auto & materialUniformBuffer = scene.materialUniformBuffers[i];
                        materialUniformBuffer.Export(&(materialsExportBuffer[i]), sizeof(GLTF::Material));
                    }
                }
                else
                {
                    materialsToExport = scene.materials;
                }
                
                ArrayHeader materialsHeader;
                materialsHeader.SizeInBytes = materialsToExport.size() * sizeof(GLTF::Material);
                materialsHeader.Count = materialsToExport.size();
                output.write(reinterpret_cast<const char*>(&materialsHeader), sizeof(ArrayHeader));
                
                output.write(reinterpret_cast<const char*>(materialsToExport.data()), materialsHeader.SizeInBytes);
            }
        }
        
        // Transforms
        {
            GeneralRegion region = Transforms;
            output.write(reinterpret_cast<const char*>(&region), sizeof(GeneralRegion));
            
            ArrayHeader transformsHeader;
            transformsHeader.SizeInBytes = scene.transforms.size() * sizeof(GLTF::Transform);
            transformsHeader.Count = scene.transforms.size();
            output.write(reinterpret_cast<const char*>(&transformsHeader), sizeof(ArrayHeader));
            
            output.write(reinterpret_cast<const char*>(scene.transforms.data()), transformsHeader.SizeInBytes);
        }
        
        // Instances
        {
            GeneralRegion region = Instances;
            output.write(reinterpret_cast<const char*>(&region), sizeof(GeneralRegion));
            
            ArrayHeader instancesHeader;
            instancesHeader.SizeInBytes = scene.instances.size() * sizeof(GLTF::MeshInstance);
            instancesHeader.Count = scene.instances.size();
            output.write(reinterpret_cast<const char*>(&instancesHeader), sizeof(ArrayHeader));
            
            output.write(reinterpret_cast<const char*>(scene.instances.data()), instancesHeader.SizeInBytes);
        }
        
        output.close();
        return true;
    }
}
