#include "GLTF/SceneLoader.h"

#ifdef USE_TINY_GLTF_3


#include <functional>
#include <tiny_gltf_v3.h>

// safe version of strcmp comparing a string view tyoe object and a literal string
#define SAFE_STRCMP_S_LS(ObjectStr, ObjectStrLen, LiteralString) strncmp(ObjectStr, LiteralString, std::min((size_t)(ObjectStrLen), strlen(LiteralString)))
#define SAFE_STRCMP_S_S(ObjectStrA, ObjectStrLenA, ObjectStrB, ObjectStrLenB) strncmp(ObjectStrA, ObjectStrB, std::min(ObjectStrLenA, ObjectStrLenB))


namespace GLTF
{
    struct ImageLoadFreeState
    {
        
    };
    
    static int32_t ImageLoadCallback(tg3_image_result *result, const tg3_image_request *request, void *user_data)
    {
        
    }

    static void ImageReleaseCallback(uint8_t *pixels, void *user_data)
    {
        
    }

    static const tg3_extension* FindExtension(const tg3_extras_ext& extentions, std::string_view extension)
    {
        for (size_t i = 0; i < extentions.extensions_count; ++i)
        {
            if (SAFE_STRCMP_S_S(
                extentions.extensions[i].name.data,
                (size_t)(extentions.extensions[i].name.len),
                extension.data(),
                extension.size()) == 0
                )
                return extentions.extensions + i;
        }

        return nullptr;
    }

    static const tg3_value* FindValueInObject(const tg3_value& object, std::string_view field)
    {
        if (object.type != TG3_VALUE_OBJECT) return nullptr;
        
        for (size_t i = 0; i < object.object_count; ++i)
        {
            if (SAFE_STRCMP_S_S(
                object.object_data[i].key.data,
                (size_t)(object.object_data[i].key.len),
                field.data(),
                field.size()) == 0
                )
                return &(object.object_data[i].value);
        }

        return nullptr;
    }

    static int FindAccessorInPrimitive(const tg3_primitive& primitive, std::string_view accessor)
    {
        for (size_t i = 0; i < primitive.attributes_count; ++i)
        {
            if (SAFE_STRCMP_S_S(
                primitive.attributes[i].key.data,
                (size_t)(primitive.attributes[i].key.len),
                accessor.data(),
                accessor.size()) == 0
                )
                return primitive.attributes[i].value;
        }

        return -1;
    }

    static float ValueAsFloat(const tg3_value& object)
    {
#ifdef CONFIG_DEBUG
        AssertOrErrorCall(object.type == TG3_VALUE_REAL || object.type == TG3_VALUE_INT, return 0, "Given TG3 Object is not of the numeric type")
#endif // CONFIG_DEBUG
        
        return object.type == TG3_VALUE_REAL ?
            static_cast<float>(object.real_val) :
            object.type == TG3_VALUE_INT ?
                static_cast<float>(object.int_val) :
                0; 
    }

    static bool TryLoopOverBufferFromAccessor(
        const tinygltf3::Model& model, uint32_t AccessorIndex,
        const std::function<bool(const tg3_accessor& Accessor)>& execCheckType,
        std::span<const uint8_t>& OutView, size_t& OutElementCount)
    {
        const auto& accessor = model->accessors[AccessorIndex];
        if(!execCheckType(accessor)) return false;
        
        const auto& bufferView = model->buffer_views[accessor.buffer_view];
        const auto& buffer = model->buffers[bufferView.buffer];

        const uint8_t* rawBuffer = buffer.data.data + bufferView.byte_offset;
        const size_t rawBufferSize = bufferView.byte_length;
        const size_t byteStride = tg3_component_size(accessor.component_type) * tg3_num_components(accessor.type);
        
        OutView = std::span<const uint8_t>(rawBuffer, rawBuffer + rawBufferSize);
        OutElementCount = rawBufferSize / byteStride;

        return true;
    }

    Transform GetNodeWorldTransform(const tinygltf3::Model& model, const tg3_node& NodeObject)
    {
        Transform transform;

        if (NodeObject.has_matrix == 1)
        {
            transform.Type = Transform::Matrix;
            for (size_t i = 0; i < 16; ++i)
            {
                transform.Value.asMatrix[i] = static_cast<float>(NodeObject.matrix[i]);
            }
            
            return transform;
        }

        transform.Type = Transform::Properties;
        transform.Value.asProperties = Math::WorldTransformF{};
        transform.Value.asProperties.Position = Math::Point3f(
            static_cast<float>(NodeObject.translation[0]),
            static_cast<float>(NodeObject.translation[1]),
            static_cast<float>(NodeObject.translation[2])
            );

        transform.Value.asProperties.Rotation = Math::QuaternionF(
            static_cast<float>(NodeObject.rotation[0]),
            static_cast<float>(NodeObject.rotation[1]),
            static_cast<float>(NodeObject.rotation[2]),
            static_cast<float>(NodeObject.rotation[3])
            );

        transform.Value.asProperties.Scale = Math::Vector3f(
            static_cast<float>(NodeObject.scale[0]),
            static_cast<float>(NodeObject.scale[1]),
            static_cast<float>(NodeObject.scale[2])
            );
        
        return transform;
    }

    void LoadSceneTree(
        const tinygltf3::Model& model,
        int node,
        CPUScene& outScene, const Transform& parent = Transform{}
        )
    {
        const auto& NodeObject = model->nodes[node];

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
        for (size_t i = 0; i < NodeObject.children_count; i++)
            LoadSceneTree(model,  NodeObject.children[i], outScene, NewParentTransform);
        
        // Skip unsupported scene elements
        if(NodeObject.mesh == -1) return;

        uint8_t vertexGroupIndex = 0;
        for (size_t i = 0; i < model->meshes[NodeObject.mesh].primitives_count; i++)
        {
            const auto & primitive = model->meshes[NodeObject.mesh].primitives[i];
            MeshInstance inst;
            inst.mesh = NodeObject.mesh;
            inst.material = primitive.material;
            inst.transform = transformIndex;
            inst.vertexGroup = vertexGroupIndex++;

            outScene.instances.emplace_back(inst);
        }
    }
    
    bool LoadCPUScene(const std::filesystem::path& path, CPUScene& scene)
    {
        AssertOrErrorCallF(exists(path), return false, "No such file or directory \"%s\"", path.generic_string().c_str())

        AssertOrErrorCallF(exists(path), return false, "No such file or directory \"%s\"", path.generic_string().c_str())
        if(!exists(path)) return false;
        
        tinygltf3::Model model;
        tinygltf3::ErrorStack errors;
        tg3_parse_options options;

        tg3_parse_options_init(&options);
        // options.image.load_image = &ImageLoadCallback;
        // options.image.free_image = &ImageReleaseCallback;        
        tg3_error_code rc = tinygltf3::parse_file(model, errors, path.generic_string().c_str(), &options);
        AssertOrErrorCall(rc == TG3_OK, goto on_failed_tg3_parse, "Failed to load gltf model")

        // 0 - Clear & Reserve memory
        scene.Clear();
        scene.textures.reserve(model->textures_count);
        scene.materials.reserve(model->materials_count);
        scene.meshes.reserve(model->meshes_count);
        
        // 1 - Textures
        {
            for (size_t i = 0; i < model->textures_count; ++i)
            {
                const auto& texture = model->textures[i];
                const auto & image = model->images[texture.source];
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
                        
                    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGEF("Unsupported image %s, unsupported component count (%d)", texture.name.data, image.component)
                    }
                
                Image::Type Type;
                Image::Encoding Encoding = Image::Linear;
                switch(image.pixel_type)
                {
                case TG3_COMPONENT_TYPE_BYTE: 
                    Type = Image::Byte;
                    break;
                case TG3_COMPONENT_TYPE_UNSIGNED_BYTE:                     
                    Type = Image::UnsignedByte;
                    Encoding = Image::sRGB;
                    break;
                case TG3_COMPONENT_TYPE_SHORT: 
                    Type = Image::Short;
                    break;
                case TG3_COMPONENT_TYPE_UNSIGNED_SHORT: 
                    Type = Image::UnsignedShort;
                    break;
                case TG3_COMPONENT_TYPE_INT: 
                    Type = Image::Int;
                    break;
                case TG3_COMPONENT_TYPE_UNSIGNED_INT: 
                    Type = Image::UnsignedInt;
                    break;
                case TG3_COMPONENT_TYPE_FLOAT: 
                    Type = Image::Float;
                    break;
                case TG3_COMPONENT_TYPE_DOUBLE: 
                    Type = Image::Double;
                    break;
                        
                    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGEF("Unsupported image %s, unsupported data type (%d)", texture.name.data, image.pixel_type)
                    }
                
                scene.textures.emplace_back(image.width, image.height, Type, Layout, Encoding, image.image.data);
            }
        }

        // 2 - Materials
        {
            for (size_t i = 0; i < model->materials_count; ++i)
            {
                const auto& material = model->materials[i];
                scene.materials.emplace_back();
                Material& MaterialObject = *(scene.materials.end() - 1);

                // Unpack color
                MaterialObject.color = Math::Vector4f(
                    static_cast<float>(material.pbr_metallic_roughness.base_color_factor[0]),
                    static_cast<float>(material.pbr_metallic_roughness.base_color_factor[1]),
                    static_cast<float>(material.pbr_metallic_roughness.base_color_factor[2]),
                    1.0f
                );
                if(material.pbr_metallic_roughness.base_color_texture.index >= 0)
                {
                    MaterialObject.colorTexture = material.pbr_metallic_roughness.base_color_texture.index;
                }
                MaterialObject.emissive = Math::Vector4f(
                    static_cast<float>(material.emissive_factor[0]),
                    static_cast<float>(material.emissive_factor[1]),
                    static_cast<float>(material.emissive_factor[2]),
                    1.0f
                );
                if(material.emissive_texture.index >= 0)
                {
                    MaterialObject.emissiveTexture = material.emissive_texture.index;
                }

                // Unpack reflection
                MaterialObject.roughness = static_cast<float>(material.pbr_metallic_roughness.roughness_factor);
                MaterialObject.metallic = static_cast<float>(material.pbr_metallic_roughness.metallic_factor);
                if(material.pbr_metallic_roughness.metallic_roughness_texture.index >= 0)
                {
                    MaterialObject.metallicRoughnessTexture = material.pbr_metallic_roughness.metallic_roughness_texture.index;
                }
                 
                // Unpack AO
                if(material.occlusion_texture.index >= 0)
                {
                    MaterialObject.occlusionTexture = material.occlusion_texture.index;
                }

                // Get normal map
                if(material.normal_texture.index >= 0)
                {
                    MaterialObject.normalTexture = material.normal_texture.index;
                }

                MaterialObject.flags = MaterialObject.flags | (material.double_sided ? Material::EFlags::TwoSided :  Material::EFlags::None);
                if (SAFE_STRCMP_S_LS(material.alpha_mode.data, material.alpha_mode.len, "BLEND") == 0) 
                {
                    MaterialObject.flags = MaterialObject.flags | Material::EFlags::Transparent;
                }
                if (SAFE_STRCMP_S_LS(material.alpha_mode.data, material.alpha_mode.len, "MASK") == 0)
                {
                    MaterialObject.flags = MaterialObject.flags | Material::EFlags::Masked;
                }

                //KHR_materials_ior
                if (const tg3_extension* extension = FindExtension(material.ext, "KHR_materials_ior"); extension != nullptr)
                {
                    MaterialObject.flags = MaterialObject.flags | Material::EFlags::UseIORExt;
                    if (const tg3_value* ior = FindValueInObject(extension->value, "ior"); ior != nullptr && ior->type == TG3_VALUE_REAL || ior->type == TG3_VALUE_INT)
                    {
                        MaterialObject.ior = ValueAsFloat(*ior);
                    }
                }

                //KHR_materials_specular
                if (const tg3_extension* extension = FindExtension(material.ext, "KHR_materials_specular"); extension != nullptr)
                {
                    MaterialObject.flags = MaterialObject.flags | Material::EFlags::UseSpecularExt;

                    if (const tg3_value* specularFactor = FindValueInObject(extension->value, "specularFactor"); specularFactor != nullptr && specularFactor->type == TG3_VALUE_REAL || specularFactor->type == TG3_VALUE_INT)
                    {
                        MaterialObject.specular = ValueAsFloat(*specularFactor);
                    }

                    if (const tg3_value* specularTex = FindValueInObject(extension->value, "specularTexture"); specularTex != nullptr && specularTex->type == TG3_VALUE_INT && specularTex->int_val >= 0)
                    {
                        MaterialObject.specularTexture = static_cast<uint32_t>(specularTex->int_val); 
                    }

                    if (const tg3_value* specularColorTex = FindValueInObject(extension->value, "specularColorFactor"); specularColorTex != nullptr && specularColorTex->type == TG3_VALUE_ARRAY && specularColorTex->array_count == 3)
                    {
                        MaterialObject.specularColor.x = ValueAsFloat(specularColorTex->array_data[0]);
                        MaterialObject.specularColor.x = ValueAsFloat(specularColorTex->array_data[1]);
                        MaterialObject.specularColor.x = ValueAsFloat(specularColorTex->array_data[2]);
                        MaterialObject.specularColor.w = 1.0f;
                    }

                    if (const tg3_value* specularColorTex = FindValueInObject(extension->value, "specularColorTexture"); specularColorTex != nullptr && specularColorTex->type == TG3_VALUE_INT && specularColorTex->int_val >= 0)
                    {
                        MaterialObject.specularColorTexture = static_cast<uint32_t>(specularColorTex->int_val); 
                    }
                }

                //KHR_materials_transmission
                if (const tg3_extension* extension = FindExtension(material.ext, "KHR_materials_transmission"); extension != nullptr)
                {
                    MaterialObject.flags = MaterialObject.flags | Material::EFlags::UseTransmissionExt;

                    if (const tg3_value* transmission = FindValueInObject(extension->value, "transmissionFactor"); transmission != nullptr && transmission->type == TG3_VALUE_REAL || transmission->type == TG3_VALUE_INT)
                    {
                        MaterialObject.transmission = ValueAsFloat(*transmission);
                    }

                    if (const tg3_value* transmissionTex = FindValueInObject(extension->value, "transmissionTexture"); transmissionTex != nullptr && transmissionTex->type == TG3_VALUE_INT && transmissionTex->int_val >= 0)
                    {
                        MaterialObject.specularColorTexture = static_cast<uint32_t>(transmissionTex->int_val); 
                    }
                }
                
                // KHR_materials_pbrSpecularGlossiness
                if (const tg3_extension* extension = FindExtension(material.ext, "KHR_materials_pbrSpecularGlossiness"); extension != nullptr)
                {
                    MaterialObject.flags = MaterialObject.flags | Material::EFlags::UseSpecularGlossinessPBRExt;
                    
                    if (const tg3_value* diffuseTex = FindValueInObject(extension->value, "diffuseTexture"); diffuseTex != nullptr && diffuseTex->type == TG3_VALUE_INT && diffuseTex->int_val >= 0)
                    {
                        MaterialObject.colorTexture = static_cast<uint32_t>(diffuseTex->int_val); 
                    }

                    if (const tg3_value* diffuseColor = FindValueInObject(extension->value, "diffuseFactor"); diffuseColor != nullptr && diffuseColor->type == TG3_VALUE_ARRAY && diffuseColor->array_count == 4)
                    {
                        MaterialObject.color.x = ValueAsFloat(diffuseColor->array_data[0]);
                        MaterialObject.color.x = ValueAsFloat(diffuseColor->array_data[1]);
                        MaterialObject.color.x = ValueAsFloat(diffuseColor->array_data[2]);
                        MaterialObject.color.w = ValueAsFloat(diffuseColor->array_data[3]);
                    }

                    if (const tg3_value* specularColor = FindValueInObject(extension->value, "specularFactor"); specularColor != nullptr && specularColor->type == TG3_VALUE_ARRAY && specularColor->array_count == 3)
                    {
                        MaterialObject.specularColor.x = ValueAsFloat(specularColor->array_data[0]);
                        MaterialObject.specularColor.x = ValueAsFloat(specularColor->array_data[1]);
                        MaterialObject.specularColor.x = ValueAsFloat(specularColor->array_data[2]);
                        MaterialObject.specularColor.w = 1.0f;
                    }

                    if (const tg3_value* glossinessFactor = FindValueInObject(extension->value, "glossinessFactor"); glossinessFactor != nullptr && glossinessFactor->type == TG3_VALUE_REAL || glossinessFactor->type == TG3_VALUE_INT)
                    {
                        MaterialObject.roughness = 1.f - ValueAsFloat(*glossinessFactor);
                    }

                    if (const tg3_value* specularGlossinessTex = FindValueInObject(extension->value, "specularGlossinessTexture"); specularGlossinessTex != nullptr && specularGlossinessTex->type == TG3_VALUE_INT && specularGlossinessTex->int_val >= 0)
                    {
                        MaterialObject.specularTexture = static_cast<uint32_t>(specularGlossinessTex->int_val); 
                    }
                }
            }
        }

        // 3 - Meshes
        {
            for (size_t i = 0; i < model->meshes_count; ++i)
            {
                const auto& mesh = model->meshes[i];
                scene.meshes.emplace_back();
                Mesh& MeshObject = *(scene.meshes.end() - 1);

                const int MainPrimitiveType = mesh.primitives[0].mode;
                switch (MainPrimitiveType)
                {
                case TG3_MODE_POINTS:          MeshObject.BeginMesh(Mesh::VertexType::POINTS); break;
                case TG3_MODE_LINE:            MeshObject.BeginMesh(Mesh::VertexType::LINES); break;
                case TG3_MODE_LINE_LOOP:       MeshObject.BeginMesh(Mesh::VertexType::LINE_LOOP); break;
                case TG3_MODE_LINE_STRIP:      MeshObject.BeginMesh(Mesh::VertexType::LINE_STRIP); break;
                case TG3_MODE_TRIANGLES:       MeshObject.BeginMesh(Mesh::VertexType::TRIANGLES); break;
                case TG3_MODE_TRIANGLE_STRIP:  MeshObject.BeginMesh(Mesh::VertexType::TRIANGLE_STRIP); break;
                case TG3_MODE_TRIANGLE_FAN:    MeshObject.BeginMesh(Mesh::VertexType::TRIANGLE_FAN); break;
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
                    size_t Count;
                };
                
                // states
                std::vector<BufferDataView> Indexes;
                std::vector<BufferDataView> Positions;
                std::vector<BufferDataView> Normals;
                std::vector<BufferDataView> TextCoords;
                size_t PreviousVertexCount = 0;
                
                for (size_t j = 0; j < mesh.primitives_count; ++j)
                {
                    const auto & primitive = mesh.primitives[j];
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
                            [](const tg3_accessor& Accessor)
                            {
                                switch(Accessor.component_type)
                                {
                                case TG3_COMPONENT_TYPE_BYTE:
                                case TG3_COMPONENT_TYPE_UNSIGNED_BYTE:
                                case TG3_COMPONENT_TYPE_SHORT:
                                case TG3_COMPONENT_TYPE_UNSIGNED_SHORT:
                                case TG3_COMPONENT_TYPE_INT:
                                case TG3_COMPONENT_TYPE_UNSIGNED_INT:
                                    return true;
                                default:
                                    EngineLoggerError("Unsupported index component type for vertex indexation.");
                                    return false;
                                }
                            },
                            CurrentView, CurrentVertexCount);

                        if(!success) continue;
                        
                        AssertOrErrorCall(Indexes.empty() || Indexes.front().TinyGLTFComponentType ==  model->accessors[primitive.indices].component_type, continue;, "Primitive index buffer type missmatch")
                        
                        Indexes.push_back(BufferDataView{.TinyGLTFComponentType = model->accessors[primitive.indices].component_type, .View = CurrentView, .Count = CurrentVertexCount});
                    }
                    
                    if (int AccessorIndex = FindAccessorInPrimitive(primitive, "POSITION"); AccessorIndex >= 0)
                    {
                        bool success = TryLoopOverBufferFromAccessor(model, AccessorIndex,
                            [](const tg3_accessor& Accessor)
                            {
                                AssertOrErrorCall(Accessor.type == TG3_TYPE_VEC3, return false;, "Unsupported primitive position component type. Position requires a vector 3.")
                                AssertOrErrorCall(Accessor.component_type == TG3_COMPONENT_TYPE_FLOAT || Accessor.component_type == TG3_COMPONENT_TYPE_DOUBLE, return false;,
                                    "Unsupported primitive position component type. Position vector needs to be made of float or double.")

                                return true;
                            },
                            CurrentView, IsIndexed ? Dummy : CurrentVertexCount);

                        if(!success) continue;
                        
                        AssertOrErrorCall(Positions.empty() || Positions.front().TinyGLTFComponentType ==  model->accessors[AccessorIndex].component_type, continue;, "Primitive buffer type missmatch")
                        
                        Positions.push_back(BufferDataView{.TinyGLTFComponentType = model->accessors[AccessorIndex].component_type, .View = CurrentView, .Count = IsIndexed ? Dummy : CurrentVertexCount});
                    }
                    if (int AccessorIndex = FindAccessorInPrimitive(primitive, "NORMAL"); AccessorIndex >= 0)
                    {
                        bool success = TryLoopOverBufferFromAccessor(model, AccessorIndex,
                            [](const tg3_accessor& Accessor)
                            {
                                AssertOrErrorCall(Accessor.type == TG3_TYPE_VEC3, return false;, 
                                    "Unsupported primitive normal component type. Normal requires a vector 3.")
                                AssertOrErrorCall(Accessor.component_type == TG3_COMPONENT_TYPE_FLOAT || Accessor.component_type == TG3_COMPONENT_TYPE_DOUBLE, return false;,
                                    "Unsupported primitive normal component type. Normal vector needs to be made of float or double.")

                                return true;
                            },
                            CurrentView, IsIndexed ? Dummy : CurrentVertexCount);

                        if(!success) continue;
                        
                        AssertOrErrorCall(Normals.empty() || Normals.front().TinyGLTFComponentType ==  model->accessors[AccessorIndex].component_type, continue;, "Primitive buffer type missmatch")
                        
                        Normals.push_back(BufferDataView{.TinyGLTFComponentType = model->accessors[AccessorIndex].component_type, .View = CurrentView, .Count = IsIndexed ? Dummy : CurrentVertexCount});
                    }
                    if (int AccessorIndex = FindAccessorInPrimitive(primitive, "TEXCOORD_0"); AccessorIndex >= 0)
                    {
                        bool success = TryLoopOverBufferFromAccessor(model, AccessorIndex,
                            [](const tg3_accessor& Accessor)
                            {
                                AssertOrErrorCall(Accessor.type == TG3_TYPE_VEC2, return false;, 
                                    "Unsupported primitive texture coordinates 0 component type. Texture coordinates 0 requires a vector 2.")
                                AssertOrErrorCall(Accessor.component_type == TG3_COMPONENT_TYPE_FLOAT || Accessor.component_type == TG3_COMPONENT_TYPE_DOUBLE, return false;,
                                    "Unsupported primitive texture coordinates 0 component type. Texture coordinates 0 needs to be made of float or double.")

                                return true;
                            },
                            CurrentView, IsIndexed ? Dummy : CurrentVertexCount);

                        if(!success) continue;
                        
                        AssertOrErrorCall(TextCoords.empty() || TextCoords.front().TinyGLTFComponentType ==  model->accessors[AccessorIndex].component_type, continue;, "Primitive buffer type missmatch")
                        
                        TextCoords.push_back(BufferDataView{.TinyGLTFComponentType = model->accessors[AccessorIndex].component_type, .View = CurrentView, .Count = IsIndexed ? Dummy : CurrentVertexCount});
                    }
                    
                    // todo vertex color
                    MeshObject.AddVertexGroup((unsigned int)PreviousVertexCount, (unsigned int)CurrentVertexCount - (unsigned int)PreviousVertexCount);
                }
                
                if (!Indexes.empty())
                {
                    switch(Indexes.front().TinyGLTFComponentType)
                    {
                    case TG3_COMPONENT_TYPE_BYTE:
                        {
                            for (const auto& view : Indexes)
                            {
                                for (size_t i = 0; i < view.Count; i++)
                                {
                                    MeshObject.AddVertexPolygonIndex(static_cast<uint32_t>( ((int8_t*)(view.View.data()))[i] ));
                                }
                            }
                        }
                        break;
                        
                    case TG3_COMPONENT_TYPE_UNSIGNED_BYTE:
                        {
                            for (const auto& view : Indexes)
                            {
                                for (size_t i = 0; i < view.Count; i++)
                                {
                                    MeshObject.AddVertexPolygonIndex(static_cast<uint32_t>( ((uint8_t*)(view.View.data()))[i] ));
                                }
                            }
                        }
                        break;
                        
                    case TG3_COMPONENT_TYPE_SHORT:
                        {
                            for (const auto& view : Indexes)
                            {
                                for (size_t i = 0; i < view.Count; i++)
                                {
                                    MeshObject.AddVertexPolygonIndex(static_cast<uint32_t>( ((int16_t*)(view.View.data()))[i] ));
                                }
                            }
                        }
                        break;
                        
                    case TG3_COMPONENT_TYPE_UNSIGNED_SHORT:
                        {
                            for (const auto& view : Indexes)
                            {
                                for (size_t i = 0; i < view.Count; i++)
                                {
                                    MeshObject.AddVertexPolygonIndex(static_cast<uint32_t>( ((uint16_t*)(view.View.data()))[i] ));
                                }
                            }
                        }
                        break;
                        
                    case TG3_COMPONENT_TYPE_INT:
                        {
                            for (const auto& view : Indexes)
                            {
                                for (size_t i = 0; i < view.Count; i++)
                                {
                                    MeshObject.AddVertexPolygonIndex(static_cast<uint32_t>( ((int32_t*)(view.View.data()))[i] ));
                                }
                            }
                        }
                        break;
                        
                    case TG3_COMPONENT_TYPE_UNSIGNED_INT:
                        {
                            for (const auto& view : Indexes)
                            {
                                MeshObject.AddVertexPolygonIndexes({(const uint32_t*)(view.View.data()), view.Count });
                            }
                        }
                        break;
                            
                    default:
                        UNREACHABLE;
                    }
                }
                if (!Positions.empty())
                {
                    switch (Positions.front().TinyGLTFComponentType)
                    {
                    case TG3_COMPONENT_TYPE_FLOAT:
                        {
                            for (const auto& view : Positions)
                            {
                                MeshObject.AddVertexPositions({(const Math::Point3f*)(view.View.data()), view.Count});
                            }
                        }
                        break;
                        
                    case TG3_COMPONENT_TYPE_DOUBLE:
                        {
                            for (const auto& view : Positions)
                            {
                                for (size_t i = 0; i < view.Count; i++)
                                {
                                    const Math::Point3d* asVec3d = (const Math::Point3d*)(view.View.data()) + i;
                                    MeshObject.AddVertexPosition(Math::Point3f(
                                       static_cast<float>(asVec3d->x),
                                       static_cast<float>(asVec3d->y),
                                       static_cast<float>(asVec3d->z)
                                       ));
                                }
                            }
                        }
                        break;
                    
    #ifdef CONFIG_DEBUG
                        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGEF("Unsupported glTF Primitive Mode %d", Positions.front().TinyGLTFComponentType)
        #else // CONFIG_DEBUG
                        default:
                        EngineLoggerErrorF("Unsupported glTF Primitive Mode %d", Positions.front().TinyGLTFComponentType);
    #endif // !CONFIG_DEBUG
                    }
                }
                if (!Normals.empty())
                {                
                    switch (Normals.front().TinyGLTFComponentType)
                    {
                    case TG3_COMPONENT_TYPE_FLOAT:
                        {
                            for (const auto& view : Normals)
                            {
                                MeshObject.AddVertexNormals({(const Math::Vector3f*)(view.View.data()), view.Count});
                            }
                        }
                        break;
                        
                    case TG3_COMPONENT_TYPE_DOUBLE:
                        {
                            for (const auto& view : Normals)
                            {
                                for (size_t i = 0; i < view.Count; i++)
                                {
                                    const Math::Vector3d* asVec3d = (const Math::Vector3d*)(view.View.data()) + i;
                                    MeshObject.AddVertexNormal(Math::Vector3f(
                                       static_cast<float>(asVec3d->x),
                                       static_cast<float>(asVec3d->y),
                                       static_cast<float>(asVec3d->z)
                                       ));
                                }
                            }
                        }
                        break;
                    }
                }
                if (!TextCoords.empty())
                {                
                    switch (TextCoords.front().TinyGLTFComponentType)
                    {
                    case TG3_COMPONENT_TYPE_FLOAT:
                        {
                            for (const auto& view : TextCoords)
                            {
                                MeshObject.AddVertexTextureCoordinates({(const Math::Vector2f*)(view.View.data()), view.Count});
                            }
                        }
                        break;
                        
                    case TG3_COMPONENT_TYPE_DOUBLE:
                        {
                            for (const auto& view : TextCoords)
                            {
                                for (size_t i = 0; i < view.Count; i++)
                                {
                                    const Math::Vector2d* asVec2d = (const Math::Vector2d*)(view.View.data()) + i;
                                    MeshObject.AddVertexTextureCoordinate(Math::Vector2f(
                                       static_cast<float>(asVec2d->x),
                                       static_cast<float>(asVec2d->y)
                                       ));
                                }
                            }
                        }
                        break;
                    }
                    
                    // todo fix, when having smaller pool of textcoord in some meshes, fils textcoords to match position buffer
                    if (MeshObject.GetPositions().size() > MeshObject.GetTextureCoordinates().size())
                    {
                        for (size_t i = MeshObject.GetTextureCoordinates().size(); i < MeshObject.GetPositions().size(); i++)
                        {
                            MeshObject.AddVertexTextureCoordinate(Math::Vector2f(0.f));
                        }
                    }
                }            
                
                MeshObject.CommitMesh();
            }

        }

        // 4 - Scene tree
        {
            uint32_t sceneIndex = 0;
            AssertOrErrorCall(model->scenes_count > sceneIndex, return false, "No scene found")

            for (int i = 0; i < model->scenes[sceneIndex].nodes_count; i++)
                LoadSceneTree(model, model->scenes[sceneIndex].nodes[i], scene);
        }
        
        return true;
        // model->textures->

    on_failed_tg3_parse:
        for (uint32_t i = 0; i < errors.count(); i++)
        {
            _LoggerFormatedLogF("ERROR", "[%d] %s",(int)errors.entry(i)->severity, (errors.entry(i)->message ? errors.entry(i)->message : "(null)"));
        }
        goto on_failed_final;
        
    on_failed_final:
        return false;
    }
    
/*
    static struct CustomImageLoaderState
    {
        // std::filesystem::path gltfPath;
        Image::FileType imageFileType;
        bool imageHasReplacedExtension;
    };

    static bool IsImageExention(std::string_view extention)
    {
        if (
            extention.compare(".dds") == 0 ||
            extention.compare(".exr") == 0 ||
            extention.compare(".png") == 0 ||
            extention.compare(".jpg") == 0 ||
            extention.compare(".jepg") == 0 ||
            extention.compare(".tga") == 0 ||
            extention.compare(".bmp") == 0 ||
            extention.compare(".hdr") == 0 
            ) 
            return true;

        return false;
    }
    
    static bool CustomFileExists(const std::string &abs_filename, void *user_data) 
    {
        CustomImageLoaderState* State = static_cast<CustomImageLoaderState*>(user_data);
        State->imageHasReplacedExtension = false;

        std::filesystem::path p(abs_filename);
        if (p.has_extension())
        {
            static std::array<const std::string_view, 8> extensionsToCheck {
                ".dds", ".exr", ".png", ".jpg", ".jpeg", ".tga", ".bmp", ".hdr"
            };
                
            static std::array<const Image::FileType, 8> extensionsType {
                Image::FileType::DDS,
                Image::FileType::EXR, 
                Image::FileType::PNG, 
                Image::FileType::JPG, 
                Image::FileType::JPG, 
                Image::FileType::TGA,
                Image::FileType::BMP, 
                Image::FileType::HDR
            };
                
            
            std::string extension = p.extension().generic_string();
            
            if (extension == ".gltf" || extension == ".glb") {}
            else
                if (IsImageExention(extension))
                {
                    if (! std::filesystem::exists(p))
                    {
                        State->imageHasReplacedExtension = true;
                        for (size_t i = 0; i < extensionsToCheck.size(); i++)
                        {
                            p.replace_extension(extensionsToCheck[i]);
                            if (std::filesystem::exists(p))
                            {
                                State->imageFileType = extensionsType[i];
                                break;
                            }
                        }
                    }
                    else
                    {
                        for (size_t i = 0; i < extensionsToCheck.size(); i++)
                        {
                            if (extension == extensionsToCheck[i])
                            {
                                State->imageFileType = extensionsType[i];
                                break;
                            }
                        }
                    }
                
                    if (! std::filesystem::exists(p))
                    {
                        EngineLoggerErrorF("Failed to find file \"%ls\"", p.c_str());
                        return false;
                    }
                    else
                    {
                        return true;
                    }
                }
                else
                {
                    EngineLoggerErrorF("Unsupported file extension type \"%ls\"", p.extension().c_str());
                }
        }
        
        return tinygltf::FileExists(abs_filename, user_data);
    }

    static bool CustomReadWholeFile(std::vector<unsigned char> *out, std::string *err, const std::string &filepath, void *user_data) 
    {
        CustomImageLoaderState* State = static_cast<CustomImageLoaderState*>(user_data);
        
        if (State->imageHasReplacedExtension)
        {
            std::filesystem::path p(filepath);
            switch (State->imageFileType)
            {
            case Image::JPG:
                p.replace_extension(".jpg");
                if (std::filesystem::exists(p))
                {
                    return tinygltf3::ReadWholeFile(out, err, p.generic_string(), user_data);
                }
                
                p.replace_extension(".jpeg");
                if (std::filesystem::exists(p))
                {
                    return tinygltf::ReadWholeFile(out, err, p.generic_string(), user_data);
                }
                break;
            case Image::PNG:
                p.replace_extension(".png");
                if (std::filesystem::exists(p))
                {
                    return tinygltf::ReadWholeFile(out, err, p.generic_string(), user_data);
                }
                break;
            case Image::TGA:
                p.replace_extension(".tga");
                if (std::filesystem::exists(p))
                {
                    return tinygltf::ReadWholeFile(out, err, p.generic_string(), user_data);
                }
                break;
            case Image::BMP:
                p.replace_extension(".bmp");
                if (std::filesystem::exists(p))
                {
                    return tinygltf::ReadWholeFile(out, err, p.generic_string(), user_data);
                }
                break;
            case Image::HDR:
                p.replace_extension(".hdr");
                if (std::filesystem::exists(p))
                {
                    return tinygltf::ReadWholeFile(out, err, p.generic_string(), user_data);
                }
                break;
            case Image::DDS:
                p.replace_extension(".dds");
                if (std::filesystem::exists(p))
                {
                    return tinygltf::ReadWholeFile(out, err, p.generic_string(), user_data);
                }
                break;
            case Image::EXR:
                p.replace_extension(".exr");
                if (std::filesystem::exists(p))
                {
                    return tinygltf::ReadWholeFile(out, err, p.generic_string(), user_data);
                }
                break;
                
                SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported image file type")
                }
        }
        
        return tinygltf::ReadWholeFile(out, err, filepath, user_data);
    }

    static bool CustomGetFileSizeInBytes(size_t *filesize_out, std::string *err, const std::string &filepath, void *user_data) 
    {
        CustomImageLoaderState* State = static_cast<CustomImageLoaderState*>(user_data);
        
        if (State->imageHasReplacedExtension)
        {
            std::filesystem::path p(filepath);
            switch (State->imageFileType)
            {
            case Image::JPG:
                p.replace_extension(".jpg");
                if (std::filesystem::exists(p))
                {
                    return tinygltf::GetFileSizeInBytes(filesize_out, err, p.generic_string(), user_data);
                }
                
                p.replace_extension(".jpeg");
                if (std::filesystem::exists(p))
                {
                    return tinygltf::GetFileSizeInBytes(filesize_out, err, p.generic_string(), user_data);
                }
                break;
            case Image::PNG:
                p.replace_extension(".png");
                if (std::filesystem::exists(p))
                {
                    return tinygltf::GetFileSizeInBytes(filesize_out, err, p.generic_string(), user_data);
                }
                break;
            case Image::TGA:
                p.replace_extension(".tga");
                if (std::filesystem::exists(p))
                {
                    return tinygltf::GetFileSizeInBytes(filesize_out, err, p.generic_string(), user_data);
                }
                break;
            case Image::BMP:
                p.replace_extension(".bmp");
                if (std::filesystem::exists(p))
                {
                    return tinygltf::GetFileSizeInBytes(filesize_out, err, p.generic_string(), user_data);
                }
                break;
            case Image::HDR:
                p.replace_extension(".hdr");
                if (std::filesystem::exists(p))
                {
                    return tinygltf::GetFileSizeInBytes(filesize_out, err, p.generic_string(), user_data);
                }
                break;
            case Image::DDS:
                p.replace_extension(".dds");
                if (std::filesystem::exists(p))
                {
                    return tinygltf::GetFileSizeInBytes(filesize_out, err, p.generic_string(), user_data);
                }
                break;
            case Image::EXR:
                p.replace_extension(".exr");
                if (std::filesystem::exists(p))
                {
                    return tinygltf::GetFileSizeInBytes(filesize_out, err, p.generic_string(), user_data);
                }
                break;
                
                SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported image file type")
                }
        }
        
        return tinygltf::GetFileSizeInBytes(filesize_out, err, filepath, user_data);
    }

    static bool CustomImageLoader(tinygltf::Image* image, const int image_idx, std::string* err, std::string* warn,
                     int req_width, int req_height, const unsigned char* bytes, int size, void* user_data)
    {
        if (size == 0) return false;
        
        CustomImageLoaderState* State = static_cast<CustomImageLoaderState*>(user_data);
        
        Image LoadedImage = ImageLoadFromMemory(bytes, size, State->imageFileType);
        if (LoadedImage.Width() == 0 || LoadedImage.Height() == 0)
        {
            if (err) *err += "Failed to decode image from memory.";
#ifdef CONFIG_DEBUG
            EngineRuntimeBREAKPOINT
#endif // CONFIG_DEBUG
            return false;
        }
        image->width = static_cast<int>(LoadedImage.Width());
        image->height = static_cast<int>(LoadedImage.Height());
        image->component = static_cast<int>(LoadedImage.ComponentCount());
        image->bits = 8;
        image->pixel_type = TG3_COMPONENT_TYPE_UNSIGNED_BYTE;
        image->image.resize(LoadedImage.DataSize());
        std::memcpy(image->image.data(), LoadedImage.Data(), LoadedImage.DataSize());
        return true;
    }
    */
    
    void LoadSceneTree(
        const tinygltf3::Model& model,
        int node,
        GPUScene& outScene, const Transform& parent = Transform()
        )
    {
        const auto& NodeObject = model->nodes[node];

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
        for (size_t i = 0; i < NodeObject.children_count; i++)
            LoadSceneTree(model,  NodeObject.children[i], outScene, NewParentTransform);
        
        // Skip unsupported scene elements
        if(NodeObject.mesh == -1) return;

        uint8_t vertexGroupIndex = 0;
        for (size_t i = 0; i < model->meshes[NodeObject.mesh].primitives_count; i++)
        {
            const auto & primitive = model->meshes[NodeObject.mesh].primitives[i];
            MeshInstance inst;
            inst.mesh = NodeObject.mesh;
            inst.material = primitive.material;
            inst.transform = transformIndex;
            inst.vertexGroup = vertexGroupIndex++;

            outScene.instances.emplace_back(inst);
        }
    }

    void FindBoundsInPositionBuffer(std::span<const uint8_t> Buffer, int GLTFComponentType, Math::Point3f& Min, Math::Point3f& Max)
    {
        const size_t byteStride = tg3_component_size(GLTFComponentType) * tg3_num_components(TG3_TYPE_VEC3);
        const size_t ElementCount = Buffer.size() / byteStride;
        
        switch (GLTFComponentType)
        {
        case TG3_COMPONENT_TYPE_FLOAT:
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
            
        case TG3_COMPONENT_TYPE_DOUBLE:
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

    bool LoadGPUScene(const std::filesystem::path& path, GPUScene& scene)
    {
        AssertOrErrorCallF(exists(path), return false, "No such file or directory \"%s\"", path.generic_string().c_str())
        if(!exists(path)) return false;
        
        tinygltf3::Model model;
        tinygltf3::ErrorStack errors;
        tg3_parse_options options;

        tg3_parse_options_init(&options);
        // options.image.load_image = &ImageLoadCallback;
        // options.image.free_image = &ImageReleaseCallback;        
        tg3_error_code rc = tinygltf3::parse_file(model, errors, path.generic_string().c_str(), &options);
        AssertOrErrorCall(rc == TG3_OK, goto on_failed_tg3_parse, "Failed to load gltf model")

        // 0 - Clear & Reserve memory
        scene.Clear();
        scene.textures.reserve(model->textures_count);
        scene.materials.reserve(model->materials_count);
        scene.meshes.reserve(model->meshes_count);
        
        // 1 - Textures
        {
            for (size_t i = 0; i < model->textures_count; ++i)
            {
                const auto& texture = model->textures[i];
                const auto & image = model->images[texture.source];
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
                    
                    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGEF("Unsupported image %s, unsupported component count (%d)", texture.name.data, image.component)
                    }
            
                Image::Type Type;
                switch(image.pixel_type)
                {
                case TG3_COMPONENT_TYPE_BYTE: 
                    Type = Image::Byte;
                    break;
                case TG3_COMPONENT_TYPE_UNSIGNED_BYTE:                     
                    Type = Image::UnsignedByte;
                    break;
                case TG3_COMPONENT_TYPE_SHORT: 
                    Type = Image::Short;
                    break;
                case TG3_COMPONENT_TYPE_UNSIGNED_SHORT: 
                    Type = Image::UnsignedShort;
                    break;
                case TG3_COMPONENT_TYPE_INT: 
                    Type = Image::Int;
                    break;
                case TG3_COMPONENT_TYPE_UNSIGNED_INT: 
                    Type = Image::UnsignedInt;
                    break;
                case TG3_COMPONENT_TYPE_FLOAT: 
                    Type = Image::Float;
                    break;
                case TG3_COMPONENT_TYPE_DOUBLE: 
                    Type = Image::Double;
                    break;
                    
                    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGEF("Unsupported image %s, unsupported data type (%d)", texture.name.data, image.pixel_type)
                    }
            
                scene.textures.emplace_back(image.width, image.height, Type, Layout, image.image.data, image.image.count);
            }
        }

        // 2 - Materials
        {
            for (size_t i = 0; i < model->materials_count; ++i)
            {
                const auto& material = model->materials[i];
                scene.materials.emplace_back();
                Material& MaterialObject = *(scene.materials.end() - 1);

                // Unpack color
                MaterialObject.color = Math::Vector4f(
                    static_cast<float>(material.pbr_metallic_roughness.base_color_factor[0]),
                    static_cast<float>(material.pbr_metallic_roughness.base_color_factor[1]),
                    static_cast<float>(material.pbr_metallic_roughness.base_color_factor[2]),
                    1.0f
                );
                if(material.pbr_metallic_roughness.base_color_texture.index >= 0)
                {
                    MaterialObject.colorTexture = material.pbr_metallic_roughness.base_color_texture.index;
                }
                MaterialObject.emissive = Math::Vector4f(
                    static_cast<float>(material.emissive_factor[0]),
                    static_cast<float>(material.emissive_factor[1]),
                    static_cast<float>(material.emissive_factor[2]),
                    1.0f
                );
                if(material.emissive_texture.index >= 0)
                {
                    MaterialObject.emissiveTexture = material.emissive_texture.index;
                }

                // Unpack reflection
                MaterialObject.roughness = static_cast<float>(material.pbr_metallic_roughness.roughness_factor);
                MaterialObject.metallic = static_cast<float>(material.pbr_metallic_roughness.metallic_factor);
                if(material.pbr_metallic_roughness.metallic_roughness_texture.index >= 0)
                {
                    MaterialObject.metallicRoughnessTexture = material.pbr_metallic_roughness.metallic_roughness_texture.index;
                }
                 
                // Unpack AO
                if(material.occlusion_texture.index >= 0)
                {
                    MaterialObject.occlusionTexture = material.occlusion_texture.index;
                }

                // Get normal map
                if(material.normal_texture.index >= 0)
                {
                    MaterialObject.normalTexture = material.normal_texture.index;
                }

                MaterialObject.flags = MaterialObject.flags | (material.double_sided ? Material::EFlags::TwoSided :  Material::EFlags::None);
                if (SAFE_STRCMP_S_LS(material.alpha_mode.data, material.alpha_mode.len, "BLEND") == 0) 
                {
                    MaterialObject.flags = MaterialObject.flags | Material::EFlags::Transparent;
                }
                if (SAFE_STRCMP_S_LS(material.alpha_mode.data, material.alpha_mode.len, "MASK") == 0)
                {
                    MaterialObject.flags = MaterialObject.flags | Material::EFlags::Masked;
                }

                //KHR_materials_ior
                if (const tg3_extension* extension = FindExtension(material.ext, "KHR_materials_ior"); extension != nullptr)
                {
                    MaterialObject.flags = MaterialObject.flags | Material::EFlags::UseIORExt;
                    if (const tg3_value* ior = FindValueInObject(extension->value, "ior"); ior != nullptr && ior->type == TG3_VALUE_REAL || ior->type == TG3_VALUE_INT)
                    {
                        MaterialObject.ior = ValueAsFloat(*ior);
                    }
                }

                //KHR_materials_specular
                if (const tg3_extension* extension = FindExtension(material.ext, "KHR_materials_specular"); extension != nullptr)
                {
                    MaterialObject.flags = MaterialObject.flags | Material::EFlags::UseSpecularExt;

                    if (const tg3_value* specularFactor = FindValueInObject(extension->value, "specularFactor"); specularFactor != nullptr && specularFactor->type == TG3_VALUE_REAL || specularFactor->type == TG3_VALUE_INT)
                    {
                        MaterialObject.specular = ValueAsFloat(*specularFactor);
                    }

                    if (const tg3_value* specularTex = FindValueInObject(extension->value, "specularTexture"); specularTex != nullptr && specularTex->type == TG3_VALUE_INT && specularTex->int_val >= 0)
                    {
                        MaterialObject.specularTexture = static_cast<uint32_t>(specularTex->int_val); 
                    }

                    if (const tg3_value* specularColorTex = FindValueInObject(extension->value, "specularColorFactor"); specularColorTex != nullptr && specularColorTex->type == TG3_VALUE_ARRAY && specularColorTex->array_count == 3)
                    {
                        MaterialObject.specularColor.x = ValueAsFloat(specularColorTex->array_data[0]);
                        MaterialObject.specularColor.x = ValueAsFloat(specularColorTex->array_data[1]);
                        MaterialObject.specularColor.x = ValueAsFloat(specularColorTex->array_data[2]);
                        MaterialObject.specularColor.w = 1.0f;
                    }

                    if (const tg3_value* specularColorTex = FindValueInObject(extension->value, "specularColorTexture"); specularColorTex != nullptr && specularColorTex->type == TG3_VALUE_INT && specularColorTex->int_val >= 0)
                    {
                        MaterialObject.specularColorTexture = static_cast<uint32_t>(specularColorTex->int_val); 
                    }
                }

                //KHR_materials_transmission
                if (const tg3_extension* extension = FindExtension(material.ext, "KHR_materials_transmission"); extension != nullptr)
                {
                    MaterialObject.flags = MaterialObject.flags | Material::EFlags::UseTransmissionExt;

                    if (const tg3_value* transmission = FindValueInObject(extension->value, "transmissionFactor"); transmission != nullptr && transmission->type == TG3_VALUE_REAL || transmission->type == TG3_VALUE_INT)
                    {
                        MaterialObject.transmission = ValueAsFloat(*transmission);
                    }

                    if (const tg3_value* transmissionTex = FindValueInObject(extension->value, "transmissionTexture"); transmissionTex != nullptr && transmissionTex->type == TG3_VALUE_INT && transmissionTex->int_val >= 0)
                    {
                        MaterialObject.specularColorTexture = static_cast<uint32_t>(transmissionTex->int_val); 
                    }
                }
                
                // KHR_materials_pbrSpecularGlossiness
                if (const tg3_extension* extension = FindExtension(material.ext, "KHR_materials_pbrSpecularGlossiness"); extension != nullptr)
                {
                    MaterialObject.flags = MaterialObject.flags | Material::EFlags::UseSpecularGlossinessPBRExt;
                    
                    if (const tg3_value* diffuseTex = FindValueInObject(extension->value, "diffuseTexture"); diffuseTex != nullptr && diffuseTex->type == TG3_VALUE_INT && diffuseTex->int_val >= 0)
                    {
                        MaterialObject.colorTexture = static_cast<uint32_t>(diffuseTex->int_val); 
                    }

                    if (const tg3_value* diffuseColor = FindValueInObject(extension->value, "diffuseFactor"); diffuseColor != nullptr && diffuseColor->type == TG3_VALUE_ARRAY && diffuseColor->array_count == 4)
                    {
                        MaterialObject.color.x = ValueAsFloat(diffuseColor->array_data[0]);
                        MaterialObject.color.x = ValueAsFloat(diffuseColor->array_data[1]);
                        MaterialObject.color.x = ValueAsFloat(diffuseColor->array_data[2]);
                        MaterialObject.color.w = ValueAsFloat(diffuseColor->array_data[3]);
                    }

                    if (const tg3_value* specularColor = FindValueInObject(extension->value, "specularFactor"); specularColor != nullptr && specularColor->type == TG3_VALUE_ARRAY && specularColor->array_count == 3)
                    {
                        MaterialObject.specularColor.x = ValueAsFloat(specularColor->array_data[0]);
                        MaterialObject.specularColor.x = ValueAsFloat(specularColor->array_data[1]);
                        MaterialObject.specularColor.x = ValueAsFloat(specularColor->array_data[2]);
                        MaterialObject.specularColor.w = 1.0f;
                    }

                    if (const tg3_value* glossinessFactor = FindValueInObject(extension->value, "glossinessFactor"); glossinessFactor != nullptr && glossinessFactor->type == TG3_VALUE_REAL || glossinessFactor->type == TG3_VALUE_INT)
                    {
                        MaterialObject.roughness = 1.f - ValueAsFloat(*glossinessFactor);
                    }

                    if (const tg3_value* specularGlossinessTex = FindValueInObject(extension->value, "specularGlossinessTexture"); specularGlossinessTex != nullptr && specularGlossinessTex->type == TG3_VALUE_INT && specularGlossinessTex->int_val >= 0)
                    {
                        MaterialObject.specularTexture = static_cast<uint32_t>(specularGlossinessTex->int_val); 
                    }
                }
            }
        }

        // 3 - Meshes
        {
            for (size_t i = 0; i < model->meshes_count; ++i)
            {
                const auto& mesh = model->meshes[i];
                scene.meshes.emplace_back();
                MeshObject& MeshObject = *(scene.meshes.end() - 1);

                const int MainPrimitiveType = mesh.primitives[0].mode;
                switch (MainPrimitiveType)
                {
                case TG3_MODE_POINTS:          MeshObject.BeginMesh(Mesh::VertexType::POINTS); break;
                case TG3_MODE_LINE:            MeshObject.BeginMesh(Mesh::VertexType::LINES); break;
                case TG3_MODE_LINE_LOOP:       MeshObject.BeginMesh(Mesh::VertexType::LINE_LOOP); break;
                case TG3_MODE_LINE_STRIP:      MeshObject.BeginMesh(Mesh::VertexType::LINE_STRIP); break;
                case TG3_MODE_TRIANGLES:       MeshObject.BeginMesh(Mesh::VertexType::TRIANGLES); break;
                case TG3_MODE_TRIANGLE_STRIP:  MeshObject.BeginMesh(Mesh::VertexType::TRIANGLE_STRIP); break;
                case TG3_MODE_TRIANGLE_FAN:    MeshObject.BeginMesh(Mesh::VertexType::TRIANGLE_FAN); break;
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
                    size_t Count;
                };
                
                // states
                std::vector<BufferDataView> Indexes;
                std::vector<BufferDataView> Positions;
                std::vector<BufferDataView> Normals;
                std::vector<BufferDataView> TextCoords;
                size_t PreviousVertexCount = 0;
                
                for (size_t j = 0; j < mesh.primitives_count; ++j)
                {
                    const auto & primitive = mesh.primitives[j];
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
                            [](const tg3_accessor& Accessor)
                            {
                                switch(Accessor.component_type)
                                {
                                case TG3_COMPONENT_TYPE_BYTE:
                                case TG3_COMPONENT_TYPE_UNSIGNED_BYTE:
                                case TG3_COMPONENT_TYPE_SHORT:
                                case TG3_COMPONENT_TYPE_UNSIGNED_SHORT:
                                case TG3_COMPONENT_TYPE_INT:
                                case TG3_COMPONENT_TYPE_UNSIGNED_INT:
                                    return true;
                                default:
                                    EngineLoggerError("Unsupported index component type for vertex indexation.");
                                    return false;
                                }
                            },
                            CurrentView, CurrentVertexCount);

                        if(!success) continue;
                        
                        AssertOrErrorCall(Indexes.empty() || Indexes.front().TinyGLTFComponentType ==  model->accessors[primitive.indices].component_type, continue;, "Primitive index buffer type missmatch")
                        
                        Indexes.push_back(BufferDataView{.TinyGLTFComponentType = model->accessors[primitive.indices].component_type, .View = CurrentView, .Count = CurrentVertexCount});
                    }
                    
                    if (int AccessorIndex = FindAccessorInPrimitive(primitive, "POSITION"); AccessorIndex >= 0)
                    {
                        bool success = TryLoopOverBufferFromAccessor(model, AccessorIndex,
                            [](const tg3_accessor& Accessor)
                            {
                                AssertOrErrorCall(Accessor.type == TG3_TYPE_VEC3, return false;, "Unsupported primitive position component type. Position requires a vector 3.")
                                AssertOrErrorCall(Accessor.component_type == TG3_COMPONENT_TYPE_FLOAT || Accessor.component_type == TG3_COMPONENT_TYPE_DOUBLE, return false;,
                                    "Unsupported primitive position component type. Position vector needs to be made of float or double.")

                                return true;
                            },
                            CurrentView, IsIndexed ? Dummy : CurrentVertexCount);

                        if(!success) continue;
                        
                        AssertOrErrorCall(Positions.empty() || Positions.front().TinyGLTFComponentType ==  model->accessors[AccessorIndex].component_type, continue;, "Primitive buffer type missmatch")
                        
                        Positions.push_back(BufferDataView{.TinyGLTFComponentType = model->accessors[AccessorIndex].component_type, .View = CurrentView, .Count = IsIndexed ? Dummy : CurrentVertexCount});
                    FindBoundsInPositionBuffer(CurrentView, model->accessors[AccessorIndex].component_type, Min, Max);
                    }
                    if (int AccessorIndex = FindAccessorInPrimitive(primitive, "NORMAL"); AccessorIndex >= 0)
                    {
                        bool success = TryLoopOverBufferFromAccessor(model, AccessorIndex,
                            [](const tg3_accessor& Accessor)
                            {
                                AssertOrErrorCall(Accessor.type == TG3_TYPE_VEC3, return false;, 
                                    "Unsupported primitive normal component type. Normal requires a vector 3.")
                                AssertOrErrorCall(Accessor.component_type == TG3_COMPONENT_TYPE_FLOAT || Accessor.component_type == TG3_COMPONENT_TYPE_DOUBLE, return false;,
                                    "Unsupported primitive normal component type. Normal vector needs to be made of float or double.")

                                return true;
                            },
                            CurrentView, IsIndexed ? Dummy : CurrentVertexCount);

                        if(!success) continue;
                        
                        AssertOrErrorCall(Normals.empty() || Normals.front().TinyGLTFComponentType ==  model->accessors[AccessorIndex].component_type, continue;, "Primitive buffer type missmatch")
                        
                        Normals.push_back(BufferDataView{.TinyGLTFComponentType = model->accessors[AccessorIndex].component_type, .View = CurrentView, .Count = IsIndexed ? Dummy : CurrentVertexCount});
                    }
                    if (int AccessorIndex = FindAccessorInPrimitive(primitive, "TEXCOORD_0"); AccessorIndex >= 0)
                    {
                        bool success = TryLoopOverBufferFromAccessor(model, AccessorIndex,
                            [](const tg3_accessor& Accessor)
                            {
                                AssertOrErrorCall(Accessor.type == TG3_TYPE_VEC2, return false;, 
                                    "Unsupported primitive texture coordinates 0 component type. Texture coordinates 0 requires a vector 2.")
                                AssertOrErrorCall(Accessor.component_type == TG3_COMPONENT_TYPE_FLOAT || Accessor.component_type == TG3_COMPONENT_TYPE_DOUBLE, return false;,
                                    "Unsupported primitive texture coordinates 0 component type. Texture coordinates 0 needs to be made of float or double.")

                                return true;
                            },
                            CurrentView, IsIndexed ? Dummy : CurrentVertexCount);

                        if(!success) continue;
                        
                        AssertOrErrorCall(TextCoords.empty() || TextCoords.front().TinyGLTFComponentType ==  model->accessors[AccessorIndex].component_type, continue;, "Primitive buffer type missmatch")
                        
                        TextCoords.push_back(BufferDataView{.TinyGLTFComponentType = model->accessors[AccessorIndex].component_type, .View = CurrentView, .Count = IsIndexed ? Dummy : CurrentVertexCount});
                    }
                    
                    // todo vertex color
                    MeshObject.AddVertexGroup({(unsigned int)PreviousVertexCount, (unsigned int)CurrentVertexCount - (unsigned int)PreviousVertexCount, Min, Max});
                }
                
                if (!Indexes.empty())
                {
                    IndexBuffer::IndexType Type;
                    switch(Indexes.front().TinyGLTFComponentType)
                    {
                    case TG3_COMPONENT_TYPE_BYTE:          Type = IndexBuffer::Byte; break;
                    case TG3_COMPONENT_TYPE_UNSIGNED_BYTE: Type = IndexBuffer::UnsignedByte; break;
                    case TG3_COMPONENT_TYPE_SHORT:         Type = IndexBuffer::Short; break;
                    case TG3_COMPONENT_TYPE_UNSIGNED_SHORT:Type = IndexBuffer::UnsignedShort; break;
                    case TG3_COMPONENT_TYPE_INT:           Type = IndexBuffer::Int; break;
                    case TG3_COMPONENT_TYPE_UNSIGNED_INT:  Type = IndexBuffer::UnsignedInt; break;
                            
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
                    case TG3_COMPONENT_TYPE_FLOAT: Layout.Push<Math::Point3f>(1); break;
                    case TG3_COMPONENT_TYPE_DOUBLE:  Layout.Push<Math::Point3d>(1); break;
                    
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
                    case TG3_COMPONENT_TYPE_FLOAT: Layout.Push<Math::Vector3f>(1); break;
                    case TG3_COMPONENT_TYPE_DOUBLE:  Layout.Push<Math::Vector3d>(1); break;
                    
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
                    case TG3_COMPONENT_TYPE_FLOAT: Layout.Push<Math::Vector2f>(1); break;
                    case TG3_COMPONENT_TYPE_DOUBLE:  Layout.Push<Math::Vector2d>(1); break;
                    
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

        }

        // 4 - Scene tree
        {
            uint32_t sceneIndex = 0;
            AssertOrErrorCall(model->scenes_count > sceneIndex, return false, "No scene found")

            for (int i = 0; i < model->scenes[sceneIndex].nodes_count; i++)
                LoadSceneTree(model, model->scenes[sceneIndex].nodes[i], scene);
        }
        
        return true;
        // model->textures->

    on_failed_tg3_parse:
        for (uint32_t i = 0; i < errors.count(); i++)
        {
            EngineLoggerErrorF("[%d] %s", (int)errors.entry(i)->severity,  errors.entry(i)->message ? errors.entry(i)->message : "(null)");
        }
        goto on_failed_final;
        
    on_failed_final:
        return false;
    }
}
#endif // USE_TINY_GLTF_3
