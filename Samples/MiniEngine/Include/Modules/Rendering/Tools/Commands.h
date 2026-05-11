#pragma once

#include <map>
#include <string_view>
#include <utility>
#include <vector>

#include "ctti/nameof.hpp"
#include "Importers/GLTF/SceneLoader.h"
#include "Modules/Rendering/Shaders/Camera.h"
#include "Shared/Annotations.h"

#include "Rendering/VertexBuffer.h"
#include "Rendering/IndexBuffer.h"
#include "Rendering/VertexArrayObject.h"
#include "Rendering/MeshObject.h"
#include "Rendering/StorageBuffer.h"
#include "Rendering/Textures.h"
#include "Rendering/UniformBuffer.h"

namespace Rendering::Graph
{
    using Name = std::uint64_t;
    using Location = size_t;
    using Bool = bool;
    using UInt = uint32_t;
    using Int = int;
    using Float = float;
    using Size2D = Math::Vector2t<uint32_t>;;
    struct Rect {Math::Vector2t<uint32_t> Position; Math::Vector2t<uint32_t> Size;};

    constexpr UInt kEAntiAliasingNone = 0;
    constexpr UInt kEAntiAliasingMSAA = 1;
    
    INLINE Name ToName(std::string_view Name) {return std::hash<std::string_view>{}(Name);}

    namespace _Graph
    {
        template<typename T>
        class ObjectPool
        {
        public:
            ObjectPool() = default;
            
            Location GetLocation(Name name) const
            {
                return m_NamesToObject.at(name);
            }
            
            Location GetLocation(std::string_view Name) const
            {
                return m_NamesToObject.at(ToName(Name));
            }
            
            template <typename ...Args>
            Location Add(std::string_view Name, Args&& ...Params)
            {                
                Graph::Name ID = ToName(Name);
                if (m_NamesToObject.contains(ID)) return m_NamesToObject[ID];
                
                Location Index;
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
            
            T& operator[](Location Index) { return (*m_Resources[Index]).Object; }
            const T& operator[](Location Index) const { return (*m_Resources[Index]).Object; }
            
            T& Get(Name name) { return (*m_Resources[m_NamesToObject[name]]).Object; }
            const T& Get(Name name) const { return (*m_Resources[m_NamesToObject.at(name)]).Object; }
            
            void RemoveAt(Location Index)
            {
                m_NamesToObject.erase(m_Resources[Index].Name);
                m_Resources[Index].reset();
                m_FreeCount++;
            }
            void Remove(Name name) 
            { 
                Location Index = m_NamesToObject.at(name);
                RemoveAt(Index);
            }
            
        private:
            struct Element
            {
                T Object;
                Name Name;

                template <typename ...Args>
                Element(Graph::Name name, Args... Params): Object(std::forward<Args>(Params)...), Name(name) {}
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
            
            std::map<Name, Location> m_NamesToObject;
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
            Location Add(std::string_view Name, const T& BaseValue)
            {
                Graph::Name ID = ToName(Name);
                if (m_NamesToObject.contains(ID)) return m_NamesToObject[ID];
                
                Location Index;
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
            
            Location GetLocation(Name name) const
            {
                return m_NamesToObject.at(name);
            }
            
            Location GetLocation(std::string_view Name) const
            {
                return m_NamesToObject.at(ToName(Name));
            }
            
            void Set(Location Index, const T& value)
            {
                m_Resources[Index]->Object.Set(value);
            }
            
            const T& Get(Location Index) const
            {
                return m_Resources[Index]->Object.Get();
            }
            
            bool HasChanged(Location Index) const
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
                Name Name;
                
                Element(Graph::Name name, T Base): Object(Base), Name(name) {}
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
            
            std::map<Name, Location> m_NamesToObject;
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
        
        Location AddCamera();
        Location AddCamera(const Camera& camera);
        Location AddCamera(const CameraData& camera);
        void UpdateCamera(Location Index, const Camera& camera);
        void UpdateCamera(Location Index, const CameraData& camera);
        
        INLINE GLuint Handle() const { return m_GPUArray.Handle(); }
        INLINE const StorageBuffer& Buffer() const { return m_GPUArray.Buffer(); }
        
        INLINE size_t Size() const { return m_GPUArray.Size(); }
        INLINE size_t SizeInBytes() const { return m_GPUArray.SizeInBytes(); }
        
        const CameraData& GetCamera(Location Index) const {return m_CPUArray[Index];}
        const CameraData& GetMainCameraData() const {return m_CPUArray[m_MainCamera];}
        Location GetMainCamera() const {return m_MainCamera;}
        void SetMainCamera(Location Index) {m_MainCamera = Index;}
        
    private:
        void Update()
        {
            if (!m_HasChanged) return;
            
            m_GPUArray.UpdateCameras(m_CPUArray);
            
            m_HasChanged = false;
        }
        
        std::vector<bool> m_Validity;
        std::vector<CameraData> m_CPUArray;
        Rendering::CameraArray m_GPUArray;
        size_t m_FreeCount = 0;
        Location m_MainCamera = 0;
        bool m_HasChanged = false;
    };
    
    class CommandContext
    {
    public:
        friend class CommandList;
        
        CommandContext() = default;
        ~CommandContext() = default;
        CommandContext(const CommandContext& Other) = delete;
        CommandContext(CommandContext&& Other) noexcept = delete;
        CommandContext& operator=(const CommandContext& Other) = delete;
        CommandContext& operator=(CommandContext&& Other) noexcept = delete;

        template<typename T, typename ...Args> Location Add(std::string_view Name, Args ...Params) {static_assert(sizeof(T) == 0, "Unsupported object type or parameters");}
        template<typename T> T& Get(Location Index) {return ObjectList<T>()[Index];}
        template<typename T> const T& Get(Location Index) const {return ObjectList<T>()[Index];}
        template<typename T> T& Get(std::string_view Name) {return ObjectList<T>().Get(ToName(Name));}
        template<typename T> const T& Get(std::string_view Name) const {return ObjectList<T>().Get(ToName(Name));}
        
        template<typename T, typename ...Args> Location AddVariable(std::string_view Name, const T& BaseValue, Args ...Params) {static_assert(sizeof(T) == 0, "Unsupported object type or parameters");}
        template<typename T> const T& GetValue(Location Index) const {return VariableList<T>().Get(Index);}
        template<typename T> const T& GetValue(std::string_view Name) const {return VariableList<T>().Get(VariableList<T>().GetLocation(Name));}
        
        template<typename T> void SetValue(Location Index, const T& Value) {VariableList<T>().Set(Index, Value);}
        template<typename T> void SetValue(std::string_view Name, const T& Value) {VariableList<T>().Set(VariableList<T>().GetLocation(Name), Value);}
        
        template<typename T> bool HasChanged(Location Index) const {return VariableList<T>().HasChanged(Index);}
        template<typename T> bool HasChanged(std::string_view Name) const {return VariableList<T>().HasChanged(VariableList<T>().GetLocation(Name));}
        
        template<typename T> Location GetLocation(Name Name) const {static_assert(sizeof(T) == 0, "Unsupported object type");}
        template<typename T> INLINE Location GetLocation(std::string_view Name) const {return GetLocation<T>(ToName(Name));}
        
        INLINE Location AddCamera()                                                 {return m_CameraArray.AddCamera();}
        INLINE Location AddCamera(const Camera& camera)                             {return m_CameraArray.AddCamera(camera);}
        INLINE Location AddCamera(const CameraData& camera)                         {return m_CameraArray.AddCamera(camera);}
        INLINE void UpdateCamera(Location Index, const Camera& camera)              {m_CameraArray.UpdateCamera(Index, camera);}
        INLINE void UpdateCamera(Location Index, const CameraData& camera)          {m_CameraArray.UpdateCamera(Index, camera);}
        
        INLINE GLuint Handle() const                                                {return m_CameraArray.Handle();}
        INLINE const StorageBuffer& Buffer() const                                  {return m_CameraArray.Buffer();}
        
        INLINE size_t Size() const                                                  {return m_CameraArray.Size(); }
        INLINE size_t SizeInBytes() const                                           {return m_CameraArray.SizeInBytes(); }
        
        INLINE const CameraData& GetCamera(Location Index) const                    {return m_CameraArray.GetCamera(Index);}
        INLINE const CameraData& GetMainCameraData() const                          {return m_CameraArray.GetMainCameraData();}
        INLINE Location GetMainCamera() const                                       {return m_CameraArray.GetMainCamera();}
        INLINE void SetMainCamera(Location Index)                                   {m_CameraArray.SetMainCamera(Index);}
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
        _Graph::VariablePool<Bool>              m_BoolVariables;
        _Graph::VariablePool<UInt>              m_UIntVariables;
        _Graph::VariablePool<Int>               m_IntVariables;
        _Graph::VariablePool<Float>             m_FloatVariables;
        _Graph::VariablePool<Size2D>            m_Size2DVariables;
        _Graph::VariablePool<Rect>              m_RectVariables;
        _Graph::VariablePool<Math::Vector3f>    m_Vec3Variables;
        
        // Scene
        Graph::CameraArray m_CameraArray;
        GLTF::GPUScene m_SceneTree;
    };
    
    template<> Location CommandContext::Add<VertexBuffer>(std::string_view Name);

    template<> Location CommandContext::Add<IndexBuffer>(std::string_view Name);

    template<> Location CommandContext::Add<VertexArrayObject>(std::string_view Name);

    template<> Location CommandContext::Add<UniformBuffer>(std::string_view Name);
    template<> Location CommandContext::Add<UniformBuffer>(std::string_view Name, uint32_t Size);
    template<> Location CommandContext::Add<UniformBuffer>(std::string_view Name, uint32_t Size, const void* data);

    template<> Location CommandContext::Add<StorageBuffer>(std::string_view Name);
    template<> Location CommandContext::Add<StorageBuffer>(std::string_view Name, uint32_t Size);
    template<> Location CommandContext::Add<StorageBuffer>(std::string_view Name, uint32_t Size, const void* data);

    // template<> Location CommandContext::Add<MeshObject>(std::string_view Name);
    // template<> Location CommandContext::Add<MeshObject>(std::string_view Name, const Mesh& Mesh);

    template<> Location CommandContext::Add<Texture2D>(std::string_view Name, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout);
    template<> Location CommandContext::Add<Texture2D>(std::string_view Name, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout, uint8_t SampleCount);
    template<> Location CommandContext::Add<Texture2D>(std::string_view Name, const Image& Image);
    template<> Location CommandContext::Add<Texture2D>(std::string_view Name, const Image& Image, bool UseMips);
    template<> Location CommandContext::Add<Texture2D>(std::string_view Name, uint32_t width, uint32_t height, Image::Type type, Image::Layout layout, const void* ImageData, size_t ImageSize);
    template<> Location CommandContext::Add<Texture2D>(std::string_view Name, uint32_t width, uint32_t height, Image::Type type, Image::Layout layout, const void* ImageData, size_t ImageSize, bool UseMips);

    template<> Location CommandContext::Add<Texture2DArray>(std::string_view Name, uint32_t width, uint32_t height, uint32_t count, Texture::Type type, Texture::Layout layout);
    template<> Location CommandContext::Add<Texture2DArray>(std::string_view Name, uint32_t width, uint32_t height, uint32_t count, Texture::Type type, Texture::Layout layout, bool UseMips);

    template<> Location CommandContext::Add<Texture3D>(std::string_view Name, uint32_t width, uint32_t height, uint32_t depth, Texture::Type type, Texture::Layout layout);
    template<> Location CommandContext::Add<Texture3D>(std::string_view Name, const ImageCube& Image);
    template<> Location CommandContext::Add<Texture3D>(std::string_view Name, const ImageCube& Image, bool UseMips);

    template<> Location CommandContext::Add<TextureCube>(std::string_view Name, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout);
    template<> Location CommandContext::Add<TextureCube>(std::string_view Name, uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout, bool UseMips);
    template<> Location CommandContext::Add<TextureCube>(std::string_view Name, std::span<const TextureCube::FacePair> Faces);
    template<> Location CommandContext::Add<TextureCube>(std::string_view Name, std::span<const TextureCube::FacePair> Faces, bool UseMips);
    
    // template<> Location CommandContext::Add<TextureCubeView>(std::string_view Name, const TextureCube& texture, uint32_t MipLevel);
    // template<> Location CommandContext::Add<TextureCubeView>(std::string_view Name, const TextureCube& texture, uint32_t MipLevel, uint32_t MipCount); 
    // template<> Location CommandContext::Add<TextureCubeView>(std::string_view Name, std::string_view texture, uint32_t MipLevel);
    // template<> Location CommandContext::Add<TextureCubeView>(std::string_view Name, std::string_view texture, uint32_t MipLevel, uint32_t MipCount);
    // template<> Location CommandContext::Add<TextureCubeView>(std::string_view Name, Location texture, uint32_t MipLevel);
    // template<> Location CommandContext::Add<TextureCubeView>(std::string_view Name, Location texture, uint32_t MipLevel, uint32_t MipCount);

    template<> Location CommandContext::AddVariable<Bool>(std::string_view Name, const Bool& BaseValue);
    template<> Location CommandContext::AddVariable<UInt>(std::string_view Name, const UInt& BaseValue);
    template<> Location CommandContext::AddVariable<Int>(std::string_view Name, const Int& BaseValue);
    template<> Location CommandContext::AddVariable<Float>(std::string_view Name, const Float& BaseValue);
    template<> Location CommandContext::AddVariable<Size2D>(std::string_view Name, const Size2D& BaseValue);
    template<> Location CommandContext::AddVariable<Rect>(std::string_view Name, const Rect& BaseValue);
    template<> Location CommandContext::AddVariable<Math::Vector3f>(std::string_view Name, const Math::Vector3f& BaseValue);
    
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
    
    template<> INLINE _Graph::VariablePool<Bool>&                   CommandContext::VariableList<Bool>()            {return m_BoolVariables;}
    template<> INLINE _Graph::VariablePool<UInt>&                   CommandContext::VariableList<UInt>()            {return m_UIntVariables;}
    template<> INLINE _Graph::VariablePool<Int>&                    CommandContext::VariableList<Int>()             {return m_IntVariables;}
    template<> INLINE _Graph::VariablePool<Float>&                  CommandContext::VariableList<Float>()           {return m_FloatVariables;}
    template<> INLINE _Graph::VariablePool<Size2D>&                 CommandContext::VariableList<Size2D>()          {return m_Size2DVariables;}
    template<> INLINE _Graph::VariablePool<Rect>&                   CommandContext::VariableList<Rect>()            {return m_RectVariables;}
    template<> INLINE _Graph::VariablePool<Math::Vector3f>&         CommandContext::VariableList<Math::Vector3f>()  {return m_Vec3Variables;}
    
    template<> INLINE const _Graph::VariablePool<Bool>&             CommandContext::VariableList<Bool>()            const {return m_BoolVariables;}
    template<> INLINE const _Graph::VariablePool<UInt>&             CommandContext::VariableList<UInt>()            const {return m_UIntVariables;}
    template<> INLINE const _Graph::VariablePool<Int>&              CommandContext::VariableList<Int>()             const {return m_IntVariables;}
    template<> INLINE const _Graph::VariablePool<Float>&            CommandContext::VariableList<Float>()           const {return m_FloatVariables;}
    template<> INLINE const _Graph::VariablePool<Size2D>&           CommandContext::VariableList<Size2D>()          const {return m_Size2DVariables;}
    template<> INLINE const _Graph::VariablePool<Rect>&             CommandContext::VariableList<Rect>()            const {return m_RectVariables;}
    template<> INLINE const _Graph::VariablePool<Math::Vector3f>&   CommandContext::VariableList<Math::Vector3f>()  const {return m_Vec3Variables;}
    
    template<> INLINE Location CommandContext::GetLocation<VertexBuffer>(Name Name) const      {return ObjectList<VertexBuffer>().GetLocation(Name);}
    template<> INLINE Location CommandContext::GetLocation<IndexBuffer>(Name Name) const       {return ObjectList<IndexBuffer>().GetLocation(Name);}
    template<> INLINE Location CommandContext::GetLocation<VertexArrayObject>(Name Name) const {return ObjectList<VertexArrayObject>().GetLocation(Name);}
    template<> INLINE Location CommandContext::GetLocation<UniformBuffer>(Name Name) const     {return ObjectList<UniformBuffer>().GetLocation(Name);}
    template<> INLINE Location CommandContext::GetLocation<StorageBuffer>(Name Name) const     {return ObjectList<StorageBuffer>().GetLocation(Name);}
    // template<> INLINE Location CommandContext::GetLocation<MeshObject>(Name Name) const        {return ObjectList<MeshObject>().GetLocation(Name);}
    template<> INLINE Location CommandContext::GetLocation<Texture2D>(Name Name) const         {return ObjectList<Texture2D>().GetLocation(Name);}
    template<> INLINE Location CommandContext::GetLocation<Texture2DArray>(Name Name) const    {return ObjectList<Texture2DArray>().GetLocation(Name);}
    template<> INLINE Location CommandContext::GetLocation<Texture3D>(Name Name) const         {return ObjectList<Texture3D>().GetLocation(Name);}
    template<> INLINE Location CommandContext::GetLocation<TextureCube>(Name Name) const       {return ObjectList<TextureCube>().GetLocation(Name);}
    // template<> INLINE Location CommandContext::GetLocation<TextureCubeView>(Name Name) const   {return ObjectList<TextureCubeView>().GetLocation(Name);}
    template<> INLINE Location CommandContext::GetLocation<Bool>(Name Name) const              {return VariableList<Bool>().GetLocation(Name);}
    template<> INLINE Location CommandContext::GetLocation<UInt>(Name Name) const              {return VariableList<UInt>().GetLocation(Name);}
    template<> INLINE Location CommandContext::GetLocation<Int>(Name Name) const               {return VariableList<Int>().GetLocation(Name);}
    template<> INLINE Location CommandContext::GetLocation<Float>(Name Name) const             {return VariableList<Float>().GetLocation(Name);}
    template<> INLINE Location CommandContext::GetLocation<Size2D>(Name Name) const            {return VariableList<Size2D>().GetLocation(Name);}
    template<> INLINE Location CommandContext::GetLocation<Rect>(Name Name) const              {return VariableList<Rect>().GetLocation(Name);}
    template<> INLINE Location CommandContext::GetLocation<Math::Vector3f>(Name Name) const    {return VariableList<Math::Vector3f>().GetLocation(Name);}
    
    class Command
    {
    public:
        friend class CommandList;
        
        virtual ~Command() = default;
        
    protected:
        Command(CommandContext& Resources) {}
        
        virtual void OnReloadShaders(CommandContext& Resources) = 0;
        virtual void OnUpdate(CommandContext& Resources, double DeltaTime) = 0;
        virtual void OnExecute(const CommandContext& Resources) = 0;
    };
    
    class CommandList
    {
    public:
        CommandList() = default;
        ~CommandList() = default;
        CommandList(const CommandList& Other) = delete;
        CommandList(CommandList&& Other) noexcept = delete;
        CommandList& operator=(const CommandList& Other) = delete;
        CommandList& operator=(CommandList&& Other) noexcept = delete;

        // Used to add, set or get variables
        CommandContext& Context() {return m_Context;}
        
        template<typename T> requires (std::is_base_of_v<Command, T>)
        void PushNode()
        {
            m_Commands.emplace_back(ctti::nameof<T>(), std::make_unique<T>(m_Context));
        }
        
        void ReloadShaders();
        void Update(double DeltaTime);
        void Render() const;
        
    private:
        struct CommandInst
        {
            ctti::detail::cstring name;
            std::unique_ptr<Command> node;
        };
        
        CommandContext m_Context;
        std::vector<CommandInst> m_Commands;
    };
}
