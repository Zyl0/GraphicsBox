Ray Tracing
===========
[Home](../Documentation.md)

Header: ```<RayTracing/RayTrace.h>```

 - [Base API](#base-api)
 - [Acceleration Structures](#acceleration-structures)

## Base API

A ```Ray``` holds information to describe a geometrical ray, with ```origin```, ```direction``` and ```distance```.

A ```Hit``` hold information describing an intersection test result between a face and a ray, with ```u, v ``` baricenters, ```t``` distance from the ray origin and ```face``` index.

The intersection test between a [mesh](Mesh.md)'s face and a ray is done using the following function. For now only triangle primitives are supported.
```c++
// Ray tracing intersection test for trianges
Hit IntersectTriangle(const Mesh::ConstFace& Face, const Ray& Ray);
```

From the ```Hit``` result, to optain the hit vertex interpolated data can be calculated from the following functions. For now only triangle primitives are supported.
```c++
// Face Vertex data interpolation for trianges
float VertexInterpolateTriangle(const Hit& Hit, float a, float b, float c);

// Face Vertex data interpolation for trianges
Math::Vector2f VertexInterpolateTriangle(const Hit& Hit, Math::Vector2f a, Math::Vector2f b, Math::Vector2f c);

// Face Vertex data interpolation for trianges
Math::Vector3f VertexInterpolateTriangle(const Hit& Hit, Math::Vector3f a, Math::Vector3f b, Math::Vector3f c);

// Face Vertex data interpolation for trianges
Math::Vector4f VertexInterpolateTriangle(const Hit& Hit, Math::Vector4f a, Math::Vector4f b, Math::Vector4f c);
```

To ease the process of ray tracing, the ```TraceRay``` helper class wraps the mesh faces iteration and intersection test.

```c++
Ray ray = {
    .origin = /* ... */, 
    .direction = /* ... */, 
    .distance = /* ... */
};

TraceRay rayTracer{mesh, /*, first_vertex, vertex_count, */ ray /*, world to model transform*/};
for (Hit hit : PrimaryRayTracer)
{
    // Any hit, called within the iteration. Breaking here breaks the iteration
}

if (Hit Closest = PrimaryRayTracer.ClosestHit(); Closest)
{
    // Closest Hit
}
else
{
    // Miss
}
```

## Acceleration structures

The ```BLAS``` (Bottom Layer Acceleration Structure) groups faces of a mesh and arrange them in a hierarchical bounds tree. This is used when iterating through faces to test to cut the unecessary faces to test. A BLAS correspond to a specific [```Mesh```](Mesh.md) and [```Mesh::VertexGroup```](Mesh.md). The BLAS only stores mesh face indexes as mesh information. To get the mesh data, you need to retrieve it from [```Mesh::ConstFaces```](Mesh.md) or [```Mesh::Faces```](Mesh.md) view classes.

```BLAS``` needs to be constructed from the following function. The ```BuildBLAS``` function takes an optional LeafSize which correspond to the maximum face count in a ```BLAS``` leaf; when this value is set to TriangleWave::kThreadCount, enables simd face grouping and simd intersection.
```c++
BLAS BuildBLAS(const Mesh& Mesh, uint8_t VertexGroup, uint32_t LeafSize = TriangleWave::kThreadCount);
```

The ```TLAS``` (Bottom Layer Acceleration Structure) holds references to ```BLAS``` pointers grouped instances information (World To Model transform matrix and material index) and arrange them in a hierarchical bounds tree

```TLAS``` needs to be constructed from the class default constructor. Then the following functions are used to add instances and update the ```TLAS``` tree.
```c++
void TLASAddInstance(TLAS& BVH, const BLAS& BLAS, size_t Material, const Math::Transform4f& Transform);

void TLASRebuild(TLAS& BVH):
```

The ```TraceRayTLAS``` and ```TraceRayBLAS``` helper classes wrap the iteration code to deal with TLAS/BLAS tree traversal and face intersection test.

```c++
Ray ray = {
    .origin = /* ... */, 
    .direction = /* ... */, 
    .distance = /* ... */
};

Hit ClosestHit{};
TraceRayTLAS rayTracerTLAS{tlas, ray};
for (BVHHit tlasHit : rayTracerTLAS)
{
    const TLASElement& Elt = tlas.Elements[tlas.Tree[tlasHit.NodeIndex].LeftIndex()];

    // Early material checks
    
    TraceRayBLAS rayTracerBLAS{*(Elt.BLAS), ray, Elt.WorldToModel};
    for (Hit Hit : rayTracerBLAS)
    {                    
        // Any hit, called within the iteration. Breaking here breaks the iteration  
    }
    
    if (Hit Closest = rayTracerBLAS.ClosestHit(); Closest)
    {
        // BLAS Closest Hit
        ClosestHit = Closest;
    }
    else
    {
        // BLAS Miss
    }
}

if (ClosestHit)
{
    // Closest Hit
}
else
{
    // Miss
}
```