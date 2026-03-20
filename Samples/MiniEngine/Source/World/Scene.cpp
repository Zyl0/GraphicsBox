#include "World/Scene.h"

#include <tiny_gltf.h>

#include "Math/Math.h"
#include "Shared/Assertion.h"


namespace Engine
{
    SceneResources g_SceneResources;
    Resources g_Scene;
}

bool TryLoopOverBufferFromAccessor(
    const tinygltf::Model& model, uint32_t AccessorIndex, const std::vector<uint32_t>* OptIterationIndexList,
    const std::function<bool(const tinygltf::Accessor& Accessor)>& execCheckType,
    const std::function<void(const void* sample, size_t byteStride, const tinygltf::Accessor& Accessor)>& execLoopBody)
{
    bool useIndexes = OptIterationIndexList != nullptr;

    const auto& accessor = model.accessors[AccessorIndex];
    if(!execCheckType(accessor)) return false;
    
    const auto& bufferView = model.bufferViews[accessor.bufferView];
    const auto& buffer = model.buffers[bufferView.buffer];

    const uint8_t* rawBuffer = buffer.data.data() + bufferView.byteOffset;
    const size_t rawBufferSize = bufferView.byteLength;
    const size_t byteStride = tinygltf::GetComponentSizeInBytes(accessor.componentType) * tinygltf::GetNumComponentsInType(accessor.type);

    if(useIndexes)
    {
        for (uint32_t index : *OptIterationIndexList)
        {
            size_t offset = accessor.byteOffset + index * (byteStride + accessor.byteOffset);
            Assert(offset + byteStride <= rawBufferSize);
            const int8_t* sample = (int8_t*)(rawBuffer + offset);
            execLoopBody(sample, byteStride, accessor);
        }
    }
    else
    {
        size_t offset = accessor.byteOffset;

        while(offset + byteStride <= rawBufferSize)
        {
            const int8_t* sample = (int8_t*)(rawBuffer + offset);
            execLoopBody(sample, byteStride, accessor);
            offset += (byteStride + accessor.byteOffset);
        }
    }

    return true;
}

Math::Transform4f GetNodeTransform(const tinygltf::Model& model, const tinygltf::Node& NodeObject)
{
    Math::Transform4f transform;
    if(!NodeObject.matrix.empty())
    {
        transform = Math::Transform4f(
            static_cast<float>(NodeObject.matrix[0]), static_cast<float>(NodeObject.matrix[3]), static_cast<float>(NodeObject.matrix[7]), static_cast<float>(NodeObject.matrix[11]),
            static_cast<float>(NodeObject.matrix[1]), static_cast<float>(NodeObject.matrix[4]), static_cast<float>(NodeObject.matrix[8]), static_cast<float>(NodeObject.matrix[12]),
            static_cast<float>(NodeObject.matrix[2]), static_cast<float>(NodeObject.matrix[5]), static_cast<float>(NodeObject.matrix[9]), static_cast<float>(NodeObject.matrix[13])
        );
    }
    else
    {
        Math::Transform4f Translation = Math::MakeHomogeneousIdentity<float>();
        if(!NodeObject.translation.empty())
        {
            Translation = Math::MakeHomogeneousTranslation(
                static_cast<float>(NodeObject.translation[0]),
                static_cast<float>(NodeObject.translation[1]),
                static_cast<float>(NodeObject.translation[2]));
        }
        Math::Transform4f Rotation = Math::MakeHomogeneousIdentity<float>();
        if(!NodeObject.rotation.empty())
        {
            Rotation = Math::Transform4f(
                Math::QuaternionF(
                    static_cast<float>(NodeObject.rotation[0]),
                    static_cast<float>(NodeObject.rotation[1]),
                    static_cast<float>(NodeObject.rotation[2]),
                    static_cast<float>(NodeObject.rotation[3])
                ).GetRotationMatrix());
        }
        Math::Transform4f Scale = Math::MakeHomogeneousIdentity<float>();
        if(!NodeObject.scale.empty())
        {
            Scale = Math::MakeHomogeneousScale(
                static_cast<float>(NodeObject.scale[0]),
                static_cast<float>(NodeObject.scale[1]),
                static_cast<float>(NodeObject.scale[2]));
        }
        transform = Translation * Rotation * Scale;                    
    }

    return transform;
}

Math::WorldTransformF GetNodeWorldTransform(const tinygltf::Model& model, const tinygltf::Node& NodeObject)
{
    Math::WorldTransformF transform;
    AssertOrError(NodeObject.matrix.empty(), "Matrix transform are not yet supported") // todo support matrix parameters extraction

    if(!NodeObject.translation.empty())
    transform.Position = Math::Point3f(
        static_cast<float>(NodeObject.translation[0]),
        static_cast<float>(NodeObject.translation[1]),
        static_cast<float>(NodeObject.translation[2])
        );

    if(!NodeObject.rotation.empty())
    transform.Rotation = Math::QuaternionF(
        static_cast<float>(NodeObject.rotation[0]),
        static_cast<float>(NodeObject.rotation[1]),
        static_cast<float>(NodeObject.rotation[2]),
        static_cast<float>(NodeObject.rotation[3])
        );

    if(!NodeObject.scale.empty())
    transform.Scale = Math::Vector3f(
        static_cast<float>(NodeObject.scale[0]),
        static_cast<float>(NodeObject.scale[1]),
        static_cast<float>(NodeObject.scale[2])
        );
    
    return transform;
}

void BuildDrawCallsSceneTree(
    const tinygltf::Model& model, const SharedPtr<engine::rendering::Renderer>& Renderer, const std::map<engine::Name, std::vector<engine::Name>>& MeshesMaterial,
    int node,
    SharedPtr<SceneHandle>& outScene, const Math::WorldTransformF& parent = WorldTransformF()
    )
{
    const auto& NodeObject = model.nodes[node];
    
    Math::WorldTransformF transform = GetNodeWorldTransform(model, NodeObject);

    transform = parent * transform;
    
    // Recursive call
    for (int child : NodeObject.children)
        BuildDrawCallsSceneTree(model, Renderer, MeshesMaterial, child, outScene, transform);
    
    // Skip unsupported scene elements
    if(NodeObject.mesh == -1) return;

    size_t drawCallIndex = outScene->m_DrawCalls.size();
    outScene->m_DrawCalls.emplace_back( Renderer->AddMesh() );

    engine::Name meshName = outScene->m_Meshes[NodeObject.mesh];
    outScene->m_DrawCalls[drawCallIndex].primitive.name = meshName;

    for (engine::Name material : MeshesMaterial.at(meshName))
        outScene->m_DrawCalls[drawCallIndex].primitive.materials->push_back(material);

    outScene->m_DrawCalls[drawCallIndex].primitive.transform = transform;
}


bool LoadGLTFMesh(const std::filesystem::path& Filename, Mesh& OutMesh)
{
    tinygltf::TinyGLTF loader;

    tinygltf::Model model;
    std::string ErrorBuffer;
    std::string WarningBuffer;
    std::vector<std::string> SupportedExtensions = {"glb", "gltf"};
    int ext = engine::files::CheckFileExtension(path.string(), SupportedExtensions);

    switch (ext)
    {
        case 0:
            loader.LoadBinaryFromFile(&model, &ErrorBuffer, &WarningBuffer, path.string().c_str());
            break;
        case 1:
            loader.LoadASCIIFromFile(&model, &ErrorBuffer, &WarningBuffer, path.string().c_str());
            break;
        default:
            EngineLoggerWarnF("Unsupported glTF file extension %s", path.string().c_str());
            return MakeShared<SceneHandle>();
    }    

    AssertOrErrorCallF(ErrorBuffer.empty(), return MakeShared<SceneHandle>(), "Failed to load scene %s. Loader returned the following error(s) : %s", path.string().c_str(), ErrorBuffer.c_str());

    AssertOrWarnCallF(WarningBuffer.empty(), , "Loaded scene %s with the following warning(s) : %s", path.string().c_str(), WarningBuffer.c_str());

    auto Renderer = engine::RenderBox::Get()->GetModule<engine::rendering::Renderer>().Lock();
    auto ResourceHolder = engine::RenderBox::Get()->GetModule<engine::assets::ResourceHolder>().Lock();
    
    SharedPtr<SceneHandle> scene = MakeShared<SceneHandle>();

    scene->m_Textures.reserve(!model.textures.empty() ? model.textures.size() : 0);
    scene->m_Materials.reserve(!model.materials.empty() ? model.materials.size() : 0);
    scene->m_Meshes.reserve(!model.meshes.empty() ? model.meshes.size() : 0);

    std::map<engine::Name, std::vector<engine::Name>> MeshesMaterial;

    // Unpack GLTF Scene content
    for (const auto & texture : model.textures)
    {
        engine::Name TextureName;
        if(texture.name.empty())
            TextureName = engine::Name::NAME("TEXTURE_" + std::to_string(scene->m_Textures.size()) );
        else
            TextureName = engine::Name::NAME(texture.name);
        engine::rhal::Texture2D& TextureObject = ResourceHolder->AddEmplace<engine::rhal::Texture2D>(TextureName);
        scene->m_Textures.push_back(TextureName);

        const auto & image = model.images[texture.source];
        AssertOrErrorCall(!image.as_is, continue;, "Custom image file type are not supported")

        engine::rhal::Texture::DataFormat Format;
        engine::rhal::Texture::InternalDataFormat InternalFormat;
        switch(image.component)
        {
        case 1:
            Format = engine::rhal::Texture::RED;
            InternalFormat = engine::rhal::Texture::Internal_RED;
            break;
        case 2: 
            Format = engine::rhal::Texture::RG;
            InternalFormat = engine::rhal::Texture::Internal_RG;
            break;
        case 3: 
            Format = engine::rhal::Texture::RGB;
            InternalFormat = engine::rhal::Texture::Internal_RGB;
            break;
        case 4: 
            Format = engine::rhal::Texture::RGBA;
            InternalFormat = engine::rhal::Texture::Internal_RGBA;
            break;
        default:
            EngineLoggerWarnF("Unsupported image %s, unsupported component count (%d)", texture.name.c_str(), image.component);
            continue;
        }

        engine::rhal::Texture::DataType DataType = engine::rhal::Texture::UNSIGNED_BYTE;
        switch(image.pixel_type)
        {
            case TINYGLTF_COMPONENT_TYPE_BYTE: 
                DataType = engine::rhal::Texture::BYTE;
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: 
                DataType = engine::rhal::Texture::UNSIGNED_BYTE;
                if(Format == engine::rhal::Texture::RGBA)
                {
                    //DataType = engine::rhal::Texture::UNSIGNED_INT_8_8_8_8;
                    InternalFormat = engine::rhal::Texture::Internal_RGBA8;
                }
                break;
            case TINYGLTF_COMPONENT_TYPE_SHORT: 
                DataType = engine::rhal::Texture::SHORT;
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: 
                DataType = engine::rhal::Texture::UNSIGNED_SHORT;
                if(Format == engine::rhal::Texture::RGBA)
                {
                    InternalFormat = engine::rhal::Texture::Internal_RGBA16;
                }
                break;
            case TINYGLTF_COMPONENT_TYPE_INT: 
                DataType = engine::rhal::Texture::INT;
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: 
                DataType = engine::rhal::Texture::UNSIGNED_INT;
                break;
            case TINYGLTF_COMPONENT_TYPE_FLOAT: 
                DataType = engine::rhal::Texture::FLOAT;
                break;
            case TINYGLTF_COMPONENT_TYPE_DOUBLE: 
                DataType = engine::rhal::Texture::DOUBLE;
                break;
        }

        TextureObject.TextureData(
                        image.image.data(), image.width, image.height,
                        Format, DataType, InternalFormat,
                        true
                        );
    }

    for (const auto & material : model.materials)
    {
        // todo improve memory pool to specify chunk size (because 4gb is insane)
        
        engine::Name MaterialName = engine::Name::NAME(material.name);
        scene->m_Materials.push_back(MaterialName);
        engine::rendering::Material& MaterialObject = ResourceHolder->Add<engine::rendering::Material>(
               MaterialName,
               engine::rendering::MaterialBuilder().SetMain(sh_PBRMaterial).Build()
           );
        engine::rendering::ProgramLibBasePBRMaterial::Parameters* parameters =
            MaterialObject.GetUniformStructureAs<engine::rendering::ProgramLibBasePBRMaterial::Parameters>().Get();

        // Unpack color
        parameters->BaseColorValue = Math::Vector3f(
            static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[0]),
            static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[1]),
            static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[2])
        );
        if(material.pbrMetallicRoughness.baseColorTexture.index > 0)
        {
            parameters->UseColorTexture = true;
            parameters->BaseColor =  ResourceHolder->Get<engine::rhal::Texture2D>(
               scene->m_Textures[material.pbrMetallicRoughness.baseColorTexture.index]
            )->GetReference();
        }

        // Unpack reflection
        parameters->RoughnessValue = static_cast<float>(material.pbrMetallicRoughness.roughnessFactor);
        parameters->MetalicValue = static_cast<float>(material.pbrMetallicRoughness.metallicFactor);
        if(material.pbrMetallicRoughness.metallicRoughnessTexture.index > 0)
        {
            parameters->UsePackedSRM = true;
            parameters->ReflectionSMR =  ResourceHolder->Get<engine::rhal::Texture2D>(
                scene->m_Textures[material.pbrMetallicRoughness.metallicRoughnessTexture.index]
            )->GetReference();
        }

        // Unpack AO
        if(material.occlusionTexture.index > 0)
        {
            parameters->UseAmbiantOclusionTexture = true;
            parameters->AmbiantOclusionMap =  ResourceHolder->Get<engine::rhal::Texture2D>(
                scene->m_Textures[material.occlusionTexture.index]
            )->GetReference();
        }

        // Get normal map
        if(material.normalTexture.index > 0)
        {
            parameters->UseNormalMap = true;
            parameters->NormalMap =  ResourceHolder->Get<engine::rhal::Texture2D>(
               scene->m_Textures[material.normalTexture.index]
            )->GetReference();
        }

        MaterialObject.SetTwoSidedDraw(material.doubleSided);

        // todo support AO and other gltf material properties
    }

    for (const auto & mesh : model.meshes)
    {
        engine::Name MeshName = engine::Name::NAME(mesh.name);
        Mesh& MeshObject = ResourceHolder->AddEmplace<Mesh>(MeshName);
        scene->m_Meshes.push_back(MeshName);

        const int MainPrimitiveType = mesh.primitives[0].mode;
        switch (MainPrimitiveType)
        {
        case TINYGLTF_MODE_POINTS: MeshObject.BeginMesh(engine::rhal::geometry::POINTS); break;
        case TINYGLTF_MODE_LINE: MeshObject.BeginMesh(engine::rhal::geometry::LINES); break;
        case TINYGLTF_MODE_LINE_LOOP: MeshObject.BeginMesh(engine::rhal::geometry::LINE_LOOP); break;
        case TINYGLTF_MODE_LINE_STRIP: MeshObject.BeginMesh(engine::rhal::geometry::LINE_STRIP); break;
        case TINYGLTF_MODE_TRIANGLES: MeshObject.BeginMesh(engine::rhal::geometry::TRIANGLES); break;
        case TINYGLTF_MODE_TRIANGLE_STRIP: MeshObject.BeginMesh(engine::rhal::geometry::TRIANGLE_STRIP); break;
        case TINYGLTF_MODE_TRIANGLE_FAN: MeshObject.BeginMesh(engine::rhal::geometry::TRIANGLE_FAN); break;
        default:
            EngineLoggerWarnF("Unsupported glTF Primitive Mode %d", MainPrimitiveType);
        }

        MeshesMaterial.emplace(MeshName, std::vector<engine::Name>());
        auto & MaterialArray = MeshesMaterial[MeshName];
        
        for (const auto & primitive : mesh.primitives)
        {
            AssertOrErrorCall(MainPrimitiveType == primitive.mode, continue;, "Inconsistent primitive mode detected. Unsupported.");
            AssertOrErrorCall(primitive.material >= 0, continue;, "Broken primitive detected.");

            uint32_t vertexCountBefore = MeshObject.GetVertexCount();

            std::vector<uint32_t> indices;
            if(primitive.indices >= 0)
            {
                bool success = TryLoopOverBufferFromAccessor(model, primitive.indices, nullptr,
                    [](const tinygltf::Accessor& Accessor)
                    {
                        switch(Accessor.componentType)
                        {
                        case TINYGLTF_COMPONENT_TYPE_BYTE:
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                        case TINYGLTF_COMPONENT_TYPE_SHORT:
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                        case TINYGLTF_COMPONENT_TYPE_INT:
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                            return true;
                        default:
                            EngineLoggerError("Unsupported index component type for vertex indexation.");
                            return false;
                        }
                    },
                    [&indices](const void* sample, size_t byteStride, const tinygltf::Accessor& Accessor)
                    {
                        switch(Accessor.componentType)
                        {
                        case TINYGLTF_COMPONENT_TYPE_BYTE:
                            indices.push_back(static_cast<uint32_t>( ((int8_t*)sample)[0] ));
                            break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                            indices.push_back(static_cast<uint32_t>( ((uint8_t*)sample)[0] ));
                            break;
                        case TINYGLTF_COMPONENT_TYPE_SHORT:
                            indices.push_back(static_cast<uint32_t>( ((int16_t*)sample)[0] ));
                            break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                            indices.push_back(static_cast<uint32_t>( ((uint16_t*)sample)[0] ));
                            break;
                        case TINYGLTF_COMPONENT_TYPE_INT:
                            indices.push_back(static_cast<uint32_t>( ((int32_t*)sample)[0] ));
                            break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                            indices.push_back( ((uint32_t*)sample)[0] );
                            break;
                        default:
                            UNREACHABLE;
                        }
                    });

                if(!success) continue;
            }
            
            if(primitive.attributes.contains("POSITION"))
            {
                int AccessorIndex = primitive.attributes.at("POSITION");
                bool success = TryLoopOverBufferFromAccessor(model, AccessorIndex,
                    !indices.empty() ? &indices : nullptr,
                    [](const tinygltf::Accessor& Accessor)
                    {
                        AssertOrErrorCall(Accessor.type == TINYGLTF_TYPE_VEC3, return false;, "Unsupported primitive position component type. Position requires a vector 3.")
                        AssertOrErrorCall(Accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT || Accessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE, return false;,
                            "Unsupported primitive position component type. Position vector needs to be made of float or double.")

                        return true;
                    },
                    [&MeshObject](const void* sample, size_t byteStride, const tinygltf::Accessor& Accessor)
                    {
                        switch(Accessor.componentType)
                        {
                        case TINYGLTF_COMPONENT_TYPE_FLOAT :
                            {
                                Assert(byteStride == 3 * sizeof(float))
                                // Interpret data
                                const Math::Point3f* asVec3f = reinterpret_cast<const Math::Point3f*>(sample);
                                MeshObject.AddVertexPosition(*asVec3f);
                            }
                            break;
                            
                        case TINYGLTF_COMPONENT_TYPE_DOUBLE :
                            {
                                Assert(byteStride == 3 * sizeof(double))
                                const Math::Point3d* asVec3d = reinterpret_cast<const Math::Point3d*>(sample);
                                MeshObject.AddVertexPosition(Math::Point3f(
                                   static_cast<double>(asVec3d->x),
                                   static_cast<double>(asVec3d->y),
                                   static_cast<double>(asVec3d->z)
                                   ));
                            }
                            break;
                        default:
                            UNREACHABLE;
                        }
                    });

                if(!success) continue;
            }
            if(primitive.attributes.contains("NORMAL"))
            {
                int AccessorIndex = primitive.attributes.at("NORMAL");
                bool success = TryLoopOverBufferFromAccessor(model, AccessorIndex,
                    !indices.empty() ? &indices : nullptr,
                    [](const tinygltf::Accessor& Accessor)
                    {
                        AssertOrErrorCall(Accessor.type == TINYGLTF_TYPE_VEC3, return false;, 
                            "Unsupported primitive normal component type. Normal requires a vector 3.")
                        AssertOrErrorCall(Accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT || Accessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE, return false;,
                            "Unsupported primitive normal component type. Normal vector needs to be made of float or double.")

                        return true;
                    },
                    [&MeshObject](const void* sample, size_t byteStride, const tinygltf::Accessor& Accessor)
                    {
                        switch(Accessor.componentType)
                        {
                        case TINYGLTF_COMPONENT_TYPE_FLOAT :
                            {
                                Assert(byteStride == 3 * sizeof(float))
                                // Interpret data
                                const Math::Point3f* asVec3f = reinterpret_cast<const Math::Point3f*>(sample);
                                MeshObject.AddVertexNormal(*asVec3f);
                            }
                            break;
                            
                        case TINYGLTF_COMPONENT_TYPE_DOUBLE :
                            {
                                Assert(byteStride == 3 * sizeof(double))
                                const Math::Point3d* asVec3d = reinterpret_cast<const Math::Point3d*>(sample);
                                MeshObject.AddVertexNormal(Math::Vector3f(
                                   static_cast<double>(asVec3d->x),
                                   static_cast<double>(asVec3d->y),
                                   static_cast<double>(asVec3d->z)
                                   ));
                            }
                            break;
                        default:
                            UNREACHABLE;
                        }
                    });

                if(!success) continue;
            }
            if(primitive.attributes.contains("TEXCOORD_0"))
            {
                int AccessorIndex = primitive.attributes.at("TEXCOORD_0");
                bool success = TryLoopOverBufferFromAccessor(model, AccessorIndex,
                    !indices.empty() ? &indices : nullptr,
                    [](const tinygltf::Accessor& Accessor)
                    {
                        AssertOrErrorCall(Accessor.type == TINYGLTF_TYPE_VEC2, return false;, 
                            "Unsupported primitive texture coordinates 0 component type. Texture coordinates 0 requires a vector 2.")
                        AssertOrErrorCall(Accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT || Accessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE, return false;,
                            "Unsupported primitive texture coordinates 0 component type. Texture coordinates 0 needs to be made of float or double.")

                        return true;
                    },
                    [&MeshObject](const void* sample, size_t byteStride, const tinygltf::Accessor& Accessor)
                    {
                        switch(Accessor.componentType)
                        {
                        case TINYGLTF_COMPONENT_TYPE_FLOAT :
                            {
                                Assert(byteStride == 2 * sizeof(float))
                                // Interpret data
                                const Math::Vector2f* asVec3f = reinterpret_cast<const Math::Vector2f*>(sample);
                                MeshObject.AddVertexTextureCoordinate(*asVec3f);
                            }
                            break;
                            
                        case TINYGLTF_COMPONENT_TYPE_DOUBLE :
                            {
                                Assert(byteStride == 2 * sizeof(double))
                                const Math::Vector2d* asVec3d = reinterpret_cast<const Math::Vector2d*>(sample);
                                MeshObject.AddVertexTextureCoordinate(Math::Vector2f(
                                   static_cast<double>(asVec3d->x),
                                   static_cast<double>(asVec3d->y)
                                   ));
                            }
                            break;
                        default:
                            UNREACHABLE;
                        }
                    });

                if(!success) continue;
            }

            // todo vertex color
            
            uint32_t vertexCountAfter = MeshObject.GetVertexCount();
            MeshObject.AddVertexGroup(vertexCountBefore, vertexCountAfter - vertexCountBefore);
            MaterialArray.push_back(scene->m_Materials[primitive.material]);
        }
        
        MeshObject.CommitMesh();
    }


    // Unpack GLTF Scene
    uint32_t sceneIndex = 0;
    AssertOrErrorCall(model.scenes.size() > sceneIndex, return nullptr, "No scene found")

    for (int node : model.scenes[sceneIndex].nodes)
        BuildDrawCallsSceneTree(model, Renderer, MeshesMaterial, node, scene);

    return scene;
}