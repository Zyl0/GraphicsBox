#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "Math/Vector.h"
#include "Shared/Annotations.h"

/**
 * @brief A rendering::Mesh is a representation of a mesh on the CPU.
 */
class Mesh
{
public:
    enum VertexType : uint8_t
    {
        POINTS = 0,
        LINE_STRIP,
        LINE_LOOP,
        LINES,
        LINE_STRIP_ADJACENCY,
        LINES_ADJACENCY,
        PATCHES,
        TRIANGLE_STRIP_ADJACENCY,
        TRIANGLES_ADJACENCY,
        TRIANGLE_STRIP,
        TRIANGLE_FAN,
        QUAD_STRIP,
        TRIANGLES,
        QUADS,
        _Count
    };
    
    /**
     * @brief A vertex group is a group of vertex that is supposed to be drawn in the same draw call.
     */
    struct VertexGroup
    {
        /**
         * @brief The index of the first vertex to draw.
         */
        unsigned int FirstVertex;
        /**
         * @brief The count of vertices to draw
         */
        unsigned int VertexCount;
        
        Math::Point3f BoundsMin;
        Math::Point3f BoundsMax;
    };
    
    Mesh() = default;
    ~Mesh() = default;

    Mesh(const Mesh& Other) = delete;

    Mesh(Mesh&& Other) noexcept
        : bIsInEditMode(Other.bIsInEditMode),
          m_mesh_type(Other.m_mesh_type),
          m_indexes(std::move(Other.m_indexes)),
          m_positions(std::move(Other.m_positions)),
          m_normals(std::move(Other.m_normals)),
          m_tangents(std::move(Other.m_tangents)),
          m_texture_coordinates(std::move(Other.m_texture_coordinates)),
          m_vertex_group(std::move(Other.m_vertex_group))
    {
    }

    Mesh& operator=(const Mesh& Other) = delete;

    Mesh& operator=(Mesh&& Other) noexcept
    {
        if (this == &Other)
            return *this;
        bIsInEditMode = Other.bIsInEditMode;
        m_mesh_type = Other.m_mesh_type;
        m_indexes = std::move(Other.m_indexes);
        m_positions = std::move(Other.m_positions);
        m_normals = std::move(Other.m_normals);
        m_tangents = std::move(Other.m_tangents);
        m_texture_coordinates = std::move(Other.m_texture_coordinates);
        m_vertex_group = std::move(Other.m_vertex_group);
        return *this;
    }

    /**
     * @return true if mesh is indexed.
     */
    INLINE NO_DISCARD bool IsIndexedMesh() const {return !m_indexes.empty();}

    /**
     * @return GPU side mesh primitive type.
     */
    INLINE NO_DISCARD VertexType GetMeshType() const {return m_mesh_type;}

    INLINE NO_DISCARD std::span<const Math::Point3f> GetPositions() const              {return m_positions;}
    INLINE NO_DISCARD std::span<const Math::Vector3f> GetNormals() const               {return m_normals;}
    INLINE NO_DISCARD std::span<const Math::Vector3f> GetTangents() const              {return m_tangents;}
    INLINE NO_DISCARD std::span<const Math::Vector2f> GetTextureCoordinates() const    {return m_texture_coordinates;}
    INLINE NO_DISCARD std::span<const unsigned int>  GetIndices() const                {return m_indexes;}
    INLINE NO_DISCARD std::span<const VertexGroup> GetVertexGroups() const             {return m_vertex_group;}
    
    INLINE NO_DISCARD bool HasNormals() const {return !m_normals.empty();}
    INLINE NO_DISCARD bool HasTangents() const {return !m_tangents.empty();}
    INLINE NO_DISCARD bool HasTextureCoordinates() const {return !m_texture_coordinates.empty();}

    NO_DISCARD uint32_t GetFaceCount() const;
    NO_DISCARD static uint32_t FaceIndex(Mesh::VertexType FaceType, uint32_t FirstVertexIndex);
    NO_DISCARD static uint32_t FaceVertexIncrement(Mesh::VertexType FaceType);

    NO_DISCARD uint32_t GetVertexCount() const;
    
    /**
     * @brief Initialize ore reset mesh vertex data (CPU Side) and put it in edit mode (allow mesh edition).
     * @param meshType Mesh primitive type
     */
    void BeginMesh(VertexType meshType);

    /**
     * @brief Put mesh in edit mode.
     */
    void EditMesh();
    
    /**
     * @brief Add a position vertex data to the mesh. The mesh needs to be in edit mode to perform this operation.
     * @param p vertex position
     */
    void AddVertexPosition(const Math::Point3f &p);
    
    /**
     * @brief Add multiple position vertex data to the mesh. The mesh needs to be in edit mode to perform this operation.
     * @param ps vertex positions
     */
    void AddVertexPositions(std::span<const Math::Point3f> ps);
    
    /**
     * @brief Add a normal vector vertex data to the mesh. The mesh needs to be in edit mode to perform this operation.
     * @param n vertex normal
     */
    void AddVertexNormal(const Math::Vector3f &n);
    
    /**
     * @brief Add multiple normal vector vertex data to the mesh. The mesh needs to be in edit mode to perform this operation.
     * @param ns vertex normals
     */
    void AddVertexNormals(std::span<const Math::Vector3f> ns);
    
    /**
     * @brief Add a texture coordinate vertex data to the mesh. The mesh needs to be in edit mode to perform this operation.
     * @param uv vertex UV
     */
    void AddVertexTextureCoordinate(const Math::Vector2f &uv);
    
    /**
     * @brief Add a texture coordinate vertex data to the mesh. The mesh needs to be in edit mode to perform this operation.
     * @param uvs vertex UVs
     */
    void AddVertexTextureCoordinates(std::span<const Math::Vector2f> uvs);

    /**
     * @brief Add a vertex group to the mesh. The mesh needs to be in edit mode to perform this operation.
     * @param first index of the first vertex of the group
     * @param count vertex count of the group
     */
    void AddVertexGroup(unsigned int first, unsigned int count);
    
    /**
     * @brief Add a polygon vertex index to the mesh. This is for indexed mesh only. The mesh needs to be in edit mode to perform this operation.
     * @param index vertex index
     */
    void AddVertexPolygonIndex(unsigned int index);
    
    /**
     * @brief Add a polygon vertex index to the mesh. This is for indexed mesh only. The mesh needs to be in edit mode to perform this operation.
     * @param indexes vertex indexes
     */
    void AddVertexPolygonIndexes(std::span<const unsigned int> indexes);

    /**
     * @brief Modify the vertex position of a mesh. The mesh needs to be in edit mode to perform this operation.
     * @param i the index of the vertex to modify.
     * @param p the new vertex position position.
     */
    void SetVertexPosition(unsigned int i, const Math::Point3f &p);
    
    /**
     * @brief Modify the vertex normal of a mesh. The mesh needs to be in edit mode to perform this operation.
     * @param i the index of the vertex to modify.
     * @param n the new normal vector.
     */
    void SetVertexNormal(unsigned int i, const Math::Vector3f &n);
    
    /**
     * @brief Modify the vertex texture coordinates of a mesh. The mesh needs to be in edit mode to perform this operation.
     * @param i the index of the vertex to modify.
     * @param uv the new texture coordinates.
     */
    void SetVertexTextureCoordinate(unsigned int i, const Math::Vector2f &uv);

    /**
     * @brief Modify the polygon vertex index of a mesh. The mesh needs to be in edit mode to perform this operation.
     * @param i Polygon vertex index.
     * @param index the new vertex index.
     */
    void SetVertexPolygonIndex(unsigned int i, unsigned int index);

    /**
     * @brief Modify a vertex group of a mesh. The mesh needs to be in edit mode to perform this operation.
     * @param i index of the vertex group to modify.
     * @param first new first index of the vertex group.
     * @param count new vertex count.
     */
    void SetVertexGroup(unsigned int i, unsigned int first, unsigned int count);

    void GenerateAdgacentcy();
    
    /**
     * @brief Generates the vertex normal of a mesh. The mesh needs to be in edit mode to perform this operation.
     */
    void GenerateNormals();

    void GenerateTangents();

    void GenerateBounds();

    void ClearTangents();

    struct Vertex
    {
    private:
        struct VertexIterator;
    public:
        using iterator = VertexIterator;

        Vertex() : m_MeshReference(nullptr), m_Vertex(0) {}

        Vertex(Mesh& MeshReference, uint32_t Vertex) : m_MeshReference(&MeshReference), m_Vertex(Vertex) {}

        Vertex(const iterator& it) : m_MeshReference(it.m_MeshReference), m_Vertex(it.m_Vertex) {}
        
        NO_DISCARD Math::Point3f& Position();
        NO_DISCARD const Math::Point3f& Position() const;

        NO_DISCARD Math::Vector3f& Normal();
        NO_DISCARD const Math::Vector3f& Normal() const;

        NO_DISCARD Math::Vector3f& Tangent();
        NO_DISCARD const Math::Vector3f& Tangent() const;

        NO_DISCARD Math::Vector2f& TextureCoordinate();
        NO_DISCARD const Math::Vector2f& TextureCoordinate() const;

        INLINE NO_DISCARD bool IsValid() const {return m_MeshReference != nullptr && m_Vertex < m_MeshReference->GetVertexCount();}
        INLINE NO_DISCARD Mesh* GetMesh() {return m_MeshReference;}
        INLINE NO_DISCARD uint32_t GetIndex() const {return m_Vertex;}

    private:
        struct VertexIterator
        {
            friend Vertex;

            VertexIterator() : m_MeshReference(nullptr), m_Vertex(0) {}
            VertexIterator(Mesh& MeshReference, uint32_t Vertex) : m_MeshReference(&MeshReference), m_Vertex(Vertex) {}

            INLINE VertexIterator& operator++ () {m_Vertex++; return *this;}
            INLINE Vertex operator* () const {return Vertex(*this);}
            INLINE bool operator!= (const VertexIterator& outer) const {return m_MeshReference != outer.m_MeshReference || m_Vertex != outer.m_Vertex;}
            INLINE bool operator== (const VertexIterator& outer) const {return m_MeshReference == outer.m_MeshReference && m_Vertex == outer.m_Vertex;}

            INLINE NO_DISCARD bool IsValid() const {return m_MeshReference != nullptr && m_Vertex < m_MeshReference->GetVertexCount();}
            INLINE NO_DISCARD Mesh* GetMesh() {return m_MeshReference;}
            INLINE NO_DISCARD uint32_t GetIndex() const {return m_Vertex;}
        private:
            Mesh* m_MeshReference;
            uint32_t m_Vertex;
        };

        Mesh* m_MeshReference;
        uint32_t m_Vertex;
    };

    struct ConstVertex
    {
    private:
        struct VertexConstIterator;
    public:
        using iterator = VertexConstIterator;
        using const_iterator = VertexConstIterator;

        ConstVertex() : m_MeshReference(nullptr), m_Vertex(0) {}

        ConstVertex(const Mesh& MeshReference, uint32_t Vertex) : m_MeshReference(&MeshReference), m_Vertex(Vertex) {}

        ConstVertex(const_iterator it) : ConstVertex()
        {
            if (!it.IsValid()) return;

            m_MeshReference = it.m_MeshReference;
            m_Vertex = it.m_Vertex;
        }

        ConstVertex(Vertex vertex) : ConstVertex()
        {
            if (!vertex.IsValid()) return;

            m_MeshReference = vertex.GetMesh();
            m_Vertex = vertex.GetIndex();
        }

        ConstVertex(Vertex::iterator it) : ConstVertex()
        {
            if (!it.IsValid()) return;

            m_MeshReference = it.GetMesh();
            m_Vertex = it.GetIndex();
        }

        NO_DISCARD const Math::Point3f& Position() const;

        NO_DISCARD const Math::Vector3f& Normal() const;

        NO_DISCARD const Math::Vector3f& Tangent() const;

        NO_DISCARD const Math::Vector2f& TextureCoordinate() const;

        INLINE NO_DISCARD bool IsValid() const {return m_MeshReference != nullptr && m_Vertex < m_MeshReference->GetVertexCount();}
        INLINE NO_DISCARD const Mesh* GetMesh() const {return m_MeshReference;}
        INLINE NO_DISCARD uint32_t GetIndex() const {return m_Vertex;}

    private:
        struct VertexConstIterator
        {
            friend ConstVertex;

            VertexConstIterator() : m_MeshReference(nullptr), m_Vertex(0) {}
            VertexConstIterator(const Mesh& MeshReference, uint32_t Vertex) : m_MeshReference(&MeshReference), m_Vertex(Vertex) {}

            INLINE VertexConstIterator& operator++ () {m_Vertex++; return *this;}
            INLINE ConstVertex operator* () const {return ConstVertex(*this);}
            INLINE bool operator!= (const VertexConstIterator& outer) const {return m_MeshReference != outer.m_MeshReference || m_Vertex != outer.m_Vertex;}
            INLINE bool operator== (const VertexConstIterator& outer) const {return m_MeshReference == outer.m_MeshReference && m_Vertex == outer.m_Vertex;}

            INLINE NO_DISCARD bool IsValid() const {return m_MeshReference != nullptr && m_Vertex < m_MeshReference->GetVertexCount();}
            INLINE NO_DISCARD const Mesh* GetMesh() const {return m_MeshReference;}
            INLINE NO_DISCARD uint32_t GetIndex() const {return m_Vertex;}
        private:
            const Mesh* m_MeshReference;
            uint32_t m_Vertex;
        };

        const Mesh* m_MeshReference;
        uint32_t m_Vertex;
    };

    struct Face
    {
    private:
        struct FaceIterator;
    public:
        using iterator = FaceIterator;

        Face() : m_MeshReference(nullptr), m_FirstVertex(0) {}
        Face(Mesh& MeshReference, uint32_t FirstVertex) : m_MeshReference(&MeshReference), m_FirstVertex(FirstVertex) {}
        Face(iterator it) : Face()
        {
            if (!it.IsValid()) return;
            m_MeshReference = it.m_MeshReference;
            m_FirstVertex = it.m_FirstVertex;
        }

        NO_DISCARD Vertex GetVertex(uint8_t index);
        NO_DISCARD ConstVertex GetVertex(uint8_t index) const;
        INLINE Vertex operator[] (uint8_t index) {return GetVertex(index);}
        INLINE ConstVertex operator[] (uint8_t index) const {return GetVertex(index);}

        INLINE NO_DISCARD Math::Point3f& Position(uint8_t vertex) {AssertFaceReadable(vertex); return GetVertex(vertex).Position();}
        INLINE NO_DISCARD const Math::Point3f& Position(uint8_t vertex) const {AssertFaceReadable(vertex); return GetVertex(vertex).Position();}

        INLINE NO_DISCARD Math::Vector3f& Normal(uint8_t vertex) {AssertFaceReadable(vertex); return GetVertex(vertex).Normal();}
        INLINE NO_DISCARD const Math::Vector3f& Normal(uint8_t vertex) const {AssertFaceReadable(vertex); return GetVertex(vertex).Normal();}

        INLINE NO_DISCARD Math::Vector3f& Tangent(uint8_t vertex) {AssertFaceReadable(vertex); return GetVertex(vertex).Tangent();}
        INLINE NO_DISCARD const Math::Vector3f& Tangent(uint8_t vertex) const {AssertFaceReadable(vertex); return GetVertex(vertex).Tangent();}

        INLINE NO_DISCARD Math::Vector2f& TextureCoordinate(uint8_t vertex) {AssertFaceReadable(vertex); return GetVertex(vertex).TextureCoordinate();}
        INLINE NO_DISCARD const Math::Vector2f& TextureCoordinate(uint8_t vertex) const {AssertFaceReadable(vertex); return GetVertex(vertex).TextureCoordinate();}

        NO_DISCARD Vertex::iterator begin();
        NO_DISCARD ConstVertex::iterator begin() const;
        NO_DISCARD Vertex::iterator end();
        NO_DISCARD ConstVertex::iterator end() const;
     
        INLINE NO_DISCARD Vertex::iterator first() {return begin();}
        INLINE NO_DISCARD ConstVertex::iterator first() const {return begin();}
        NO_DISCARD Vertex::iterator last();
        NO_DISCARD ConstVertex::iterator last() const;

        INLINE NO_DISCARD bool IsValid() const {return m_MeshReference != nullptr && m_FirstVertex < m_MeshReference->GetVertexCount();}
        INLINE NO_DISCARD Mesh* GetMesh() {return m_MeshReference;}
        INLINE NO_DISCARD uint32_t FirstVertex() const {return m_FirstVertex;}

    private:
        void AssertFaceReadable(uint8_t vertex) const;
     
        struct FaceIterator
        {
            friend Face;

            FaceIterator() : m_MeshReference(nullptr), m_FirstVertex(0) {}
            FaceIterator(Mesh& MeshReference, uint32_t FirstVertex) : m_MeshReference(&MeshReference), m_FirstVertex(FirstVertex) {}

            FaceIterator& operator++ () {m_FirstVertex += Mesh::FaceVertexIncrement(m_MeshReference->GetMeshType()); return *this;}
            INLINE Face operator* () const {return Face(*this);}
            INLINE bool operator!= (const FaceIterator& outer) const {return m_MeshReference != outer.m_MeshReference || m_FirstVertex != outer.m_FirstVertex;}
            INLINE bool operator== (const FaceIterator& outer) const {return m_MeshReference == outer.m_MeshReference && m_FirstVertex == outer.m_FirstVertex;}

            INLINE NO_DISCARD bool IsValid() const {return m_MeshReference != nullptr && m_FirstVertex < m_MeshReference->GetVertexCount();}
            INLINE NO_DISCARD Mesh* GetMesh() {return m_MeshReference;}
            INLINE NO_DISCARD uint32_t FirstVertex() const {return m_FirstVertex;}
        private:
            Mesh* m_MeshReference;
            uint32_t m_FirstVertex;
        };

        Mesh* m_MeshReference;
        uint32_t m_FirstVertex;
    };


    struct ConstFace
    {
    private:
        struct ConstFaceIterator;
    public:
        using iterator = ConstFaceIterator;
        using const_iterator = ConstFaceIterator;

        ConstFace() : m_MeshReference(nullptr), m_FirstVertex(0) {}
        ConstFace(const Mesh& MeshReference, uint32_t FirstVertex) : m_MeshReference(&MeshReference), m_FirstVertex(FirstVertex) {}
        ConstFace(iterator it) : ConstFace()
        {
            if (!it.IsValid()) return;
            m_MeshReference = it.m_MeshReference;
            m_FirstVertex = it.m_FirstVertex;
        }
        ConstFace(Face face): ConstFace()
        {
            if (!face.IsValid()) return;
            m_MeshReference = face.GetMesh();
            m_FirstVertex = face.FirstVertex();
        }

        NO_DISCARD ConstVertex GetVertex(uint8_t index) const;
        INLINE ConstVertex operator[] (uint8_t index) const {return GetVertex(index);}

        INLINE NO_DISCARD const Math::Point3f& Position(uint8_t vertex) const {AssertFaceReadable(vertex); return GetVertex(vertex).Position();}

        INLINE NO_DISCARD const Math::Vector3f& Normal(uint8_t vertex) const {AssertFaceReadable(vertex); return GetVertex(vertex).Normal();}

        INLINE NO_DISCARD const Math::Vector3f& Tangent(uint8_t vertex) const {AssertFaceReadable(vertex); return GetVertex(vertex).Tangent();}

        INLINE NO_DISCARD const Math::Vector2f& TextureCoordinate(uint8_t vertex) const {AssertFaceReadable(vertex); return GetVertex(vertex).TextureCoordinate();}

        NO_DISCARD ConstVertex::iterator begin() const;
        NO_DISCARD ConstVertex::iterator end() const;

        INLINE NO_DISCARD ConstVertex::iterator first() const {return begin();}
        NO_DISCARD ConstVertex::iterator last() const;

        INLINE NO_DISCARD bool IsValid() const {return m_MeshReference != nullptr && m_FirstVertex < m_MeshReference->GetVertexCount();}
        INLINE NO_DISCARD const Mesh* GetMesh() {return m_MeshReference;}
        INLINE NO_DISCARD uint32_t FirstVertex() const {return m_FirstVertex;}

    private:
        void AssertFaceReadable(uint8_t vertex) const;

        struct ConstFaceIterator
        {
            friend ConstFace;

            ConstFaceIterator() : m_MeshReference(nullptr), m_FirstVertex(0) {}
            ConstFaceIterator(const Mesh& MeshReference, uint32_t FirstVertex) : m_MeshReference(&MeshReference), m_FirstVertex(FirstVertex) {}

            ConstFaceIterator& operator++ () {m_FirstVertex += Mesh::FaceVertexIncrement(m_MeshReference->GetMeshType()); return *this;}
            INLINE ConstFace operator* () const {return ConstFace(*this);}
            INLINE bool operator!= (const ConstFaceIterator& outer) const {return m_MeshReference != outer.m_MeshReference || m_FirstVertex != outer.m_FirstVertex;}
            INLINE bool operator== (const ConstFaceIterator& outer) const {return m_MeshReference == outer.m_MeshReference && m_FirstVertex == outer.m_FirstVertex;}

            INLINE NO_DISCARD bool IsValid() const {return m_MeshReference != nullptr && m_FirstVertex < m_MeshReference->GetVertexCount();}
            INLINE NO_DISCARD const Mesh* GetMesh() {return m_MeshReference;}
            INLINE NO_DISCARD uint32_t FirstVertex() const {return m_FirstVertex;}
        private:
            const Mesh* m_MeshReference;
            uint32_t m_FirstVertex;
        };

        const Mesh* m_MeshReference;
        uint32_t m_FirstVertex;
    };

    struct Vertices
    {
        using iterator = Vertex::iterator;
        using const_iterator = ConstVertex::iterator;

        Vertices() : m_MeshReference(nullptr) {}
        Vertices(Mesh& MeshReference) : m_MeshReference(&MeshReference) {}

        INLINE ConstVertex operator[] (uint32_t index) const {return IsValid() ? ConstVertex(*m_MeshReference, index) : ConstVertex();}
        INLINE Vertex operator[] (unsigned index) {return IsValid() ? Vertex(*m_MeshReference, index) : Vertex();}

        INLINE NO_DISCARD iterator begin() {return IsValid() ? iterator(*m_MeshReference, 0) : iterator();}
        INLINE NO_DISCARD const_iterator begin() const {return IsValid() ? const_iterator(*m_MeshReference, 0) : const_iterator();}
        INLINE NO_DISCARD iterator end() {return IsValid() ? iterator(*m_MeshReference, m_MeshReference->GetVertexCount()) : iterator();}
        INLINE NO_DISCARD const_iterator end() const {return IsValid() ? const_iterator(*m_MeshReference, m_MeshReference->GetVertexCount()) : const_iterator();}

        INLINE NO_DISCARD iterator first() {return begin();}
        INLINE NO_DISCARD const_iterator first() const {return begin();}
        INLINE NO_DISCARD iterator last() {return IsValid() ? iterator(*m_MeshReference,  m_MeshReference->GetVertexCount() > 0 ?  m_MeshReference->GetVertexCount() - 1 : 0) : iterator();}
        INLINE NO_DISCARD const_iterator last() const {return IsValid() ? const_iterator(*m_MeshReference,  m_MeshReference->GetVertexCount() > 0 ?  m_MeshReference->GetVertexCount() - 1 : 0) : const_iterator();}

        INLINE NO_DISCARD bool IsValid() const {return m_MeshReference != nullptr;}
    private:        
        Mesh* m_MeshReference;
    };

    struct ConstVertices
    {
        using iterator = ConstVertex::iterator;
        using const_iterator = ConstVertex::iterator;

        ConstVertices() : m_MeshReference(nullptr) {}
        ConstVertices(const Mesh& MeshReference) : m_MeshReference(&MeshReference) {}

        INLINE ConstVertex operator[] (uint32_t index) const {return IsValid() ? ConstVertex(*m_MeshReference, index) : ConstVertex();}

        INLINE NO_DISCARD iterator begin() {return IsValid() ? iterator(*m_MeshReference, 0) : iterator();}
        INLINE NO_DISCARD const_iterator begin() const {return IsValid() ? const_iterator(*m_MeshReference, 0) : const_iterator();}
        INLINE NO_DISCARD iterator end() {return IsValid() ? iterator(*m_MeshReference, m_MeshReference->GetVertexCount()) : iterator();}
        INLINE NO_DISCARD const_iterator end() const {return IsValid() ? const_iterator(*m_MeshReference, m_MeshReference->GetVertexCount()) : const_iterator();}

        INLINE NO_DISCARD iterator first() {return begin();}
        INLINE NO_DISCARD const_iterator first() const {return begin();}
        INLINE NO_DISCARD iterator last() {return IsValid() ? iterator(*m_MeshReference,  m_MeshReference->GetVertexCount() > 0 ?  m_MeshReference->GetVertexCount() - 1 : 0) : iterator();}
        INLINE NO_DISCARD const_iterator last() const {return IsValid() ? const_iterator(*m_MeshReference,  m_MeshReference->GetVertexCount() > 0 ?  m_MeshReference->GetVertexCount() - 1 : 0) : const_iterator();}

        INLINE NO_DISCARD bool IsValid() const {return m_MeshReference != nullptr;}
    private:        
        const Mesh* m_MeshReference;
    };
 
    struct Faces
    {
        using iterator = Face::iterator;
        using const_iterator = ConstFace::iterator;

        Faces() : m_MeshReference(nullptr) {}
        Faces(Mesh& MeshReference) : m_MeshReference(&MeshReference) {}

        INLINE Face operator[] (unsigned index) {return IsValid() ? Face(*m_MeshReference, GetFaceFirstVertex(index)) : Face();}
        INLINE ConstFace operator[](uint32_t index) const {return IsValid() ? ConstFace(*m_MeshReference, index) : ConstFace();}

        INLINE NO_DISCARD iterator begin() {return IsValid() ? iterator(*m_MeshReference, 0) : iterator();}
        INLINE NO_DISCARD const_iterator begin() const {return IsValid() ? const_iterator(*m_MeshReference, 0) : const_iterator();}
        INLINE NO_DISCARD iterator end() {return IsValid() ? iterator(*m_MeshReference, m_MeshReference->GetVertexCount()) : iterator();}
        INLINE NO_DISCARD const_iterator end() const {return IsValid() ? const_iterator(*m_MeshReference, m_MeshReference->GetVertexCount()) : const_iterator();}
     
        INLINE NO_DISCARD iterator first() {return begin();}
        INLINE NO_DISCARD const_iterator first() const {return begin();}
        INLINE NO_DISCARD iterator last()
        {
            if (!IsValid()) return {};
            unsigned count = m_MeshReference->GetFaceCount();
            return iterator(*m_MeshReference, count > 0 ? count - 1 : 0);
        }
        INLINE NO_DISCARD const_iterator last() const
        {
            if (!IsValid()) return {};
            unsigned count = m_MeshReference->GetFaceCount();
            return const_iterator(*m_MeshReference, count > 0 ? count - 1 : 0);
        }

        INLINE NO_DISCARD bool IsValid() const {return m_MeshReference != nullptr;}
    private:
        uint32_t GetFaceFirstVertex(uint32_t index) const;
        
        Mesh* m_MeshReference;
    };

    struct ConstFaces
    {
        using iterator = ConstFace::iterator;
        using const_iterator = ConstFace::iterator;

        ConstFaces() : m_MeshReference(nullptr) {}
        ConstFaces(const Mesh& MeshReference) : m_MeshReference(&MeshReference) {}

        INLINE ConstFace operator[](uint32_t index) const {return IsValid() ? ConstFace(*m_MeshReference, GetFaceFirstVertex(index)) : ConstFace();}

        INLINE NO_DISCARD iterator begin() {return IsValid() ? iterator(*m_MeshReference, 0) : iterator();}
        INLINE NO_DISCARD const_iterator begin() const {return IsValid() ? const_iterator(*m_MeshReference, 0) : const_iterator();}
        INLINE NO_DISCARD iterator end() {return IsValid() ? iterator(*m_MeshReference, m_MeshReference->GetVertexCount()) : iterator();}
        INLINE NO_DISCARD const_iterator end() const {return IsValid() ? const_iterator(*m_MeshReference, m_MeshReference->GetVertexCount()) : const_iterator();}

        INLINE NO_DISCARD iterator first() {return begin();}
        INLINE NO_DISCARD const_iterator first() const {return begin();}
        INLINE NO_DISCARD iterator last()
        {
            if (!IsValid()) return {};
            unsigned count = m_MeshReference->GetFaceCount();
            return iterator(*m_MeshReference, count > 0 ? count - 1 : 0);
        }
        INLINE NO_DISCARD const_iterator last() const
        {
            if (!IsValid()) return {};
            unsigned count = m_MeshReference->GetFaceCount();
            return const_iterator(*m_MeshReference, count > 0 ? count - 1 : 0);
        }

        INLINE NO_DISCARD bool IsValid() const {return m_MeshReference != nullptr;}
    private:
        uint32_t GetFaceFirstVertex(uint32_t index) const;
        
        const Mesh* m_MeshReference;
    };
    /**
     * @brief Put the mesh out of edit mode and initialize it on the GPU side.
     */
    void CommitMesh();

    void Clear(bool ResizeToZero = false);

private:
    bool bIsInEditMode = false;
    
    /**
     * @brief Mesh primitive type (lines, triangles, quads, etc)
     */
    VertexType m_mesh_type = VertexType::TRIANGLES;

    std::vector<unsigned int> m_indexes;
    std::vector<Math::Point3f> m_positions;
    std::vector<Math::Vector3f> m_normals;
    std::vector<Math::Vector3f> m_tangents;
    std::vector<Math::Vector2f> m_texture_coordinates;
    std::vector<VertexGroup> m_vertex_group;
};
