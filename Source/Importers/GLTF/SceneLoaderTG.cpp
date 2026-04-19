#include "GLTF/SceneLoader.h"

#ifdef USE_TINY_GLTF
#include <tiny_gltf.h>

namespace  GLTF
{
    static bool TryLoopOverBufferFromAccessor(
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

    Transform GetNodeWorldTransform(const tinygltf::Model& model, const tinygltf::Node& NodeObject)
    {
        Transform transform;

        if (!NodeObject.matrix.empty())
        {
            transform.Type = Transform::Matrix;
            for (size_t i = 0; i < NodeObject.matrix.size(); ++i)
            {
                transform.Value.asMatrix[i] = static_cast<float>(NodeObject.matrix[i]);
            }
            
            return transform;
        }

        transform.Type = Transform::Properties;
        transform.Value.asProperties = Math::WorldTransformF{};
        if(!NodeObject.translation.empty())
            transform.Value.asProperties.Position = Math::Point3f(
                static_cast<float>(NodeObject.translation[0]),
                static_cast<float>(NodeObject.translation[1]),
                static_cast<float>(NodeObject.translation[2])
                );

        if(!NodeObject.rotation.empty())
            transform.Value.asProperties.Rotation = Math::QuaternionF(
                static_cast<float>(NodeObject.rotation[0]),
                static_cast<float>(NodeObject.rotation[1]),
                static_cast<float>(NodeObject.rotation[2]),
                static_cast<float>(NodeObject.rotation[3])
                );

        if(!NodeObject.scale.empty())
            transform.Value.asProperties.Scale = Math::Vector3f(
                static_cast<float>(NodeObject.scale[0]),
                static_cast<float>(NodeObject.scale[1]),
                static_cast<float>(NodeObject.scale[2])
                );
        
        return transform;
    }

    void LoadTextures(const tinygltf::Model& model, CPUScene& scene)
    {
        for (const auto & texture : model.textures)
        {            
            const auto & image = model.images[texture.source];
            AssertOrErrorCall(!image.as_is, continue;, "Custom image file type are not supported")

            Image::Layout Layout;
            switch(image.component)
            {
            case 1:
                Layout = Image::R;
                break;
            case 2: 
                Layout = Image::RG;
                break;
            case 3: 
                Layout = Image::RGB;
                break;
            case 4: 
                Layout = Image::RGBA;
                break;
                    
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGEF("Unsupported image %s, unsupported component count (%d)", texture.name.c_str(), image.component)
            }
            
            Image::Type Type;
            switch(image.pixel_type)
            {
                case TINYGLTF_COMPONENT_TYPE_BYTE: 
                    Type = Image::Byte;
                    break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:                     
                    Type = Image::UnsignedByte;
                    break;
                case TINYGLTF_COMPONENT_TYPE_SHORT: 
                    Type = Image::Short;
                    break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: 
                    Type = Image::UnsignedShort;
                    break;
                case TINYGLTF_COMPONENT_TYPE_INT: 
                    Type = Image::Int;
                    break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: 
                    Type = Image::UnsignedInt;
                    break;
                case TINYGLTF_COMPONENT_TYPE_FLOAT: 
                    Type = Image::Float;
                    break;
                case TINYGLTF_COMPONENT_TYPE_DOUBLE: 
                    Type = Image::Double;
                    break;
                    
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGEF("Unsupported image %s, unsupported data type (%d)", texture.name.c_str(), image.pixel_type)
            }
            
            scene.textures.emplace_back(image.width, image.height, Type, Layout, image.image.data());
        }
    }

    void LoadMaterials(const tinygltf::Model& model, CPUScene& scene)
    {
        for (const auto & material : model.materials)
        {
            scene.materials.emplace_back();
            Material& MaterialObject = *(scene.materials.end() - 1);

            // Unpack color
            MaterialObject.color = Math::Vector4f(
                static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[0]),
                static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[1]),
                static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[2]),
                1.0f
            );
            if(material.pbrMetallicRoughness.baseColorTexture.index >= 0)
            {
                MaterialObject.colorTexture = material.pbrMetallicRoughness.baseColorTexture.index;
            }
            MaterialObject.emissive = Math::Vector4f(
                static_cast<float>(material.emissiveFactor[0]),
                static_cast<float>(material.emissiveFactor[1]),
                static_cast<float>(material.emissiveFactor[2]),
                1.0f
            );
            if(material.emissiveTexture.index >= 0)
            {
                MaterialObject.emissiveTexture = material.emissiveTexture.index;
            }

            // Unpack reflection
            MaterialObject.roughness = static_cast<float>(material.pbrMetallicRoughness.roughnessFactor);
            MaterialObject.metallic = static_cast<float>(material.pbrMetallicRoughness.metallicFactor);
            if(material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0)
            {
                MaterialObject.metallicRoughnessTexture = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
            }
             
            // Unpack AO
            if(material.occlusionTexture.index >= 0)
            {
                MaterialObject.occlusionTexture = material.occlusionTexture.index;
            }

            // Get normal map
            if(material.normalTexture.index >= 0)
            {
                MaterialObject.normalTexture = material.normalTexture.index;
            }

            MaterialObject.twoSided = material.doubleSided;

            //KHR_materials_ior
            if(material.extensions.contains("KHR_materials_ior"))
            {
                auto& extension = material.extensions.at("KHR_materials_ior");

                if(extension.Has("ior"))
                    MaterialObject.ior = static_cast<float>(extension.Get("ior").GetNumberAsDouble());
            }

             //KHR_materials_specular
             if(material.extensions.contains("KHR_materials_specular"))
             {
                 auto& extension = material.extensions.at("KHR_materials_specular");

                 if(extension.Has("specularFactor"))
                    MaterialObject.specular = static_cast<float>(extension.Get("specularFactor").GetNumberAsDouble());

                 if(extension.Has("specularTexture"))
                 {
                     int32_t textureIndex = extension.Get("specularTexture").Get("index").GetNumberAsInt();
                     if(textureIndex >= 0)
                     {
                         MaterialObject.specularTexture = textureIndex;
                     }
                 }
                 
                 if(extension.Has("specularColorFactor"))
                 {
                     MaterialObject.specularColor.x = static_cast<float>(extension.Get("specularColorFactor").Get(0).GetNumberAsDouble());
                     MaterialObject.specularColor.x = static_cast<float>(extension.Get("specularColorFactor").Get(1).GetNumberAsDouble());
                     MaterialObject.specularColor.x = static_cast<float>(extension.Get("specularColorFactor").Get(2).GetNumberAsDouble());
                     MaterialObject.specularColor.w = 1.0f;
                 }

                 if(extension.Has("specularColorTexture"))
                 {
                     int32_t textureIndex = extension.Get("specularColorTexture").Get("index").GetNumberAsInt();
                     if(textureIndex >= 0)
                     {
                         MaterialObject.specularColorTexture = textureIndex;
                     }
                 }
             }

             //KHR_materials_transmission
             if(material.extensions.contains("KHR_materials_transmission"))
             {
                 auto& extension = material.extensions.at("KHR_materials_transmission");

                 if(extension.Has("transmissionFactor"))
                    MaterialObject.transmission = static_cast<float>(extension.Get("transmissionFactor").GetNumberAsDouble());

                 if(extension.Has("transmissionTexture"))
                 {
                     int32_t textureIndex = extension.Get("transmissionTexture").Get("index").GetNumberAsInt();
                     if(textureIndex >= 0)
                     {
                         MaterialObject.transmissionTexture = textureIndex;
                     }
                 }
             }
        }
    }

    void LoadSceneTree(
        const tinygltf::Model& model,
        int node,
        CPUScene& outScene, const Transform& parent = Transform{}
        )
    {
        const auto& NodeObject = model.nodes[node];

        size_t transformIndex = outScene.transforms.size();
        outScene.transforms.emplace_back();
        Transform& TransformObject = outScene.transforms.back();
        
        TransformObject = GetNodeWorldTransform(model, NodeObject);
        
        // Parent transform
        switch (TransformObject.Type)
        {
        case Transform::Properties:
            switch (parent.Type)
            {
            case Transform::Properties:
                TransformObject.Value.asProperties = parent.Value.asProperties * TransformObject.Value.asProperties;
                break;
                    
            case Transform::Matrix:
                TransformObject.Value.asMatrix = parent.Value.asMatrix * TransformObject.Value.asProperties.GetTransform();
                TransformObject.Type = Transform::Matrix;
                break;
            
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
            }
            break;
            
        case Transform::Matrix:
            switch (parent.Type)
            {
            case Transform::Properties:
                TransformObject.Value.asMatrix = parent.Value.asProperties.GetTransform() * TransformObject.Value.asMatrix;
                break;
                    
            case Transform::Matrix:
                TransformObject.Value.asMatrix = parent.Value.asMatrix * TransformObject.Value.asMatrix;
                break;
                
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
            }
        break;
        
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
        }
        
        Transform NewParentTransform = TransformObject;
        
        // Recursive call
        for (int child : NodeObject.children)
            LoadSceneTree(model, child, outScene, NewParentTransform);
        
        // Skip unsupported scene elements
        if(NodeObject.mesh == -1) return;

        uint8_t vertexGroupIndex = 0;
        for (const auto & primitive : model.meshes[NodeObject.mesh].primitives)
        {
            MeshInstance inst;
            inst.mesh = NodeObject.mesh;
            inst.material = primitive.material;
            inst.transform = transformIndex;
            inst.vertexGroup = vertexGroupIndex++;

            outScene.instances.emplace_back(inst);
        }
    }
    
    bool GLTF::LoadCPUScene(const std::filesystem::path& path, CPUScene& scene)
    {
        tinygltf::TinyGLTF loader;

        AssertOrErrorCallF(exists(path), return false, "No such file or directory \"%s\"", path.generic_string().c_str())
        if(!exists(path)) return false;

        tinygltf::Model model;
        std::string ErrorBuffer;
        std::string WarningBuffer;
        bool status;

        if (path.extension() == "glb")
        {
            status = loader.LoadBinaryFromFile(&model, &ErrorBuffer, &WarningBuffer, path.string().c_str());
        }
        else if (path.extension() == "gltf")
        {
            status = loader.LoadASCIIFromFile(&model, &ErrorBuffer, &WarningBuffer, path.string().c_str());
        }
        else
        {
            EngineLoggerErrorF("Unsupported file extension (%s) for gltf file", path.extension().string().c_str());
            return false;
        }

        // Handle loading errors
        AssertOrErrorCallF(status && ErrorBuffer.empty(), return false, "Failed to load scene %s. Loader returned the following error(s) : %s", path.string().c_str(), ErrorBuffer.c_str());
        AssertOrWarnCallF(WarningBuffer.empty(), , "Loaded scene %s with the following warning(s) : %s", path.string().c_str(), WarningBuffer.c_str());

        // Reserve memory
        scene.textures.reserve(!model.textures.empty() ? model.textures.size() : 0);
        scene.materials.reserve(!model.materials.empty() ? model.materials.size() : 0);
        scene.meshes.reserve(!model.meshes.empty() ? model.meshes.size() : 0);

        // Load textures
        LoadTextures(model, scene);
        
        // Load materials
        LoadMaterials(model, scene);

        // Load meshes
        for (const auto & mesh : model.meshes)
        {
            scene.meshes.emplace_back();
            Mesh& MeshObject = *(scene.meshes.end() - 1);

            const int MainPrimitiveType = mesh.primitives[0].mode;
            switch (MainPrimitiveType)
            {
            case TINYGLTF_MODE_POINTS:          MeshObject.BeginMesh(Mesh::VertexType::POINTS); break;
            case TINYGLTF_MODE_LINE:            MeshObject.BeginMesh(Mesh::VertexType::LINES); break;
            case TINYGLTF_MODE_LINE_LOOP:       MeshObject.BeginMesh(Mesh::VertexType::LINE_LOOP); break;
            case TINYGLTF_MODE_LINE_STRIP:      MeshObject.BeginMesh(Mesh::VertexType::LINE_STRIP); break;
            case TINYGLTF_MODE_TRIANGLES:       MeshObject.BeginMesh(Mesh::VertexType::TRIANGLES); break;
            case TINYGLTF_MODE_TRIANGLE_STRIP:  MeshObject.BeginMesh(Mesh::VertexType::TRIANGLE_STRIP); break;
            case TINYGLTF_MODE_TRIANGLE_FAN:    MeshObject.BeginMesh(Mesh::VertexType::TRIANGLE_FAN); break;
#ifdef CONFIG_DEBUG
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGEF("Unsupported glTF Primitive Mode %d", MainPrimitiveType)
#else // CONFIG_DEBUG
            default:
                EngineLoggerErrorF("Unsupported glTF Primitive Mode %d", MainPrimitiveType);
#endif // !CONFIG_DEBUG
            }
            
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
                                       static_cast<float>(asVec3d->x),
                                       static_cast<float>(asVec3d->y),
                                       static_cast<float>(asVec3d->z)
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
                                       static_cast<float>(asVec3d->x),
                                       static_cast<float>(asVec3d->y),
                                       static_cast<float>(asVec3d->z)
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
                                       static_cast<float>(asVec3d->x),
                                       static_cast<float>(asVec3d->y)
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
            }
            
            MeshObject.CommitMesh();
        }

        // Load Scene Tree
        uint32_t sceneIndex = 0;
        AssertOrErrorCall(model.scenes.size() > sceneIndex, return false, "No scene found")

        for (int node : model.scenes[sceneIndex].nodes)
            LoadSceneTree(model, node, scene);
        
    }
    
    void LoadSceneTree(
        const tinygltf::Model& model,
        int node,
        GPUScene& outScene, const Transform& parent = Transform()
        )
    {
        const auto& NodeObject = model.nodes[node];

        size_t transformIndex = outScene.transforms.size();
        outScene.transforms.emplace_back();
        Transform& TransformObject = outScene.transforms.back();
        
        TransformObject = GetNodeWorldTransform(model, NodeObject);
        
        // Parent transform
        switch (TransformObject.Type)
        {
        case Transform::Properties:
            switch (parent.Type)
            {
            case Transform::Properties:
                TransformObject.Value.asProperties = parent.Value.asProperties * TransformObject.Value.asProperties;
                break;
                
            case Transform::Matrix:
                TransformObject.Value.asMatrix = parent.Value.asMatrix * TransformObject.Value.asProperties.GetTransform();
                TransformObject.Type = Transform::Matrix;
                break;
            
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
            }
            break;
            
        case Transform::Matrix:
            switch (parent.Type)
            {
            case Transform::Properties:
                TransformObject.Value.asMatrix = parent.Value.asProperties.GetTransform() * TransformObject.Value.asMatrix;
                break;
                    
            case Transform::Matrix:
                TransformObject.Value.asMatrix = parent.Value.asMatrix * TransformObject.Value.asMatrix;
                break;
                
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
            }
            break;
            
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
        }
        
        Transform NewParentTransform = TransformObject;
        
        // Recursive call
        for (int child : NodeObject.children)
            LoadSceneTree(model, child, outScene, NewParentTransform);
        
        // Skip unsupported scene elements
        if(NodeObject.mesh == -1) return;

        uint8_t vertexGroupIndex = 0;
        for (const auto & primitive : model.meshes[NodeObject.mesh].primitives)
        {
            MeshInstance inst;
            inst.mesh = NodeObject.mesh;
            inst.material = primitive.material;
            inst.transform = transformIndex;
            inst.vertexGroup = vertexGroupIndex++;

            outScene.instances.emplace_back(inst);
        }
    }
    
    void LoadTextures(const tinygltf::Model& model, GPUScene& scene)
    {
        for (const auto & texture : model.textures)
        {            
            const auto & image = model.images[texture.source];
            AssertOrErrorCall(!image.as_is, continue;, "Custom image file type are not supported")

            Image::Layout Layout;
            switch(image.component)
            {
            case 1:
                Layout = Image::R;
                break;
            case 2: 
                Layout = Image::RG;
                break;
            case 3: 
                Layout = Image::RGB;
                break;
            case 4: 
                Layout = Image::RGBA;
                break;
                    
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGEF("Unsupported image %s, unsupported component count (%d)", texture.name.c_str(), image.component)
            }
            
            Image::Type Type;
            switch(image.pixel_type)
            {
                case TINYGLTF_COMPONENT_TYPE_BYTE: 
                    Type = Image::Byte;
                    break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:                     
                    Type = Image::UnsignedByte;
                    break;
                case TINYGLTF_COMPONENT_TYPE_SHORT: 
                    Type = Image::Short;
                    break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: 
                    Type = Image::UnsignedShort;
                    break;
                case TINYGLTF_COMPONENT_TYPE_INT: 
                    Type = Image::Int;
                    break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: 
                    Type = Image::UnsignedInt;
                    break;
                case TINYGLTF_COMPONENT_TYPE_FLOAT: 
                    Type = Image::Float;
                    break;
                case TINYGLTF_COMPONENT_TYPE_DOUBLE: 
                    Type = Image::Double;
                    break;
                    
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGEF("Unsupported image %s, unsupported data type (%d)", texture.name.c_str(), image.pixel_type)
            }
            
            scene.textures.emplace_back(image.width, image.height, Type, Layout, image.image.data(), image.image.size());
        }
    }

    void LoadMaterials(const tinygltf::Model& model, GPUScene& scene)
    {
        for (const auto & material : model.materials)
        {
            scene.materials.emplace_back();
            Material& MaterialObject = *(scene.materials.end() - 1);

            // Unpack color
            MaterialObject.color = Math::Vector4f(
                static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[0]),
                static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[1]),
                static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[2]),
                1.0f
            );
            if(material.pbrMetallicRoughness.baseColorTexture.index >= 0)
            {
                MaterialObject.colorTexture = material.pbrMetallicRoughness.baseColorTexture.index;
            }
            MaterialObject.emissive = Math::Vector4f(
                static_cast<float>(material.emissiveFactor[0]),
                static_cast<float>(material.emissiveFactor[1]),
                static_cast<float>(material.emissiveFactor[2]),
                1.0f
            );
            if(material.emissiveTexture.index >= 0)
            {
                MaterialObject.emissiveTexture = material.emissiveTexture.index;
            }

            // Unpack reflection
            MaterialObject.roughness = static_cast<float>(material.pbrMetallicRoughness.roughnessFactor);
            MaterialObject.metallic = static_cast<float>(material.pbrMetallicRoughness.metallicFactor);
            if(material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0)
            {
                MaterialObject.metallicRoughnessTexture = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
            }
             
            // Unpack AO
            if(material.occlusionTexture.index >= 0)
            {
                MaterialObject.occlusionTexture = material.occlusionTexture.index;
            }

            // Get normal map
            if(material.normalTexture.index >= 0)
            {
                MaterialObject.normalTexture = material.normalTexture.index;
            }

            MaterialObject.twoSided = material.doubleSided;

            //KHR_materials_ior
            if(material.extensions.contains("KHR_materials_ior"))
            {
                auto& extension = material.extensions.at("KHR_materials_ior");

                if(extension.Has("ior"))
                    MaterialObject.ior = static_cast<float>(extension.Get("ior").GetNumberAsDouble());
            }

             //KHR_materials_specular
             if(material.extensions.contains("KHR_materials_specular"))
             {
                 auto& extension = material.extensions.at("KHR_materials_specular");

                 if(extension.Has("specularFactor"))
                    MaterialObject.specular = static_cast<float>(extension.Get("specularFactor").GetNumberAsDouble());

                 if(extension.Has("specularTexture"))
                 {
                     int32_t textureIndex = extension.Get("specularTexture").Get("index").GetNumberAsInt();
                     if(textureIndex >= 0)
                     {
                         MaterialObject.specularTexture = textureIndex;
                     }
                 }
                 
                 if(extension.Has("specularColorFactor"))
                 {
                     MaterialObject.specularColor.x = static_cast<float>(extension.Get("specularColorFactor").Get(0).GetNumberAsDouble());
                     MaterialObject.specularColor.x = static_cast<float>(extension.Get("specularColorFactor").Get(1).GetNumberAsDouble());
                     MaterialObject.specularColor.x = static_cast<float>(extension.Get("specularColorFactor").Get(2).GetNumberAsDouble());
                     MaterialObject.specularColor.w = 1.0f;
                 }

                 if(extension.Has("specularColorTexture"))
                 {
                     int32_t textureIndex = extension.Get("specularColorTexture").Get("index").GetNumberAsInt();
                     if(textureIndex >= 0)
                     {
                         MaterialObject.specularColorTexture = textureIndex;
                     }
                 }
             }

             //KHR_materials_transmission
             if(material.extensions.contains("KHR_materials_transmission"))
             {
                 auto& extension = material.extensions.at("KHR_materials_transmission");

                 if(extension.Has("transmissionFactor"))
                    MaterialObject.transmission = static_cast<float>(extension.Get("transmissionFactor").GetNumberAsDouble());

                 if(extension.Has("transmissionTexture"))
                 {
                     int32_t textureIndex = extension.Get("transmissionTexture").Get("index").GetNumberAsInt();
                     if(textureIndex >= 0)
                     {
                         MaterialObject.transmissionTexture = textureIndex;
                     }
                 }
             }
        }
    }

    static bool TryLoopOverBufferFromAccessor(
    const tinygltf::Model& model, uint32_t AccessorIndex,
    const std::function<bool(const tinygltf::Accessor& Accessor)>& execCheckType,
    std::span<const uint8_t>& OutView, size_t& OutElementCount)
    {
        const auto& accessor = model.accessors[AccessorIndex];
        if(!execCheckType(accessor)) return false;
        
        const auto& bufferView = model.bufferViews[accessor.bufferView];
        const auto& buffer = model.buffers[bufferView.buffer];

        const uint8_t* rawBuffer = buffer.data.data() + bufferView.byteOffset;
        const size_t rawBufferSize = bufferView.byteLength;
        const size_t byteStride = tinygltf::GetComponentSizeInBytes(accessor.componentType) * tinygltf::GetNumComponentsInType(accessor.type);
        
        OutView = std::span<const uint8_t>(rawBuffer, rawBuffer + rawBufferSize);
        OutElementCount = rawBufferSize / byteStride;

        return true;
    }
    
    void FindBoundsInPositionBuffer(std::span<const uint8_t> Buffer, int GLTFComponentType, Math::Point3f& Min, Math::Point3f& Max)
    {
        const size_t byteStride = tinygltf::GetComponentSizeInBytes(GLTFComponentType) * tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
        const size_t ElementCount = Buffer.size() / byteStride;
        
        switch (GLTFComponentType)
        {
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            for (size_t i = 0; i < ElementCount; i++)
            {
                const uint8_t* Sample = &Buffer[i * byteStride];
                
                const Math::Point3f* AsPoint3 = reinterpret_cast<const Math::Point3f*>(Sample);
                Min.x = Min.x < AsPoint3->x ? Min.x : AsPoint3->x;
                Min.y = Min.y < AsPoint3->y ? Min.y : AsPoint3->y;
                Min.z = Min.z < AsPoint3->z ? Min.z : AsPoint3->z;
                Max.x = Max.x > AsPoint3->x ? Max.x : AsPoint3->x;
                Max.y = Max.y > AsPoint3->y ? Max.y : AsPoint3->y;
                Max.z = Max.z > AsPoint3->z ? Max.z : AsPoint3->z;
            }
            break;
            
        case TINYGLTF_COMPONENT_TYPE_DOUBLE:
            for (size_t i = 0; i < ElementCount; i++)
            {
                const uint8_t* Sample = &Buffer[i * byteStride];
                
                const Math::Point3d* AsPoint3 = reinterpret_cast<const Math::Point3d*>(Sample);
                Min.x = Min.x < AsPoint3->x ? Min.x : AsPoint3->x;
                Min.y = Min.y < AsPoint3->y ? Min.y : AsPoint3->y;
                Min.z = Min.z < AsPoint3->z ? Min.z : AsPoint3->z;
                Max.x = Max.x > AsPoint3->x ? Max.x : AsPoint3->x;
                Max.y = Max.y > AsPoint3->y ? Max.y : AsPoint3->y;
                Max.z = Max.z > AsPoint3->z ? Max.z : AsPoint3->z;
            }
            break;
                
#ifdef CONFIG_DEBUG
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGEF("Unsupported glTF Primitive Mode %d", GLTFComponentType)
#else // CONFIG_DEBUG
        default:
            EngineLoggerErrorF("Unsupported glTF Primitive Mode %d", GLTFComponentType);
#endif // !CONFIG_DEBUG
        }
    }
    
    bool GLTF::LoadGPUScene(const std::filesystem::path& path, GPUScene& scene)
    {
        tinygltf::TinyGLTF loader;

        AssertOrErrorCallF(exists(path), return false, "No such file or directory \"%s\"", path.generic_string().c_str())
        if(!exists(path)) return false;

        tinygltf::Model model;
        std::string ErrorBuffer;
        std::string WarningBuffer;
        bool status;

        if (path.extension().compare(".glb") == 0)
        {
            status = loader.LoadBinaryFromFile(&model, &ErrorBuffer, &WarningBuffer, path.string().c_str());
        }
        else if (path.extension().compare(".gltf") == 0)
        {
            status = loader.LoadASCIIFromFile(&model, &ErrorBuffer, &WarningBuffer, path.string().c_str());
        }
        else
        {
            EngineLoggerErrorF("Unsupported file extension (%s) for gltf file", path.extension().string().c_str());
            return false;
        }

        // Handle loading errors
        AssertOrErrorCallF(status && ErrorBuffer.empty(), return false, "Failed to load scene %s. Loader returned the following error(s) : %s", path.string().c_str(), ErrorBuffer.c_str());
        AssertOrWarnCallF(WarningBuffer.empty(), , "Loaded scene %s with the following warning(s) : %s", path.string().c_str(), WarningBuffer.c_str());

        // Reserve memory
        scene.textures.reserve(!model.textures.empty() ? model.textures.size() : 0);
        scene.materials.reserve(!model.materials.empty() ? model.materials.size() : 0);
        scene.meshes.reserve(!model.meshes.empty() ? model.meshes.size() : 0);

        // Load textures
        LoadTextures(model, scene);
        
        // Load materials
        LoadMaterials(model, scene);

        // Load meshes
        for (const auto & mesh : model.meshes)
        {
            scene.meshes.emplace_back();
            MeshObject& MeshObject = scene.meshes.back();
            
            const int MainPrimitiveType = mesh.primitives[0].mode;
            switch (MainPrimitiveType)
            {
            case TINYGLTF_MODE_POINTS:          MeshObject.BeginMesh(Mesh::VertexType::POINTS); break;
            case TINYGLTF_MODE_LINE:            MeshObject.BeginMesh(Mesh::VertexType::LINES); break;
            case TINYGLTF_MODE_LINE_LOOP:       MeshObject.BeginMesh(Mesh::VertexType::LINE_LOOP); break;
            case TINYGLTF_MODE_LINE_STRIP:      MeshObject.BeginMesh(Mesh::VertexType::LINE_STRIP); break;
            case TINYGLTF_MODE_TRIANGLES:       MeshObject.BeginMesh(Mesh::VertexType::TRIANGLES); break;
            case TINYGLTF_MODE_TRIANGLE_STRIP:  MeshObject.BeginMesh(Mesh::VertexType::TRIANGLE_STRIP); break;
            case TINYGLTF_MODE_TRIANGLE_FAN:    MeshObject.BeginMesh(Mesh::VertexType::TRIANGLE_FAN); break;
                
#ifdef CONFIG_DEBUG
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGEF("Unsupported glTF Primitive Mode %d", MainPrimitiveType)
#else // CONFIG_DEBUG
            default:
                EngineLoggerErrorF("Unsupported glTF Primitive Mode %d", MainPrimitiveType);
#endif // !CONFIG_DEBUG
            }
            
            struct BufferDataView
            {
                int TinyGLTFComponentType;
                std::span<const uint8_t> View;
            };
            
            // states
            std::vector<BufferDataView> Indexes;
            std::vector<BufferDataView> Positions;
            std::vector<BufferDataView> Normals;
            std::vector<BufferDataView> TextCoords;
            size_t PreviousVertexCount = 0;
            
            for (const auto & primitive : mesh.primitives)
            {
                AssertOrErrorCall(MainPrimitiveType == primitive.mode, continue;, "Inconsistent primitive mode detected. Unsupported.")
                AssertOrErrorCall(primitive.material >= 0, continue;, "Broken primitive detected.")

                bool IsIndexed = primitive.indices >= 0;
                std::span<const uint8_t> CurrentView{};
                size_t CurrentVertexCount = 0, Dummy = 0; 
                
                // Bounds
                Math::Point3f Min = {FLT_MAX}, Max = {FLT_MIN};

                if(IsIndexed)
                {
                    bool success = TryLoopOverBufferFromAccessor(model, primitive.indices,
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
                        CurrentView, CurrentVertexCount);

                    if(!success) continue;
                    
                    AssertOrErrorCallF(Indexes.empty() || Indexes.front().TinyGLTFComponentType ==  model.accessors[primitive.indices].componentType, continue;, "Primitive index buffer type missmatch")
                    
                    Indexes.push_back(BufferDataView{.TinyGLTFComponentType = model.accessors[primitive.indices].componentType, .View = CurrentView});
                }
                
                if(primitive.attributes.contains("POSITION"))
                {
                    int AccessorIndex = primitive.attributes.at("POSITION");
                    bool success = TryLoopOverBufferFromAccessor(model, AccessorIndex,
                        [](const tinygltf::Accessor& Accessor)
                        {
                            AssertOrErrorCall(Accessor.type == TINYGLTF_TYPE_VEC3, return false;, "Unsupported primitive position component type. Position requires a vector 3.")
                            AssertOrErrorCall(Accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT || Accessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE, return false;,
                                "Unsupported primitive position component type. Position vector needs to be made of float or double.")

                            return true;
                        },
                        CurrentView, IsIndexed ? Dummy : CurrentVertexCount);

                    if(!success) continue;
                    
                    AssertOrErrorCallF(Positions.empty() || Positions.front().TinyGLTFComponentType ==  model.accessors[AccessorIndex].componentType, continue;, "Primitive buffer type missmatch")
                    
                    Positions.push_back(BufferDataView{.TinyGLTFComponentType = model.accessors[AccessorIndex].componentType, .View = CurrentView});
                    
                    FindBoundsInPositionBuffer(CurrentView, model.accessors[AccessorIndex].componentType, Min, Max);
                }
                if(primitive.attributes.contains("NORMAL"))
                {
                    int AccessorIndex = primitive.attributes.at("NORMAL");
                    bool success = TryLoopOverBufferFromAccessor(model, AccessorIndex,
                        [](const tinygltf::Accessor& Accessor)
                        {
                            AssertOrErrorCall(Accessor.type == TINYGLTF_TYPE_VEC3, return false;, 
                                "Unsupported primitive normal component type. Normal requires a vector 3.")
                            AssertOrErrorCall(Accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT || Accessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE, return false;,
                                "Unsupported primitive normal component type. Normal vector needs to be made of float or double.")

                            return true;
                        },
                        CurrentView, IsIndexed ? Dummy : CurrentVertexCount);

                    if(!success) continue;
                    
                    AssertOrErrorCallF(Normals.empty() || Normals.front().TinyGLTFComponentType ==  model.accessors[AccessorIndex].componentType, continue;, "Primitive buffer type missmatch")
                    
                    Normals.push_back(BufferDataView{.TinyGLTFComponentType = model.accessors[AccessorIndex].componentType, .View = CurrentView});
                }
                if(primitive.attributes.contains("TEXCOORD_0"))
                {
                    int AccessorIndex = primitive.attributes.at("TEXCOORD_0");
                    bool success = TryLoopOverBufferFromAccessor(model, AccessorIndex,
                        [](const tinygltf::Accessor& Accessor)
                        {
                            AssertOrErrorCall(Accessor.type == TINYGLTF_TYPE_VEC2, return false;, 
                                "Unsupported primitive texture coordinates 0 component type. Texture coordinates 0 requires a vector 2.")
                            AssertOrErrorCall(Accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT || Accessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE, return false;,
                                "Unsupported primitive texture coordinates 0 component type. Texture coordinates 0 needs to be made of float or double.")

                            return true;
                        },
                        CurrentView, IsIndexed ? Dummy : CurrentVertexCount);

                    if(!success) continue;
                    
                    AssertOrErrorCallF(TextCoords.empty() || TextCoords.front().TinyGLTFComponentType ==  model.accessors[AccessorIndex].componentType, continue;, "Primitive buffer type missmatch")
                    
                    TextCoords.push_back(BufferDataView{.TinyGLTFComponentType = model.accessors[AccessorIndex].componentType, .View = CurrentView});
                }
                
                // todo vertex color
                MeshObject.AddVertexGroup({(unsigned int)PreviousVertexCount, (unsigned int)CurrentVertexCount - (unsigned int)PreviousVertexCount, Min, Max});
            }
            
            if (!Indexes.empty())
            {
                IndexBuffer::IndexType Type;
                switch(Indexes.front().TinyGLTFComponentType)
                {
                case TINYGLTF_COMPONENT_TYPE_BYTE:          Type = IndexBuffer::Byte; break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: Type = IndexBuffer::UnsignedByte; break;
                case TINYGLTF_COMPONENT_TYPE_SHORT:         Type = IndexBuffer::Short; break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:Type = IndexBuffer::UnsignedShort; break;
                case TINYGLTF_COMPONENT_TYPE_INT:           Type = IndexBuffer::Int; break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:  Type = IndexBuffer::UnsignedInt; break;
                        
                default:
                    UNREACHABLE;
                }
                
                size_t Size = 0;
                
                for (const auto& view : Indexes)
                {
                    Size += view.View.size();
                }
                
                MeshObject.SetIndexBuffer(Type, (Size) / ToGLIndexSize(Type));
                
                size_t Offset = 0;
                for (const auto& view : Indexes)
                {
                    MeshObject.SetIndexSubBuffer(view.View.data(), Offset, view.View.size());
                    Offset += view.View.size();
                }
            }
            if (!Positions.empty())
            {
                VertexArrayObject::Layout Layout{}; 
                switch (Positions.front().TinyGLTFComponentType)
                {
                case TINYGLTF_COMPONENT_TYPE_FLOAT: Layout.Push<Math::Point3f>(1); break;
                case TINYGLTF_COMPONENT_TYPE_DOUBLE:  Layout.Push<Math::Point3d>(1); break;
                
#ifdef CONFIG_DEBUG
                SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGEF("Unsupported glTF Primitive Mode %d", Positions.front().TinyGLTFComponentType)
#else // CONFIG_DEBUG
                default:
                    EngineLoggerErrorF("Unsupported glTF Primitive Mode %d", Positions.front().TinyGLTFComponentType);
#endif // !CONFIG_DEBUG
                }
                
                size_t Size = 0;
                
                for (const auto& view : Positions)
                {
                    Size += view.View.size();
                }
                
                size_t VBO = MeshObject.AddVertexBuffer(Layout, Size);
                
                size_t Offset = 0;
                for (const auto& view : Positions)
                {
                    MeshObject.SetVertexSubBuffer(VBO, view.View.data(), Offset, view.View.size());
                    Offset += view.View.size();
                }
            }
            if (!Normals.empty())
            {
                VertexArrayObject::Layout Layout{}; 
                switch (Normals.front().TinyGLTFComponentType)
                {
                case TINYGLTF_COMPONENT_TYPE_FLOAT: Layout.Push<Math::Vector3f>(1); break;
                case TINYGLTF_COMPONENT_TYPE_DOUBLE:  Layout.Push<Math::Vector3d>(1); break;
                
#ifdef CONFIG_DEBUG
                SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGEF("Unsupported glTF Primitive Mode %d", Normals.front().TinyGLTFComponentType)
#else // CONFIG_DEBUG
                default:
                    EngineLoggerErrorF("Unsupported glTF Primitive Mode %d", Normals.front().TinyGLTFComponentType);
#endif // !CONFIG_DEBUG
                }
                
                size_t Size = 0;
                
                for (const auto& view : Normals)
                {
                    Size += view.View.size();
                }
                
                size_t VBO = MeshObject.AddVertexBuffer(Layout, Size);
                
                size_t Offset = 0;
                for (const auto& view : Normals)
                {
                    MeshObject.SetVertexSubBuffer(VBO, view.View.data(), Offset, view.View.size());
                    Offset += view.View.size();
                }
            }
            if (!TextCoords.empty())
            {
                VertexArrayObject::Layout Layout{}; 
                switch (TextCoords.front().TinyGLTFComponentType)
                {
                case TINYGLTF_COMPONENT_TYPE_FLOAT: Layout.Push<Math::Vector2f>(1); break;
                case TINYGLTF_COMPONENT_TYPE_DOUBLE:  Layout.Push<Math::Vector2d>(1); break;
                
#ifdef CONFIG_DEBUG
                SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGEF("Unsupported glTF Primitive Mode %d", TextCoords.front().TinyGLTFComponentType)
#else // CONFIG_DEBUG
                default:
                    EngineLoggerErrorF("Unsupported glTF Primitive Mode %d", TextCoords.front().TinyGLTFComponentType);
#endif // !CONFIG_DEBUG
                }
                
                size_t Size = 0;
                
                for (const auto& view : TextCoords)
                {
                    Size += view.View.size();
                }
                
                size_t VBO = MeshObject.AddVertexBuffer(Layout, Size);
                
                size_t Offset = 0;
                for (const auto& view : TextCoords)
                {
                    MeshObject.SetVertexSubBuffer(VBO, view.View.data(), Offset, view.View.size());
                    Offset += view.View.size();
                }
            }            
        
            MeshObject.EndMesh();
        }

        // Load Scene Tree
        uint32_t sceneIndex = 0;
        AssertOrErrorCall(model.scenes.size() > sceneIndex, return false, "No scene found")

        for (int node : model.scenes[sceneIndex].nodes)
            LoadSceneTree(model, node, scene);
        
        return true;
    }

    
}
#endif // USE_TINY_GLTF