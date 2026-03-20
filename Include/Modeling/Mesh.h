#pragma once

#include <filesystem>
#include <vector>

#include "Vector.h"
#include "Shared/Annotations.h"

/**
 * @brief Mesh is a representation of a mesh for both CPU and GPU side used for rendering.
 */
class Mesh
{
public:
    enum VertexType
    {
        POINTS = 0,
        LINE_STRIP,
        LINE_LOOP,
        LINES,
        LINE_STRIP_ADJACENCY,
        LINES_ADJACENCY,
        TRIANGLE_STRIP,
        TRIANGLE_FAN,
        TRIANGLES,
        TRIANGLE_STRIP_ADJACENCY,
        TRIANGLES_ADJACENCY,
        QUADS,
        QUAD_STRIP,
        PATCHES,
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
    INLINE bool IsIndexedMesh() const {return !m_indexes.empty();}

    /**
     * @return GPU side mesh primitive type.
     */
    INLINE VertexType GetMeshType() const {return m_mesh_type;}

    INLINE const std::vector<Math::Point3f> &GetPositions() const           {return m_positions;}
    INLINE const std::vector<Math::Vector3f> &GetNormals() const            {return m_normals;}
    INLINE const std::vector<Math::Vector3f> &GetTangents() const            {return m_tangents;}
    INLINE const std::vector<Math::Vector2f> &GetTextureCoordinates() const {return m_texture_coordinates;}
    INLINE const std::vector<unsigned int> &GetIndexes() const                      {return m_indexes;}

    INLINE const std::vector<VertexGroup> &GetVertexGroups() const                  {return m_vertex_group;}
    
    INLINE bool HasNormals() const {return !m_normals.empty();}
    INLINE bool HasTangents() const {return !m_tangents.empty();}
    INLINE bool HasTextureCoordinates() const {return !m_texture_coordinates.empty();}

    unsigned int GetFaceCount() const;

    unsigned int GetVertexCount() const;
    
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
     * @brief Add a normal vector vertex data to the mesh. The mesh needs to be in edit mode to perform this operation.
     * @param n vertex normal
     */
    void AddVertexNormal(const Math::Vector3f &n);
    
    /**
     * @brief Add a texture coordinate vertex data to the mesh. The mesh needs to be in edit mode to perform this operation.
     * @param uv vertex data
     */
    void AddVertexTextureCoordinate(const Math::Vector2f &uv);

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

    void ClearTangents();

    struct Vertex
    {
    private:
        struct VertexIterator;
    public:
        using iterator = VertexIterator;

        Vertex(Mesh& MeshReference, uint32_t Vertex) : m_MeshReference(MeshReference), m_Vertex(Vertex) {}

        Math::Point3f& Position();
        const Math::Point3f& Position() const;

        Math::Vector3f& Normal();
        const Math::Vector3f& Normal() const;

        Math::Vector3f& Tangent();
        const Math::Vector3f& Tangent() const;

        Math::Vector2f& TextureCoordinate();
        const Math::Vector2f& TextureCoordinate() const;
    private:
        struct VertexIterator
        {
            VertexIterator(Mesh& MeshReference, uint32_t Vertex) : m_MeshReference(MeshReference), m_Vertex(Vertex) {}

            INLINE VertexIterator& operator++ () {m_Vertex++; return *this;}
            INLINE Vertex operator* () const {return Vertex(m_MeshReference, m_Vertex);}
            INLINE bool operator!= (const VertexIterator& outer) const {return &(m_MeshReference) != &(outer.m_MeshReference) || m_Vertex != outer.m_Vertex;}
            INLINE bool operator== (const VertexIterator& outer) const {return &(m_MeshReference) == &(outer.m_MeshReference) && m_Vertex == outer.m_Vertex;}
        private:
            Mesh& m_MeshReference;
            uint32_t m_Vertex;
        };

        Mesh& m_MeshReference;
        uint32_t m_Vertex;
    };

    struct Face
    {
    private:
        struct FaceIterator;
    public:
        using iterator = FaceIterator;

        Face(Mesh& MeshReference, uint32_t FirstVertex) : m_MeshReference(MeshReference), m_FirstVertex(FirstVertex) {}

        Vertex GetVertex(uint8_t index);
        const Vertex GetVertex(uint8_t index) const;
        INLINE Vertex operator[] (uint8_t index) const {return GetVertex(index);}

        INLINE Math::Point3f& Position(uint8_t vertex) {AssertFaceReadable(vertex); return GetVertex(vertex).Position();}
        INLINE const Math::Point3f& Position(uint8_t vertex) const {AssertFaceReadable(vertex); return GetVertex(vertex).Position();}

        INLINE Math::Vector3f& Normal(uint8_t vertex) {AssertFaceReadable(vertex); return GetVertex(vertex).Normal();}
        INLINE const Math::Vector3f& Normal(uint8_t vertex) const {AssertFaceReadable(vertex); return GetVertex(vertex).Normal();}

        INLINE Math::Vector3f& Tangent(uint8_t vertex) {AssertFaceReadable(vertex); return GetVertex(vertex).Tangent();}
        INLINE const Math::Vector3f& Tangent(uint8_t vertex) const {AssertFaceReadable(vertex); return GetVertex(vertex).Tangent();}

        INLINE Math::Vector2f& TextureCoordinate(uint8_t vertex) {AssertFaceReadable(vertex); return GetVertex(vertex).TextureCoordinate();}
        INLINE const Math::Vector2f& TextureCoordinate(uint8_t vertex) const {AssertFaceReadable(vertex); return GetVertex(vertex).TextureCoordinate();}

        Vertex::iterator begin();
        Vertex::iterator end();
     
        INLINE Vertex::iterator first() {return begin();}
        Vertex::iterator last();
    private:
        void AssertFaceReadable(uint8_t vertex) const;
     
        struct FaceIterator
        {
            FaceIterator(Mesh& MeshReference, uint32_t FirstVertex) : m_MeshReference(MeshReference), m_FirstVertex(FirstVertex) {}

            FaceIterator& operator++ ();
            INLINE Face operator* () const {return Face(m_MeshReference, m_FirstVertex);}
            INLINE bool operator!= (const FaceIterator& outer) const {return &(m_MeshReference) != &(outer.m_MeshReference) || m_FirstVertex != outer.m_FirstVertex;}
            INLINE bool operator== (const FaceIterator& outer) const {return &(m_MeshReference) == &(outer.m_MeshReference) && m_FirstVertex == outer.m_FirstVertex;}
            INLINE uint32_t FirstVertex() const {return m_FirstVertex;}
        private:
            Mesh& m_MeshReference;
            uint32_t m_FirstVertex;
        };

        Mesh& m_MeshReference;
        uint32_t m_FirstVertex;
    };

    struct Vertices
    {
        using iterator = Vertex::iterator;

        Vertices(Mesh& MeshReference) : m_MeshReference(MeshReference) {}

        Vertex operator[] (unsigned index) {return Vertex(m_MeshReference, index);}

        Vertex::iterator begin() {return iterator(m_MeshReference, 0);}
        Vertex::iterator end()   {return iterator(m_MeshReference, m_MeshReference.GetVertexCount());}
     
        INLINE Vertex::iterator first() {return begin();}
        Vertex::iterator last() {return iterator(m_MeshReference,  m_MeshReference.GetVertexCount() > 0 ?  m_MeshReference.GetVertexCount() : 0);}
    private:
        Mesh& m_MeshReference;
    };
 
    struct Faces
    {
        using iterator = Face::iterator;

        Faces(Mesh& MeshReference) : m_MeshReference(MeshReference) {}

        Face operator[] (unsigned index) {return Face(m_MeshReference, index);}

        Face::iterator begin() {return iterator(m_MeshReference, 0);}
        Face::iterator end() {return iterator(m_MeshReference, m_MeshReference.GetVertexCount());}
     
        INLINE Face::iterator first() {return begin();}
        Face::iterator last()
        {
            unsigned count = m_MeshReference.GetFaceCount();
            return iterator(m_MeshReference, count > 0 ? count - 1 : 0);
        }
    private:
        Mesh& m_MeshReference;
    };
    
    /**
     * @brief Put the mesh out of edit mode and generate required data
     */
    void CommitMesh();
    
private:        
    void GenerateBounds();

    bool bIsInEditMode = false;
    
    /**
     * @brief Mesh primitive type (lines, triangles, quads, etc)
     */
    VertexType m_mesh_type = TRIANGLES;

    std::vector<unsigned int> m_indexes;
    std::vector<Math::Point3f> m_positions;
    std::vector<Math::Vector3f> m_normals;
    std::vector<Math::Vector3f> m_tangents;
    std::vector<Math::Vector2f> m_texture_coordinates;
    std::vector<VertexGroup> m_vertex_group;
};