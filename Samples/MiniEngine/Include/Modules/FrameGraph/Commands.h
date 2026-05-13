#pragma once

#include <span>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include <type_traits>

#include "Camera/Camera.h"
#include "Importers/GLTF/SceneLoader.h"
#include "Rendering/VertexBuffer.h"
#include "Rendering/IndexBuffer.h"
#include "Rendering/VertexArrayObject.h"
#include "Rendering/MeshObject.h"
#include "Rendering/StorageBuffer.h"
#include "Rendering/Textures.h"
#include "Rendering/UniformBuffer.h"

#include "Modules/Rendering/Shaders/Camera.h"

#include "Types.h"

namespace FrameGraph
{
    namespace _Graph
    {
        template<typename T>
        class ObjectPool
        {
        public:
            ObjectPool() = default;
            
            FrameGraph::Location GetLocation(FrameGraph::Name Name) const
            {
                return m_NamesToObject.at(Name);
            }
            
            FrameGraph::Location GetLocation(std::string_view Name) const
            {
                return m_NamesToObject.at(FrameGraph::ToName(Name));
            }
            
            template <typename ...Args>
            FrameGraph::Location Add(std::string_view Name, Args&& ...Params)
            {                
                FrameGraph::Name ID = FrameGraph::ToName(Name);
                if (m_NamesToObject.contains(ID)) return m_NamesToObject[ID];
                
                FrameGraph::Location Index;
                if (m_FreeCount > 0)
                {
                    for (Index = 0; Index < m_Resources.size(); ++Index)
                    {
                        if (m_Resources[Index].has_value() == false) break;
                    }
                    
                    m_Resources[Index].emplace(ID, std::forward<Args>(Params)...);
                    m_FreeCount--;
                }
                else
                {
                    Index = m_Resources.size();
                    m_Resources.emplace_back();
                    m_Resources.back().emplace(ID, std::forward<Args>(Params)...);
                }
                
                m_NamesToObject[ID] = Index;
                
                return Index;
            }
            
            T& operator[](FrameGraph::Location Index) { return (*m_Resources[Index]).Object; }
            const T& operator[](FrameGraph::Location Index) const { return (*m_Resources[Index]).Object; }
            
            T& Get(FrameGraph::Name Name) { return (*m_Resources[m_NamesToObject[Name]]).Object; }
            const T& Get(FrameGraph::Name Name) const { return (*m_Resources[m_NamesToObject.at(Name)]).Object; }
            
            void RemoveAt(FrameGraph::Location Index)
            {
                m_NamesToObject.erase(m_Resources[Index].Name);
                m_Resources[Index].reset();
                m_FreeCount++;
            }
            void Remove(FrameGraph::Name Name) 
            { 
                FrameGraph::Location Index = m_NamesToObject.at(Name);
                RemoveAt(Index);
            }
            
        private:
            struct Element
            {
                T Object;
                FrameGraph::Name Name;

                template <typename ...Args>
                Element(FrameGraph::Name name, Args... Params): Object(std::forward<Args>(Params)...), Name(name) {}
                // Element(const Element& Other): Object(Other.Object), Name(Other.Name) {}
                Element(Element&& Other) noexcept: Object(std::move(Other.Object)), Name(Other.Name) {}
                ~Element() = default;

                // Element& operator=(const Element& Other)
                // {
                //     if (this == &Other)
                //         return *this;
                //     
                //     Object = Other.Object;
                //     Name = Other.Name;
                //     
                //     return *this;
                // }

                Element& operator=(Element&& Other) noexcept
                {
                    if (this == &Other)
                        return *this;
                    
                    Object = std::move(Other.Object);
                    Name = Other.Name;
                    
                    return *this;
                }
            };
            
            std::map<FrameGraph::Name, FrameGraph::Location> m_NamesToObject;
            std::vector<std::optional<Element>> m_Resources;
            uint64_t m_FreeCount = 0;
        };
        
        template <typename T>
        class Variable
        {
        public:            
            Variable(const T& Base) : Value(Base) {}
            ~Variable() = default;
            
            void Set(const T& value) { Value = value; m_HasChanged = true; }
            const T& Get() const { return Value; }
            bool HasChanged() const { return m_HasChanged; }
            void Reset() { m_HasChanged = false; }
            
        private:
            T Value;
            bool m_HasChanged = false;
        };
        
        template <typename T>
        class VariablePool
        {
        public:
            FrameGraph::Location Add(std::string_view Name, const T& BaseValue)
            {
                FrameGraph::Name ID = FrameGraph::ToName(Name);
                if (m_NamesToObject.contains(ID)) return m_NamesToObject[ID];
                
                FrameGraph::Location Index;
                if (m_FreeCount > 0)
                {
                    for (Index = 0; Index < m_Resources.size(); ++Index)
                    {
                        if (m_Resources[Index].has_value() == false) break;
                    }
                    
                    m_Resources[Index].emplace(ID, BaseValue);
                    m_FreeCount--;
                }
                else
                {
                    Index = m_Resources.size();
                    m_Resources.emplace_back();
                    m_Resources.back().emplace(ID, BaseValue);
                }
                
                m_NamesToObject[ID] = Index;
                
                return Index;
            }
            
            FrameGraph::Location GetLocation(FrameGraph::Name Name) const
            {
                return m_NamesToObject.at(Name);
            }
            
            FrameGraph::Location GetLocation(std::string_view Name) const
            {
                return m_NamesToObject.at(FrameGraph::ToName(Name));
            }
            
            void Set(FrameGraph::Location Index, const T& value)
            {
                m_Resources[Index]->Object.Set(value);
            }
            
            const T& Get(FrameGraph::Location Index) const
            {
                return m_Resources[Index]->Object.Get();
            }
            
            bool HasChanged(FrameGraph::Location Index) const
            {
                return m_Resources[Index]->Object.HasChanged();
            }
            
            void Reset()
            {
                for (auto& element : m_Resources)
                {
                    if (!element.has_value()) continue;
                    
                    element->Object.Reset();
                }
            }
            
        private:
            struct Element
            {
                Variable<T> Object;
                FrameGraph::Name Name;
                
                Element(FrameGraph::Name name, T Base): Object(Base), Name(name) {}
                Element(const Element& Other): Object(Other.Object), Name(Other.Name) {}
                Element(Element&& Other) noexcept: Object(std::move(Other.Object)), Name(Other.Name)  {}
                ~Element() = default;

                Element& operator=(const Element& Other)
                {
                    if (this == &Other)
                        return *this;
                    
                    Object = Other.Object;
                    Name = Other.Name;
                    
                    return *this;
                }

                Element& operator=(Element&& Other) noexcept
                {
                    if (this == &Other)
                        return *this;
                    
                    Object = std::move(Other.Object);
                    Name = Other.Name;
                    
                    return *this;
                }
            };
            
            std::map<FrameGraph::Name, FrameGraph::Location> m_NamesToObject;
            std::vector<std::optional<Element>> m_Resources;
            uint64_t m_FreeCount = 0;
        };
    }

    class CameraArray
    {
    public:
        friend class CommandContext;
        
        CameraArray() = default;
        ~CameraArray() = default;
        
        FrameGraph::Location AddCamera();
        FrameGraph::Location AddCamera(const Camera& camera);
        FrameGraph::Location AddCamera(const Rendering::CameraData& camera);
        void UpdateCamera(FrameGraph::Location Index, const Camera& camera);
        void UpdateCamera(FrameGraph::Location Index, const Rendering::CameraData& camera);
        
        INLINE GLuint Handle() const { return m_GPUArray.Handle(); }
        INLINE const StorageBuffer& Buffer() const { return m_GPUArray.Buffer(); }
        
        INLINE size_t Size() const { return m_GPUArray.Size(); }
        INLINE size_t SizeInBytes() const { return m_GPUArray.SizeInBytes(); }
        
        const Rendering::CameraData& GetCamera(FrameGraph::Location Index) const {return m_CPUArray[Index];}
        const Rendering::CameraData& GetMainCameraData() const {return m_CPUArray[m_MainCamera];}
        FrameGraph::Location GetMainCamera() const {return m_MainCamera;}
        void SetMainCamera(FrameGraph::Location Index) {m_MainCamera = Index;}
        
    private:
        void Update()
        {
            if (!m_HasChanged) return;
            
            m_GPUArray.UpdateCameras(m_CPUArray);
            
            m_HasChanged = false;
        }
        
        std::vector<bool> m_Validity;
        std::vector<Rendering::CameraData> m_CPUArray;
        Rendering::CameraArray m_GPUArray;
        size_t m_FreeCount = 0;
        FrameGraph::Location m_MainCamera = 0;
        bool m_HasChanged = false;
    };

    class CommandContext
    {
    public:
        friend class CommandPool;
        
        CommandContext() = default;
        ~CommandContext() = default;
        CommandContext(const CommandContext& Other) = delete;
        CommandContext(CommandContext&& Other) noexcept = delete;
        CommandContext& operator=(const CommandContext& Other) = delete;
        CommandContext& operator=(CommandContext&& Other) noexcept = delete;

        template<typename T, typename ...Args> FrameGraph::Location Add(std::string_view Name, Args ...Params) {static_assert(sizeof(T) == 0, "Unsupported object type or parameters");}
        template<typename T> T& Get(FrameGraph::Location Index) {return ObjectList<T>()[Index];}
        template<typename T> const T& Get(FrameGraph::Location Index) const {return ObjectList<T>()[Index];}
        template<typename T> T& Get(std::string_view Name) {return ObjectList<T>().Get(FrameGraph::ToName(Name));}
        template<typename T> const T& Get(std::string_view Name) const {return ObjectList<T>().Get(FrameGraph::ToName(Name));}
        
        template<typename T, typename ...Args> FrameGraph::Location AddVariable(std::string_view Name, const T& BaseValue, Args ...Params) {static_assert(sizeof(T) == 0, "Unsupported object type or parameters");}
        template<typename T> const T& GetValue(FrameGraph::Location Index) const {return VariableList<T>().Get(Index);}
        template<typename T> const T& GetValue(std::string_view Name) const {return VariableList<T>().Get(VariableList<T>().GetLocation(Name));}
        
        template<typename T> void SetValue(FrameGraph::Location Index, const T& Value) {VariableList<T>().Set(Index, Value);}
        template<typename T> void SetValue(std::string_view Name, const T& Value) {VariableList<T>().Set(VariableList<T>().GetLocation(Name), Value);}
        
        template<typename T> bool HasChanged(FrameGraph::Location Index) const {return VariableList<T>().HasChanged(Index);}
        template<typename T> bool HasChanged(std::string_view Name) const {return VariableList<T>().HasChanged(VariableList<T>().GetLocation(Name));}
        
        template<typename T> FrameGraph::Location GetLocation(FrameGraph::Name Name) const {static_assert(sizeof(T) == 0, "Unsupported object type");}
        template<typename T> INLINE FrameGraph::Location GetLocation(std::string_view Name) const {return GetLocation<T>(FrameGraph::ToName(Name));}

        INLINE Location AddCamera()                                                 {return m_CameraArray.AddCamera();}
        INLINE Location AddCamera(const Camera& camera)                             {return m_CameraArray.AddCamera(camera);}
        INLINE Location AddCamera(const Rendering::CameraData& camera)                         {return m_CameraArray.AddCamera(camera);}
        INLINE void UpdateCamera(FrameGraph::Location Index, const Camera& camera)              {m_CameraArray.UpdateCamera(Index, camera);}
        INLINE void UpdateCamera(FrameGraph::Location Index, const Rendering::CameraData& camera)          {m_CameraArray.UpdateCamera(Index, camera);}
        
        INLINE GLuint Handle() const                                                {return m_CameraArray.Handle();}
        INLINE const StorageBuffer& Buffer() const                                  {return m_CameraArray.Buffer();}
        
        INLINE size_t Size() const                                                  {return m_CameraArray.Size(); }
        INLINE size_t SizeInBytes() const                                           {return m_CameraArray.SizeInBytes(); }
        
        INLINE const Rendering::CameraData& GetCamera(FrameGraph::Location Index) const                    {return m_CameraArray.GetCamera(Index);}
        INLINE const Rendering::CameraData& GetMainCameraData() const                          {return m_CameraArray.GetMainCameraData();}
        INLINE FrameGraph::Location GetMainCamera() const                                       {return m_CameraArray.GetMainCamera();}
        INLINE void SetMainCamera(FrameGraph::Location Index)                                   {m_CameraArray.SetMainCamera(Index);}
        INLINE const StorageBuffer& GetCameraBuffer() const                         {return m_CameraArray.Buffer();}
        
        // TODO move to use the Mini Engine Scene instead
        INLINE const GLTF::GPUScene& Scene() const {return m_SceneTree;}
        INLINE GLTF::GPUScene& Scene() {return m_SceneTree;}
        
    private:
        template <typename T> _Graph::ObjectPool<T>& ObjectList() {static_assert(sizeof(T) == 0, "Unsupported object type");}        
        template <typename T> const _Graph::ObjectPool<T>& ObjectList() const {static_assert(sizeof(T) == 0, "Unsupported object type");}
        template <typename T> _Graph::VariablePool<T>& VariableList() {static_assert(sizeof(T) == 0, "Unsupported variable type");}  
        template <typename T> const _Graph::VariablePool<T>& VariableList() const {static_assert(sizeof(T) == 0, "Unsupported variable type");}  
        
        void UpdateCameras();
        void ConsumeVariableChanges();
        
        // Graph managed objects
        _Graph::ObjectPool<VertexBuffer>        m_VertexBuffers;
        _Graph::ObjectPool<IndexBuffer>         m_IndexBuffers;
        _Graph::ObjectPool<VertexArrayObject>   m_VAOs;
        _Graph::ObjectPool<UniformBuffer>       m_UniformBuffers;
        _Graph::ObjectPool<StorageBuffer>       m_StorageBuffers;
        // _Graph::ObjectPool<MeshObject>          m_MeshObjects; // Cannot copy Modeling::Mesh
        _Graph::ObjectPool<Texture2D>           m_Textures2D;
        _Graph::ObjectPool<Texture2DArray>      m_Textures2DArray;
        _Graph::ObjectPool<Texture3D>           m_Textures3D;
        _Graph::ObjectPool<TextureCube>         m_TexturesCube;
        // _Graph::ObjectPool<TextureCubeView>     m_TexturesCubeViews; // Somehow instances an undersired copy from the texture cube object given in paramerters
        
        // Graph variables
        _Graph::VariablePool<FrameGraph::Bool>      m_BoolVariables;
        _Graph::VariablePool<FrameGraph::UInt>      m_UIntVariables;
        _Graph::VariablePool<FrameGraph::Int>       m_IntVariables;
        _Graph::VariablePool<FrameGraph::Float>     m_FloatVariables;
        _Graph::VariablePool<FrameGraph::Size2D>    m_Size2DVariables;
        _Graph::VariablePool<FrameGraph::Rect>      m_RectVariables;
        _Graph::VariablePool<Math::Vector3f>        m_Vec3Variables;

        // Scene
        FrameGraph::CameraArray m_CameraArray;
        GLTF::GPUScene m_SceneTree;
    };

    template<> FrameGraph::Location CommandContext::Add<VertexBuffer>(std::string_view Name);

    template<> FrameGraph::Location CommandContext::Add<IndexBuffer>(std::string_view Name);

    template<> FrameGraph::Location CommandContext::Add<VertexArrayObject>(std::string_view Name);

    template<> FrameGraph::Location CommandContext::Add<UniformBuffer>(std::string_view Name);
    template<> FrameGraph::Location CommandContext::Add<UniformBuffer>(std::string_view Name, uint32_t Size);
    template<> FrameGraph::Location CommandContext::Add<UniformBuffer>(std::string_view Name, uint32_t Size, const void* data);

    template<> FrameGraph::Location CommandContext::Add<StorageBuffer>(std::string_view Name);
    template<> FrameGraph::Location CommandContext::Add<StorageBuffer>(std::string_view Name, uint32_t Size);
    template<> FrameGraph::Location CommandContext::Add<StorageBuffer>(std::string_view Name, uint32_t Size, const void* data);

    // template<> FrameGraph::Location CommandContext::Add<MeshObject>(std::string_view Name);
    // template<> FrameGraph::Location CommandContext::Add<MeshObject>(std::string_view Name, const Mesh& Mesh);

    template<> FrameGraph::Location CommandContext::Add<Texture2D>(std::string_view Name, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout);
    template<> FrameGraph::Location CommandContext::Add<Texture2D>(std::string_view Name, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout, uint8_t SampleCount);
    template<> FrameGraph::Location CommandContext::Add<Texture2D>(std::string_view Name, const Image& Image);
    template<> FrameGraph::Location CommandContext::Add<Texture2D>(std::string_view Name, const Image& Image, bool UseMips);
    template<> FrameGraph::Location CommandContext::Add<Texture2D>(std::string_view Name, uint32_t width, uint32_t height, Image::Type type, Image::Layout layout, const void* ImageData, size_t ImageSize);
    template<> FrameGraph::Location CommandContext::Add<Texture2D>(std::string_view Name, uint32_t width, uint32_t height, Image::Type type, Image::Layout layout, const void* ImageData, size_t ImageSize, bool UseMips);

    template<> FrameGraph::Location CommandContext::Add<Texture2DArray>(std::string_view Name, uint32_t width, uint32_t height, uint32_t count, Texture::Type type, Texture::Layout layout);
    template<> FrameGraph::Location CommandContext::Add<Texture2DArray>(std::string_view Name, uint32_t width, uint32_t height, uint32_t count, Texture::Type type, Texture::Layout layout, bool UseMips);

    template<> FrameGraph::Location CommandContext::Add<Texture3D>(std::string_view Name, uint32_t width, uint32_t height, uint32_t depth, Texture::Type type, Texture::Layout layout);
    template<> FrameGraph::Location CommandContext::Add<Texture3D>(std::string_view Name, const ImageCube& Image);
    template<> FrameGraph::Location CommandContext::Add<Texture3D>(std::string_view Name, const ImageCube& Image, bool UseMips);

    template<> FrameGraph::Location CommandContext::Add<TextureCube>(std::string_view Name, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout);
    template<> FrameGraph::Location CommandContext::Add<TextureCube>(std::string_view Name, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout, bool UseMips);
    template<> FrameGraph::Location CommandContext::Add<TextureCube>(std::string_view Name, std::span<const TextureCube::FacePair> Faces);
    template<> FrameGraph::Location CommandContext::Add<TextureCube>(std::string_view Name, std::span<const TextureCube::FacePair> Faces, bool UseMips);

    // template<> FrameGraph::Location CommandContext::Add<TextureCubeView>(std::string_view Name, const TextureCube& texture, uint32_t MipLevel);
    // template<> FrameGraph::Location CommandContext::Add<TextureCubeView>(std::string_view Name, const TextureCube& texture, uint32_t MipLevel, uint32_t MipCount); 
    // template<> FrameGraph::Location CommandContext::Add<TextureCubeView>(std::string_view Name, std::string_view texture, uint32_t MipLevel);
    // template<> FrameGraph::Location CommandContext::Add<TextureCubeView>(std::string_view Name, std::string_view texture, uint32_t MipLevel, uint32_t MipCount);
    // template<> FrameGraph::Location CommandContext::Add<TextureCubeView>(std::string_view Name, FrameGraph::Location texture, uint32_t MipLevel);
    // template<> FrameGraph::Location CommandContext::Add<TextureCubeView>(std::string_view Name, FrameGraph::Location texture, uint32_t MipLevel, uint32_t MipCount);

    template<> FrameGraph::Location CommandContext::AddVariable<FrameGraph::Bool>(std::string_view Name, const FrameGraph::Bool& BaseValue);
    template<> FrameGraph::Location CommandContext::AddVariable<FrameGraph::UInt>(std::string_view Name, const FrameGraph::UInt& BaseValue);
    template<> FrameGraph::Location CommandContext::AddVariable<FrameGraph::Int>(std::string_view Name, const FrameGraph::Int& BaseValue);
    template<> FrameGraph::Location CommandContext::AddVariable<FrameGraph::Float>(std::string_view Name, const FrameGraph::Float& BaseValue);
    template<> FrameGraph::Location CommandContext::AddVariable<FrameGraph::Size2D>(std::string_view Name, const FrameGraph::Size2D& BaseValue);
    template<> FrameGraph::Location CommandContext::AddVariable<FrameGraph::Rect>(std::string_view Name, const FrameGraph::Rect& BaseValue);
    template<> FrameGraph::Location CommandContext::AddVariable<Math::Vector3f>(std::string_view Name, const Math::Vector3f& BaseValue);

    template<> INLINE _Graph::ObjectPool<VertexBuffer>&            CommandContext::ObjectList<VertexBuffer>()      {return m_VertexBuffers;}
    template<> INLINE _Graph::ObjectPool<IndexBuffer>&             CommandContext::ObjectList<IndexBuffer>()       {return m_IndexBuffers;}
    template<> INLINE _Graph::ObjectPool<VertexArrayObject>&       CommandContext::ObjectList<VertexArrayObject>() {return m_VAOs;}
    template<> INLINE _Graph::ObjectPool<UniformBuffer>&           CommandContext::ObjectList<UniformBuffer>()     {return m_UniformBuffers;}
    template<> INLINE _Graph::ObjectPool<StorageBuffer>&           CommandContext::ObjectList<StorageBuffer>()     {return m_StorageBuffers;}
    // template<> INLINE _Graph::ObjectPool<MeshObject>&              CommandContext::ObjectList<MeshObject>()        {return m_MeshObjects;}
    template<> INLINE _Graph::ObjectPool<Texture2D>&               CommandContext::ObjectList<Texture2D>()         {return m_Textures2D;}
    template<> INLINE _Graph::ObjectPool<Texture2DArray>&          CommandContext::ObjectList<Texture2DArray>()    {return m_Textures2DArray;}
    template<> INLINE _Graph::ObjectPool<Texture3D>&               CommandContext::ObjectList<Texture3D>()         {return m_Textures3D;}
    template<> INLINE _Graph::ObjectPool<TextureCube>&             CommandContext::ObjectList<TextureCube>()       {return m_TexturesCube;}
    // template<> INLINE _Graph::ObjectPool<TextureCubeView>&         CommandContext::ObjectList<TextureCubeView>()   {return m_TexturesCubeViews;}

    template<> INLINE const _Graph::ObjectPool<VertexBuffer>&      CommandContext::ObjectList<VertexBuffer>()      const {return m_VertexBuffers;}
    template<> INLINE const _Graph::ObjectPool<IndexBuffer>&       CommandContext::ObjectList<IndexBuffer>()       const {return m_IndexBuffers;}
    template<> INLINE const _Graph::ObjectPool<VertexArrayObject>& CommandContext::ObjectList<VertexArrayObject>() const {return m_VAOs;}
    template<> INLINE const _Graph::ObjectPool<UniformBuffer>&     CommandContext::ObjectList<UniformBuffer>()     const {return m_UniformBuffers;}
    template<> INLINE const _Graph::ObjectPool<StorageBuffer>&     CommandContext::ObjectList<StorageBuffer>()     const {return m_StorageBuffers;}
    // template<> INLINE const _Graph::ObjectPool<MeshObject>&        CommandContext::ObjectList<MeshObject>()        const {return m_MeshObjects;}
    template<> INLINE const _Graph::ObjectPool<Texture2D>&         CommandContext::ObjectList<Texture2D>()         const {return m_Textures2D;}
    template<> INLINE const _Graph::ObjectPool<Texture2DArray>&    CommandContext::ObjectList<Texture2DArray>()    const {return m_Textures2DArray;}
    template<> INLINE const _Graph::ObjectPool<Texture3D>&         CommandContext::ObjectList<Texture3D>()         const {return m_Textures3D;}
    template<> INLINE const _Graph::ObjectPool<TextureCube>&       CommandContext::ObjectList<TextureCube>()       const {return m_TexturesCube;}
    // template<> INLINE const _Graph::ObjectPool<TextureCubeView>&   CommandContext::ObjectList<TextureCubeView>()   const {return m_TexturesCubeViews;}

    template<> INLINE _Graph::VariablePool<FrameGraph::Bool>&               CommandContext::VariableList<FrameGraph::Bool>()            {return m_BoolVariables;}
    template<> INLINE _Graph::VariablePool<FrameGraph::UInt>&               CommandContext::VariableList<FrameGraph::UInt>()            {return m_UIntVariables;}
    template<> INLINE _Graph::VariablePool<FrameGraph::Int>&                CommandContext::VariableList<FrameGraph::Int>()             {return m_IntVariables;}
    template<> INLINE _Graph::VariablePool<FrameGraph::Float>&              CommandContext::VariableList<FrameGraph::Float>()           {return m_FloatVariables;}
    template<> INLINE _Graph::VariablePool<FrameGraph::Size2D>&             CommandContext::VariableList<FrameGraph::Size2D>()          {return m_Size2DVariables;}
    template<> INLINE _Graph::VariablePool<FrameGraph::Rect>&               CommandContext::VariableList<FrameGraph::Rect>()            {return m_RectVariables;}
    template<> INLINE _Graph::VariablePool<Math::Vector3f>&         CommandContext::VariableList<Math::Vector3f>()  {return m_Vec3Variables;}

    template<> INLINE const _Graph::VariablePool<FrameGraph::Bool>&         CommandContext::VariableList<FrameGraph::Bool>()            const {return m_BoolVariables;}
    template<> INLINE const _Graph::VariablePool<FrameGraph::UInt>&         CommandContext::VariableList<FrameGraph::UInt>()            const {return m_UIntVariables;}
    template<> INLINE const _Graph::VariablePool<FrameGraph::Int>&          CommandContext::VariableList<FrameGraph::Int>()             const {return m_IntVariables;}
    template<> INLINE const _Graph::VariablePool<FrameGraph::Float>&        CommandContext::VariableList<FrameGraph::Float>()           const {return m_FloatVariables;}
    template<> INLINE const _Graph::VariablePool<FrameGraph::Size2D>&       CommandContext::VariableList<FrameGraph::Size2D>()          const {return m_Size2DVariables;}
    template<> INLINE const _Graph::VariablePool<FrameGraph::Rect>&         CommandContext::VariableList<FrameGraph::Rect>()            const {return m_RectVariables;}
    template<> INLINE const _Graph::VariablePool<Math::Vector3f>&   CommandContext::VariableList<Math::Vector3f>()  const {return m_Vec3Variables;}

    template<> INLINE FrameGraph::Location CommandContext::GetLocation<VertexBuffer>(FrameGraph::Name Name) const      {return ObjectList<VertexBuffer>().GetLocation(Name);}
    template<> INLINE FrameGraph::Location CommandContext::GetLocation<IndexBuffer>(FrameGraph::Name Name) const       {return ObjectList<IndexBuffer>().GetLocation(Name);}
    template<> INLINE FrameGraph::Location CommandContext::GetLocation<VertexArrayObject>(FrameGraph::Name Name) const {return ObjectList<VertexArrayObject>().GetLocation(Name);}
    template<> INLINE FrameGraph::Location CommandContext::GetLocation<UniformBuffer>(FrameGraph::Name Name) const     {return ObjectList<UniformBuffer>().GetLocation(Name);}
    template<> INLINE FrameGraph::Location CommandContext::GetLocation<StorageBuffer>(FrameGraph::Name Name) const     {return ObjectList<StorageBuffer>().GetLocation(Name);}
    // template<> INLINE FrameGraph::Location CommandContext::GetLocation<MeshObject>(FrameGraph::Name Name) const        {return ObjectList<MeshObject>().GetLocation(Name);}
    template<> INLINE FrameGraph::Location CommandContext::GetLocation<Texture2D>(FrameGraph::Name Name) const         {return ObjectList<Texture2D>().GetLocation(Name);}
    template<> INLINE FrameGraph::Location CommandContext::GetLocation<Texture2DArray>(FrameGraph::Name Name) const    {return ObjectList<Texture2DArray>().GetLocation(Name);}
    template<> INLINE FrameGraph::Location CommandContext::GetLocation<Texture3D>(FrameGraph::Name Name) const         {return ObjectList<Texture3D>().GetLocation(Name);}
    template<> INLINE FrameGraph::Location CommandContext::GetLocation<TextureCube>(FrameGraph::Name Name) const       {return ObjectList<TextureCube>().GetLocation(Name);}
    // template<> INLINE FrameGraph::Location CommandContext::GetLocation<TextureCubeView>(FrameGraph::Name Name) const   {return ObjectList<TextureCubeView>().GetLocation(Name);}
    template<> INLINE FrameGraph::Location CommandContext::GetLocation<FrameGraph::Bool>(FrameGraph::Name Name) const              {return VariableList<FrameGraph::Bool>().GetLocation(Name);}
    template<> INLINE FrameGraph::Location CommandContext::GetLocation<FrameGraph::UInt>(FrameGraph::Name Name) const              {return VariableList<FrameGraph::UInt>().GetLocation(Name);}
    template<> INLINE FrameGraph::Location CommandContext::GetLocation<FrameGraph::Int>(FrameGraph::Name Name) const               {return VariableList<FrameGraph::Int>().GetLocation(Name);}
    template<> INLINE FrameGraph::Location CommandContext::GetLocation<FrameGraph::Float>(FrameGraph::Name Name) const             {return VariableList<FrameGraph::Float>().GetLocation(Name);}
    template<> INLINE FrameGraph::Location CommandContext::GetLocation<FrameGraph::Size2D>(FrameGraph::Name Name) const            {return VariableList<FrameGraph::Size2D>().GetLocation(Name);}
    template<> INLINE FrameGraph::Location CommandContext::GetLocation<FrameGraph::Rect>(FrameGraph::Name Name) const              {return VariableList<FrameGraph::Rect>().GetLocation(Name);}
    template<> INLINE FrameGraph::Location CommandContext::GetLocation<Math::Vector3f>(FrameGraph::Name Name) const    {return VariableList<Math::Vector3f>().GetLocation(Name);}

    class ICommand
    {
    public:
        friend class CommandPool;
        
        virtual ~ICommand() = default;
        
    protected:
        ICommand(CommandContext& Resources) {}
        
        virtual void OnReloadShaders(CommandContext& Resources) = 0;
        virtual void OnUpdate(CommandContext& Resources, double DeltaTime) = 0;
        virtual void OnExecute(const CommandContext& Resources) = 0;
    };

    class CommandList
    {
    public:
        void Clear() {m_List.clear();}

        void Add(Location Command) {m_List.push_back(Command);}

        void Reserve(size_t Count) {m_List.reserve(Count);}

        std::span<const Location> Data() const {return m_List;}

    private:
        std::vector<Location> m_List;
    };

    class CommandPool
    {
    public:
        CommandPool() = default;
        ~CommandPool() = default;
        CommandPool(const CommandPool& Other) = delete;
        CommandPool(CommandPool&& Other) noexcept = delete;
        CommandPool& operator=(const CommandPool& Other) = delete;
        CommandPool& operator=(CommandPool&& Other) noexcept = delete;

        // Used to add, set or get variables
        CommandContext& Context() {return m_Context;}
        
        template<typename T> requires (std::is_base_of_v<ICommand, T>)
        Location PushNode()
        {
            Location Index = m_Commands.size();
            m_Commands.emplace_back(ctti::nameof<T>(), std::make_unique<T>(m_Context));
            return Index;
        }
        
        void ReloadShaders();
        void Update(double DeltaTime);
        void Render(std::span<const Location> CommandList) const;
        INLINE void Render(const CommandList& CommandList) const {Render(CommandList.Data());}
        
    private:
        struct CommandInst
        {
            ctti::detail::cstring name;
            std::unique_ptr<ICommand> node;
        };
        
        CommandContext m_Context;
        std::vector<CommandInst> m_Commands;
    };
}