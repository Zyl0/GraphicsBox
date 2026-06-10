#include "Modeling/HierarchicalDistanceFields.h"

static double G(double a, double b, double r = 0.25)
{
    double h = std::max(r - std::abs(a - b), 0.) / r;
    return (1./6.) * r * (h*h*h);  
}

double HDFUnion::Value(const Math::Vector3d& v) const
{
    return std::min(LeftSon().Value(v), LeftSon().Value(v));
}

double HDFUnion::CachedValue() const
{
    return std::min(LeftSon().CachedValue(), LeftSon().CachedValue());
}

double HDFIntersection::Value(const Math::Vector3d& v) const
{
    return std::max(LeftSon().Value(v), LeftSon().Value(v));
}

double HDFIntersection::CachedValue() const
{
    return std::max(LeftSon().CachedValue(), LeftSon().CachedValue());
}

double HDFDiff::Value(const Math::Vector3d& v) const
{
    return std::max(LeftSon().Value(v), -(LeftSon().Value(v)));
}

double HDFDiff::CachedValue() const
{
    return std::max(LeftSon().CachedValue(), -(LeftSon().CachedValue()));
}

double HDFBlend::Value(const Math::Vector3d& v) const
{
    double a = LeftSon().Value(v), b = LeftSon().Value(v);
    
    double g = G(a, b, radius);

    return std::min(a, b) - g;
}

double HDFBlend::CachedValue() const
{
    double a = LeftSon().CachedValue(), b = LeftSon().CachedValue();
    
    double g = G(a, b, radius);

    return std::min(a, b) - g;
}

double HDFSmoothUnion::Value(const Math::Vector3d& v) const
{
    double a = LeftSon().Value(v), b = -(LeftSon().Value(v));
    
    double g = G(a, b, radius);

    return std::max(a, b) + g;
}

double HDFSmoothUnion::CachedValue() const
{
    double a = LeftSon().CachedValue(), b = -(LeftSon().CachedValue());
    
    double g = G(a, b, radius);

    return std::max(a, b) + g;
}

double HDFTransform::Value(const Math::Vector3d& v) const
{
    Math::Vector4d v2 = invTransform * Math::Vector4d(v, 1.0);
    v2.xyz() /= v2.w();
    
    return LeftSon().Value(v2.xyz());
}

double HDFTransform::CachedValue() const
{
    // Transformation has already happened by now
    return LeftSon().CachedValue();
}

Math::Vector3d HDFTransform::LocalToWorld(const Math::Vector3d& v) const
{
    Math::Vector4d v2 = transform * Math::Vector4d(v, 1.0);
    v2.xyz() /= v2.w();
    
    return v2.xyz();
}

Math::Vector3d HDFTransform::WorldToLocal(const Math::Vector3d& v) const
{
    Math::Vector4d v2 = invTransform * Math::Vector4d(v, 1.0);
    v2.xyz() /= v2.w();
    
    return v2.xyz();
}

void HDFTree::SetTraversalType(TraversalType traversalType)
{
    switch (traversalType)
    {
    case TraversalType::Tree:
        m_Spheres.SetUseCachedOutputs(false);
        m_Capsules.SetUseCachedOutputs(false);
        m_SDFBoxes.SetUseCachedOutputs(false);
        m_SDFTores.SetUseCachedOutputs(false);
        if (m_HasTransforms)
        {
            m_Spheres.SetUseTransformedInputs(false);
            m_Capsules.SetUseTransformedInputs(false);
            m_SDFBoxes.SetUseTransformedInputs(false);
            m_SDFTores.SetUseTransformedInputs(false);
        }
        break;
        
    case TraversalType::BatchAssemble:
        m_Spheres.SetUseCachedOutputs(true);
        m_Capsules.SetUseCachedOutputs(true);
        m_SDFBoxes.SetUseCachedOutputs(true);
        m_SDFTores.SetUseCachedOutputs(true);
        if (m_HasTransforms)
        {
            m_Spheres.SetUseTransformedInputs(true);
            m_Capsules.SetUseTransformedInputs(true);
            m_SDFBoxes.SetUseTransformedInputs(true);
            m_SDFTores.SetUseTransformedInputs(true);
        }
        break;
        
    case TraversalType::BVHTree:
    case TraversalType::BVHBatchAssemble:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported traversal type")
    }
    
    m_TraversalType = traversalType;
}

double HDFTree::Value(const Math::Vector3d& Point) const
{
    switch (m_TraversalType)
    {
    case TraversalType::Tree:
        return m_Head.IsValid() ? m_Head.Value(Point) : 1.0f;
        
    case TraversalType::BatchAssemble:
        {
            if (!m_Head.IsValid()) return 1.0f;
            
            if (!m_Head.IsTreeNode()) return m_Head.Value(Point);
            
#ifndef CONFIG_RELEASE
            const HierarchalDistanceFieldNode* AsTree = dynamic_cast<const HierarchalDistanceFieldNode*>(&(m_Head.operator*()));
            AssertOrErrorCall(AsTree != nullptr, return 1.0f, "Scalar field marked as tree but does not inherit from HierarchalDistanceFieldNode")
#else
            const HierarchalDistanceFieldNode* AsTree = reinterpret_cast<const HierarchalDistanceFieldNode*>(&(m_Head.operator*()));
#endif // CONFIG_RELEASE
            
            if (m_HasTransforms)
            {
                // Step 1: Gather transformation
                // Travel the tree
                // The whole idea is to avoid making calls to a vtable 
                {
                    struct StackData
                    {
                        ScalarFieldRef Field;
                        Math::Vector3d Point;
                    };
                    std::stack<StackData> m_TravelStack;
                    m_TravelStack.push({m_Head, Point});
                
                    while (!m_TravelStack.empty())
                    {
                        ScalarFieldRef Field = m_TravelStack.top().Field;
                        
                        if (Field.IsValid())
                        {                            
                            if (Field.IsTreeNode())
                            {
                                HierarchalDistanceFieldNode* Node = Field.GetUncheckedTreeNode();
                                
                                if ((uintptr_t)(Field.pool) == (uintptr_t)(&m_HDFTransforms))
                                {
#ifndef CONFIG_RELEASE
                                    const HDFTransform* AsTransform = dynamic_cast<const HDFTransform*>(Field.GetUncheckedTreeNode());
                                    AssertOrErrorCall(AsTransform != nullptr, return 1.0f, "Scalar field from transform pool but does not inherit from HDFTransform")
    #else
                                    const HDFTransform* AsTransform = reinterpret_cast<const HDFTransform*>(Field.GetUncheckedTreeNode());
#endif // CONFIG_RELEASE
                                
                                    Math::Vector3d P =  AsTransform-> WorldToLocal(m_TravelStack.top().Point);
                                    
                                    m_TravelStack.push({Node->LeftSon(),  P});
                                    
                                    // Transforms only have left sides
                                    // m_TravelStack.push({Node->RightSon(), P});
                                }
                                else
                                {
                                    m_TravelStack.push({Node->LeftSon(),  m_TravelStack.top().Point});
                                    m_TravelStack.push({Node->RightSon(), m_TravelStack.top().Point});
                                }
                            }
                            else
                            {
                                Field.SetCachedInput(m_TravelStack.top().Point);
                            }
                        }
                        
                        m_TravelStack.pop();
                    }
                }
                
                // Step 2: Precompute primitives reading from the pre transformed inputs
                m_Spheres.BatchValues();
                m_Capsules.BatchValues();
                m_SDFBoxes.BatchValues();
                m_SDFTores.BatchValues();
        
                // Step 3: Travel tree using cached results
                return AsTree->CachedValue();
            }
            else
            {
                // Step 1: Precompute primitives
                m_Spheres.BatchValues(Point);
                m_Capsules.BatchValues(Point);
                m_SDFBoxes.BatchValues(Point);
                m_SDFTores.BatchValues(Point);
        
                // Step 2: Travel tree using cached results
                return AsTree->CachedValue();
            }
        }
        
    case TraversalType::BVHTree:
    case TraversalType::BVHBatchAssemble:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported traversal type")
    }
}
