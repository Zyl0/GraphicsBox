#pragma once

#include "Implicit.h"
#include "Primitives.h"
#include "Transforms.h"

#include "Memory/SparseList.h"

class HierarchalDistanceFieldNode;
class IScalarFieldPool
{
public:
    virtual ~IScalarFieldPool() = default;
    
    // Perform distance field evaluation
    virtual double Value(size_t Index, const Math::Vector3d& v) const = 0;
    
    // Perform evaluation in batch, result is stored in cache
    virtual void BatchValues(const Math::Vector3d& v, size_t Start = 0, size_t Count = std::numeric_limits<size_t>::max()) const = 0;
    
    // Perform evaluation in batch, result is stored in cache
    virtual void BatchValues(std::span<const Math::Vector3d> vs, size_t Start = 0, size_t Count = std::numeric_limits<size_t>::max()) const = 0;
    
    // Perform evaluation in batch for a set of indexes, result is stored in cache
    virtual void BatchValues(std::span<const size_t> Indexes, const Math::Vector3d& v) const = 0;
    
    // Perform evaluation in batch for a set of indexes, result is stored in cache
    virtual void BatchValues(std::span<const size_t> Indexes, std::span<const Math::Vector3d> vs) const = 0;
    
    // Perform evaluation in batch from points of the transformed inputs buffer, result is stored in cache
    virtual void BatchValues(size_t Start = 0, size_t Count = std::numeric_limits<size_t>::max()) const = 0;
    
    // Perform evaluation in batch of the transformed inputs buffer for a set of indexes, result is stored in cache
    virtual void BatchValues(std::span<const size_t> Indexes) const = 0;
    
    void SetUseCachedOutputs(bool UseCachedOutputs) {m_UseOutputCache = UseCachedOutputs; UpdateCache(m_PoolData);}
    // Read result from cache
    INLINE double CachedValue(size_t Index) const
    {
#ifndef CONFIG_RELEASE
        AssertOrError(Index < m_CachedValues.size(), "Index Out of Range")
#endif // CONFIG_RELEASE
        
        return m_CachedValues[Index];
    }
    
    virtual AnalyticScalarField& operator[](size_t Index) = 0;
    virtual const AnalyticScalarField& operator[](size_t Index) const = 0;
    
    INLINE bool IsTreeNode() const {return m_IsTreeNode;}
    
    AnalyticScalarField* GetUnchecked(size_t Index) {return (AnalyticScalarField*)(m_PoolData + m_ObjectSize * Index);}
    const AnalyticScalarField* GetUnchecked(size_t Index) const {return (AnalyticScalarField*)(m_PoolData + m_ObjectSize * Index);}
    
    HierarchalDistanceFieldNode* GetUncheckedTreeNode(size_t Index) {return (HierarchalDistanceFieldNode*)(m_PoolData + m_ObjectSize * Index);}
    const HierarchalDistanceFieldNode* GetUncheckedTreeNode(size_t Index) const {return (HierarchalDistanceFieldNode*)(m_PoolData + m_ObjectSize * Index);}

    void SetUseTransformedInputs(bool UseCachedInputs) {m_UseTransformedInputs = UseCachedInputs; UpdateCache(m_PoolData);}
    void SetInput(size_t Index, const Math::Vector3d& p) {m_TransformedInputs[Index] = p;}
    
protected:
    IScalarFieldPool()
    {
        SetUseCachedOutputs(false);
        SetUseTransformedInputs(false);
    }
    
    virtual size_t Size() const = 0;
    
    void UpdateCache(void* PoolData)
    {
        size_t size = Size();
        if (m_UseOutputCache)
        {
            m_CachedValues.resize(size); 
        }
        else if (m_CachedValues.capacity() > 0)
        {
            m_CachedValues.resize(0); 
            m_CachedValues.shrink_to_fit();
        }
        
        if (m_UseTransformedInputs)
        {
            m_TransformedInputs.resize(size);
        }
        else if (m_TransformedInputs.capacity() > 0)
        {
            m_TransformedInputs.resize(0); 
            m_TransformedInputs.shrink_to_fit();
        }
        
        m_PoolData = (uint8_t*)PoolData;
    }
    
    std::vector<Math::Point3d> m_TransformedInputs;
    std::vector<double> m_CachedValues;
    uint8_t* m_PoolData = nullptr;
    size_t m_ObjectSize = 0;
    bool m_IsTreeNode = false;
    bool m_UseOutputCache = false;
    bool m_UseTransformedInputs = false;
};

struct ScalarFieldRef
{
    IScalarFieldPool* pool;
    size_t index;
    
    static ScalarFieldRef Null() {return {nullptr, 0};}
    bool IsValid() const {return pool != nullptr;}
    double Value(const Math::Vector3d& v) const {return pool->Value(index, v);}
    double CachedValue() const {return pool->CachedValue(index);}
    INLINE void SetCachedInput(const Math::Vector3d& p) const {pool->SetInput(index, p);}
    
    AnalyticScalarField& operator*() {return pool->operator[](index);}
    const AnalyticScalarField& operator*() const {return pool->operator[](index);}
    
    AnalyticScalarField* GetUnchecked() {return pool->GetUnchecked(index);}
    const AnalyticScalarField* GetUnchecked() const {return pool->GetUnchecked(index);}
    
    HierarchalDistanceFieldNode* GetUncheckedTreeNode() {return pool->GetUncheckedTreeNode(index);}
    const HierarchalDistanceFieldNode* GetUncheckedTreeNode() const {return pool->GetUncheckedTreeNode(index);}
    
    INLINE bool IsTreeNode() const {return pool->IsTreeNode();}
};

class HierarchalDistanceFieldNode;

template <class ScalarField>  requires(std::is_base_of_v<AnalyticScalarField, ScalarField>)
class ScalarFieldPool : public IScalarFieldPool
{
public:
    ScalarFieldPool() : IScalarFieldPool()
    {
        m_IsTreeNode = std::is_base_of_v<HierarchalDistanceFieldNode, ScalarField>;
        m_PoolData = m_Pool.Data();
        m_ObjectSize = sizeof(ScalarField);
    }
    
    // Perform distance field evaluation
    double Value(size_t Index, const Math::Vector3d& v) const override
    {
#ifndef CONFIG_RELEASE
        AssertOrError(m_Pool.IsValid(Index));
#endif // CONFIG_RELEASE
        
        return m_Pool[Index].Value(v);
    }
    
    // Perform evaluation in batch, result is stored in cache
    void BatchValues(const Math::Vector3d& v, size_t Start = 0, size_t Count = std::numeric_limits<size_t>::max()) const override
    {
#ifndef CONFIG_RELEASE
        AssertOrError(Start < m_Pool.Size(), "Batch Out of Range")
        AssertOrError(Count == std::numeric_limits<size_t>::max() || Start + Count < m_Pool.Size(), "Batch Out of Range")
        for (size_t i = Start; i < Start + Count; ++i)
        {
            AssertOrError(m_Pool.IsValid(i), "Evaluation on invalid object");
        }
#endif // CONFIG_RELEASE
        
        if (Count == std::numeric_limits<size_t>::max())
        {
            Count = m_Pool.Size() - Start;
        }
        
        for (size_t i = Start; i < Start + Count; ++i)
        {
            m_CachedValues[i] = m_Pool[i].Value(v);
        }
    }
    
    // Perform evaluation in batch, result is stored in cache
    void BatchValues(std::span<const Math::Vector3d> vs, size_t Start = 0, size_t Count = std::numeric_limits<size_t>::max()) const override
    {
#ifndef CONFIG_RELEASE
        AssertOrError(Start < m_Pool.Size(), "Batch Out of Range")
        AssertOrError(Count == std::numeric_limits<size_t>::max() || Start + Count < m_Pool.Size(), "Batch Out of Range")
        for (size_t i = Start; i < Start + Count; ++i)
        {
            AssertOrError(m_Pool.IsValid(i), "Evaluation on invalid object")
        }
#endif // CONFIG_RELEASE
        
        if (Count == std::numeric_limits<size_t>::max())
        {
            Count = m_Pool.Size() - Start;
        }
        
        for (size_t i = 0; i < Count; ++i)
        {
            m_CachedValues[Start + i] = m_Pool[Start + i].Value(vs[i]);
        }
    }

    // Perform evaluation in batch for a set of indexes, result is stored in cache
    void BatchValues(std::span<const size_t> Indexes, const Math::Vector3d& v) const override
    {
        for (size_t i = 0; i < Indexes.size(); ++i)
        {
            const size_t Index = Indexes[i];
            
#ifndef CONFIG_RELEASE
            AssertOrError(Index < m_Pool.Size(), "Batch element Out of Range")
            AssertOrError(m_Pool.IsValid(i), "Evaluation on invalid object")
#endif // CONFIG_RELEASE
            
            m_CachedValues[Index] = m_Pool[Index].Value(v);
        }
    }
    
    // Perform evaluation in batch for a set of indexes, result is stored in cache
    void BatchValues(std::span<const size_t> Indexes, std::span<const Math::Vector3d> vs) const override
    {
        for (size_t i = 0; i < Indexes.size(); ++i)
        {
            const size_t Index = Indexes[i];
            
#ifndef CONFIG_RELEASE
            AssertOrError(Index < m_Pool.Size(), "Batch element Out of Range")
            AssertOrError(m_Pool.IsValid(Index), "Evaluation on invalid object")
#endif // CONFIG_RELEASE
            
            m_CachedValues[Index] = m_Pool[Index].Value(vs[i]);
        }
    }

    // Perform evaluation in batch from points of the transformed inputs buffer, result is stored in cache
    void BatchValues(size_t Start = 0, size_t Count = std::numeric_limits<size_t>::max()) const override
    {
#ifndef CONFIG_RELEASE
        AssertOrError(Start < m_Pool.Size(), "Batch Out of Range")
        AssertOrError(Count == std::numeric_limits<size_t>::max() || Start + Count < m_Pool.Size(), "Batch Out of Range")
        for (size_t i = Start; i < Start + Count; ++i)
        {
            AssertOrError(m_Pool.IsValid(i), "Evaluation on invalid object");
        }
#endif // CONFIG_RELEASE
        
        if (Count == std::numeric_limits<size_t>::max())
        {
            Count = m_Pool.Size() - Start;
        }
        
        for (size_t i = Start; i < Start + Count; ++i)
        {
            m_CachedValues[i] = m_Pool[i].Value(m_TransformedInputs[i]);
        }
    }
    
    // Perform evaluation in batch of the transformed inputs buffer for a set of indexes, result is stored in cache
    void BatchValues(std::span<const size_t> Indexes) const override
    {
        for (size_t i = 0; i < Indexes.size(); ++i)
        {
            const size_t Index = Indexes[i];
            
#ifndef CONFIG_RELEASE
            AssertOrError(Index < m_Pool.Size(), "Batch element Out of Range")
            AssertOrError(m_Pool.IsValid(Index), "Evaluation on invalid object")
#endif // CONFIG_RELEASE
            
            m_CachedValues[Index] = m_Pool[Index].Value(m_TransformedInputs[Index]);
        }
    }
    
    AnalyticScalarField& operator[](size_t Index) override
    {
        return m_Pool[Index];
    }
    
    const AnalyticScalarField& operator[](size_t Index) const override
    {
        return m_Pool[Index];
    }
    
    template <typename... Args>
    ScalarFieldRef Add(Args&... params)
    {
        size_t Size = m_Pool.AddEmplace(std::forward<Args>(params)...);
        
        UpdateCache(m_Pool.Data());
        
        return {.pool = this, .index = Size};
    }
    
    void Remove(size_t Index)
    {
        m_Pool.Remove(Index);
    }
    
protected:
    size_t Size() const override
    {
        return m_Pool.Size();
    }

private:
    SparseList<ScalarField> m_Pool;
};

class HierarchalDistanceFieldNode : public AnalyticScalarField
{    
public:
    HierarchalDistanceFieldNode(ScalarFieldRef leftSon = ScalarFieldRef::Null(), ScalarFieldRef rightSon = ScalarFieldRef::Null()) :
        m_LeftSon(leftSon),
        m_RightSon(rightSon)
    {}

    ScalarFieldRef* PopLeftSon();
    ScalarFieldRef* PopLeftRight();
    
    INLINE const ScalarFieldRef& LeftSon() const {return m_LeftSon;}
    INLINE const ScalarFieldRef& RightSon() const {return m_RightSon;}
    
    INLINE bool HasLeftSon() const {return m_LeftSon.IsValid();}
    INLINE bool HasRightSon() const {return m_RightSon.IsValid();}
    
    // Evaluate distance field using cached values from precomputing primitives
    virtual double CachedValue() const = 0;
    
private:
    ScalarFieldRef m_LeftSon;
    ScalarFieldRef m_RightSon;
};

class HDFUnion : public HierarchalDistanceFieldNode
{
private:
    /* data */
public:
    HDFUnion(ScalarFieldRef leftSon, ScalarFieldRef rightSon) :
        HierarchalDistanceFieldNode(leftSon, rightSon)
    {}

    double Value(const Math::Vector3d& v) const override;
    double CachedValue() const override;
};

class HDFIntersection : public HierarchalDistanceFieldNode
{
private:
    /* data */
public:
    HDFIntersection(ScalarFieldRef leftSon, ScalarFieldRef rightSon) :
        HierarchalDistanceFieldNode(leftSon, rightSon)
    {}

    double Value(const Math::Vector3d& v) const override;
    double CachedValue() const override;
};

class HDFDiff: public HierarchalDistanceFieldNode
{
private:
    /* data */
public:
    HDFDiff(ScalarFieldRef leftSon, ScalarFieldRef rightSon) :
        HierarchalDistanceFieldNode(leftSon, rightSon)
    {}

    double Value(const Math::Vector3d& v) const override;
    double CachedValue() const override;
};

class HDFBlend: public HierarchalDistanceFieldNode
{
private:
    double radius;
public:
    HDFBlend(ScalarFieldRef leftSon, ScalarFieldRef rightSon, double blendRadius = 0.5) :
        HierarchalDistanceFieldNode(leftSon, rightSon), radius(blendRadius)
    {}

    double Value(const Math::Vector3d& v) const override;
    double CachedValue() const override;
};

class HDFSmoothUnion: public HierarchalDistanceFieldNode
{
private:
    double radius;
public:
    HDFSmoothUnion(ScalarFieldRef leftSon, ScalarFieldRef rightSon, double blendRadius = 0.5) :
        HierarchalDistanceFieldNode(leftSon, rightSon), radius(blendRadius)
    {}

    double Value(const Math::Vector3d& v) const override;
    double CachedValue() const override;
};

class HDFTransform: public HierarchalDistanceFieldNode
{
private:
    Math::Transform4d transform;
    Math::Transform4d invTransform;
public:
    HDFTransform(ScalarFieldRef leftSon, const Math::Transform4d& t) :
        HierarchalDistanceFieldNode(leftSon),
        transform(t),
        invTransform(Inverse(t))
    {
    }

    double Value(const Math::Vector3d& v) const override;
    double CachedValue() const override;
    
    Math::Vector3d LocalToWorld(const Math::Vector3d& v) const;
    Math::Vector3d WorldToLocal(const Math::Vector3d& v) const;
};

class HDFTree: public AnalyticScalarField
{
public:
    enum class TraversalType
    {
        // Traverse all nodes of the tree as it is typically done traversing a tree
        Tree = 0,
        
        // Precomputing primitives before traversing the tree
        BatchAssemble,
        
        // Traverse all nodes of the tree as it is typically done traversing a tree
        // Use a bound check to avoid exploring out of bounds nodes
        BVHTree,
        
        // Precomputing primitives before traversing the tree
        // Use a bound check to avoid exploring out of bounds nodes
        BVHBatchAssemble,
    };
    void SetTraversalType(TraversalType traversalType);
    
    double Value(const Math::Vector3d& Point) const override;

private:
    ScalarFieldRef m_Head;
    
    // Primitives (leafs)
    ScalarFieldPool<SDFSphere> m_Spheres;
    ScalarFieldPool<SDFCapsule> m_Capsules;
    ScalarFieldPool<SDFSphere> m_SDFBoxes;
    ScalarFieldPool<SDFSphere> m_SDFTores;
    
    // Nodes
    ScalarFieldPool<HDFUnion> m_HDFUnions;
    ScalarFieldPool<HDFIntersection> m_HDFIntersections;
    ScalarFieldPool<HDFBlend> m_HDFBlend;
    ScalarFieldPool<HDFSmoothUnion> m_HDFSmoothUnions;
    ScalarFieldPool<HDFTransform> m_HDFTransforms;
    
    // Params
    TraversalType m_TraversalType = TraversalType::Tree;
    
    // Infos
    bool m_HasTransforms = false;
};
