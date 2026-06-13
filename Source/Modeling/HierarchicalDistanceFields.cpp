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
    v2.xyz() /= v2.w;
    
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
    v2.xyz() /= v2.w;
    
    return v2.xyz();
}

Math::Vector3d HDFTransform::WorldToLocal(const Math::Vector3d& v) const
{
    Math::Vector4d v2 = invTransform * Math::Vector4d(v, 1.0);
    v2.xyz() /= v2.w;
    
    return v2.xyz();
}

void HDFTree::SetTraversalType(TraversalType traversalType)
{
    switch (traversalType)
    {
    case TraversalType::Tree:
        m_Spheres.SetUseCachedOutputs(false);
        m_Capsules.SetUseCachedOutputs(false);
        m_Boxes.SetUseCachedOutputs(false);
        m_Tores.SetUseCachedOutputs(false);
        if (m_HasTransforms)
        {
            m_Spheres.SetUseTransformedInputs(false);
            m_Capsules.SetUseTransformedInputs(false);
            m_Boxes.SetUseTransformedInputs(false);
            m_Tores.SetUseTransformedInputs(false);
        }
        break;
        
    case TraversalType::BatchAssemble:
        m_Spheres.SetUseCachedOutputs(true);
        m_Capsules.SetUseCachedOutputs(true);
        m_Boxes.SetUseCachedOutputs(true);
        m_Tores.SetUseCachedOutputs(true);
        if (m_HasTransforms)
        {
            m_Spheres.SetUseTransformedInputs(true);
            m_Capsules.SetUseTransformedInputs(true);
            m_Boxes.SetUseTransformedInputs(true);
            m_Tores.SetUseTransformedInputs(true);
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
            
// #ifndef CONFIG_RELEASE
//             const HierarchalDistanceFieldNode* AsTree = dynamic_cast<const HierarchalDistanceFieldNode*>(&(m_Head.operator*()));
//             AssertOrErrorCall(AsTree != nullptr, return 1.0f, "Scalar field marked as tree but does not inherit from HierarchalDistanceFieldNode")
// #else
//             const HierarchalDistanceFieldNode* AsTree = reinterpret_cast<const HierarchalDistanceFieldNode*>(&(m_Head.operator*()));
// #endif // CONFIG_RELEASE
            
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
                m_Boxes.BatchValues();
                m_Tores.BatchValues();
            }
            else
            {
                // Step 1: Precompute primitives
                m_Spheres.BatchValues(Point);
                m_Capsules.BatchValues(Point);
                m_Boxes.BatchValues(Point);
                m_Tores.BatchValues(Point);
            }

            // Final Step: Travel tree using cached results
            {
                struct StackData
                {
                    ScalarFieldRef Field;
                    enum Stage {
                        Left = 0,
                        Right,
                        Eval
                    } stage;
                };
                std::stack<StackData> m_TravelStack;
                m_TravelStack.push({m_Head, StackData::Left});
            
                while (!m_TravelStack.empty())
                {
                traversal_begin:
                    ScalarFieldRef Field = m_TravelStack.top().Field;
                    
                    if (Field.IsValid())
                    {                            
                        if (Field.IsTreeNode())
                        {
                            HierarchalDistanceFieldNode* Node = Field.GetUncheckedTreeNode();

                            switch (m_TravelStack.top().stage)
                            {
                            case StackData::Left:
                                m_TravelStack.top().stage = StackData::Right;
                                m_TravelStack.push({Node->LeftSon(),  StackData::Left});
                                goto traversal_begin;
                            
                            case StackData::Right:
                                m_TravelStack.top().stage = StackData::Eval;
                                m_TravelStack.push({Node->RightSon(), StackData::Left});
                                goto traversal_begin;

                            case StackData::Eval:
                                Field.SetCachedOutput(Node->CachedValue());
                                break;

                            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported traversal stage")
                            }
                        }
                    }
                    
                    m_TravelStack.pop();
                }
            }
            return m_Head.CachedValue();
        }
        
    case TraversalType::BVHTree:
    case TraversalType::BVHBatchAssemble:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported traversal type")
    }
}

ScalarFieldRef HDFTree::AddSphere(double Radius, Math::Vector3d Center)
{
    return m_Spheres.Add(Radius, Center);
}

ScalarFieldRef HDFTree::AddCapsule(double Radius, Math::Vector3d A, Math::Vector3d B)
{
    return m_Capsules.Add(Radius, A, B);
}

ScalarFieldRef HDFTree::AddBox(double Radius, Math::Vector3d A, Math::Vector3d B)
{
    return m_Boxes.Add(Radius, A, B);
}

ScalarFieldRef HDFTree::AddTore(double inerRadius, double outerRadius, Math::Vector3d center)
{
    return m_Tores.Add(inerRadius, outerRadius, center);
}

ScalarFieldRef HDFTree::AddHDFUnion(ScalarFieldRef Left, ScalarFieldRef Right)
{
    return m_HDFUnions.Add(Left, Right);
}

ScalarFieldRef HDFTree::AddHDFIntersection(ScalarFieldRef Left, ScalarFieldRef Right)
{
    return m_HDFIntersections.Add(Left, Right);
}

ScalarFieldRef HDFTree::AddHDFDifference(ScalarFieldRef Left, ScalarFieldRef Right)
{
    return m_HDFDifference.Add(Left, Right);
}

ScalarFieldRef HDFTree::AddHDFSmoothUnion(ScalarFieldRef Left, ScalarFieldRef Right, double blendRadius)
{
    return m_HDFSmoothUnion.Add(Left, Right);
}

ScalarFieldRef HDFTree::AddHDFSmoothDifference(ScalarFieldRef Left, ScalarFieldRef Right, double blendRadius)
{
    return m_HDFSmoothDifferences.Add(Left, Right);
}

ScalarFieldRef HDFTree::AddTransform(ScalarFieldRef Transformee, const Math::Transform4d& Transform)
{
    return m_HDFTransforms.Add(Transformee, Transform);
}

ScalarFieldRef HDFTree::GetSphereRef(size_t Index)
{
    if (Index < m_Spheres.Size()) return {&m_Spheres, Index};
    return ScalarFieldRef::Null();
}

ScalarFieldRef HDFTree::GetCapsuleRef(size_t Index)
{
    if (Index < m_Capsules.Size()) return {&m_Capsules, Index};
    return ScalarFieldRef::Null();
}

ScalarFieldRef HDFTree::GetBoxRef(size_t Index)
{
    if (Index < m_Boxes.Size()) return {&m_Boxes, Index};
    return ScalarFieldRef::Null();
}

ScalarFieldRef HDFTree::GetToreRef(size_t Index)
{
    if (Index < m_Tores.Size()) return {&m_Tores, Index};
    return ScalarFieldRef::Null();
}

ScalarFieldRef HDFTree::GetHDFUnionRef(size_t Index)
{
    if (Index < m_HDFUnions.Size()) return {&m_HDFUnions, Index};
    return ScalarFieldRef::Null();
}

ScalarFieldRef HDFTree::GetHDFIntersectionRef(size_t Index)
{
    if (Index < m_HDFIntersections.Size()) return {&m_HDFIntersections, Index};
    return ScalarFieldRef::Null();
}

ScalarFieldRef HDFTree::GetHDFDifferenceRef(size_t Index)
{
    if (Index < m_HDFDifference.Size()) return {&m_HDFDifference, Index};
    return ScalarFieldRef::Null();
}

ScalarFieldRef HDFTree::GetHDFSmoothUnionRef(size_t Index)
{
    if (Index < m_HDFSmoothUnion.Size()) return {&m_HDFSmoothUnion, Index};
    return ScalarFieldRef::Null();
}

ScalarFieldRef HDFTree::GetHDFSmoothDifferenceRef(size_t Index)
{
    if (Index < m_HDFSmoothDifferences.Size()) return {&m_HDFSmoothDifferences, Index};
    return ScalarFieldRef::Null();
}

ScalarFieldRef HDFTree::GetTransformRef(size_t Index)
{
    if (Index < m_HDFTransforms.Size()) return {&m_HDFTransforms, Index};
    return ScalarFieldRef::Null();
}

SDFSphere& HDFTree::GetSphere(size_t Index)
{
    AssertOrError(Index < m_Spheres.Size(), "Index out of range")
    return m_Spheres.Get(Index);
}

SDFCapsule& HDFTree::GetCapsule(size_t Index)
{
    AssertOrError(Index < m_Capsules.Size(), "Index out of range")
    return m_Capsules.Get(Index);
}

SDFBox& HDFTree::GetBox(size_t Index)
{
    AssertOrError(Index < m_Boxes.Size(), "Index out of range")
    return m_Boxes.Get(Index);
}

SDFTore& HDFTree::GetTore(size_t Index)
{
    AssertOrError(Index < m_Tores.Size(), "Index out of range")
    return m_Tores.Get(Index);
}

HDFUnion& HDFTree::GetHDFUnion(size_t Index)
{
    AssertOrError(Index < m_HDFUnions.Size(), "Index out of range")
    return m_HDFUnions.Get(Index);
}

HDFIntersection& HDFTree::GetHDFIntersection(size_t Index)
{
    AssertOrError(Index < m_HDFIntersections.Size(), "Index out of range")
    return m_HDFIntersections.Get(Index);
}

HDFDiff& HDFTree::GetHDFDifference(size_t Index)
{
    AssertOrError(Index < m_HDFDifference.Size(), "Index out of range")
    return m_HDFDifference.Get(Index);
}

HDFBlend& HDFTree::GetHDFSmoothUnion(size_t Index)
{
    AssertOrError(Index < m_HDFSmoothUnion.Size(), "Index out of range")
    return m_HDFSmoothUnion.Get(Index);
}

HDFSmoothUnion& HDFTree::GetHDFSmoothDifference(size_t Index)
{
    AssertOrError(Index < m_HDFSmoothDifferences.Size(), "Index out of range")
    return m_HDFSmoothDifferences.Get(Index);
}

HDFTransform& HDFTree::GetTransform(size_t Index)
{
    AssertOrError(Index < m_HDFTransforms.Size(), "Index out of range")
    return m_HDFTransforms.Get(Index);
}

const SDFSphere& HDFTree::GetSphere(size_t Index) const
{
    AssertOrError(Index < m_Spheres.Size(), "Index out of range")
    return m_Spheres.Get(Index);
}

const SDFCapsule& HDFTree::GetCapsule(size_t Index) const
{
    AssertOrError(Index < m_Capsules.Size(), "Index out of range")
    return m_Capsules.Get(Index);
}

const SDFBox& HDFTree::GetBox(size_t Index) const
{
    AssertOrError(Index < m_Boxes.Size(), "Index out of range")
    return m_Boxes.Get(Index);
}

const SDFTore& HDFTree::GetTore(size_t Index) const
{
    AssertOrError(Index < m_Tores.Size(), "Index out of range")
    return m_Tores.Get(Index);
}

const HDFUnion& HDFTree::GetHDFUnion(size_t Index) const
{
    AssertOrError(Index < m_HDFUnions.Size(), "Index out of range")
    return m_HDFUnions.Get(Index);
}

const HDFIntersection& HDFTree::GetHDFIntersection(size_t Index) const
{
    AssertOrError(Index < m_HDFIntersections.Size(), "Index out of range")
    return m_HDFIntersections.Get(Index);
}

const HDFDiff& HDFTree::GetHDFDifference(size_t Index) const
{
    AssertOrError(Index < m_HDFDifference.Size(), "Index out of range")
    return m_HDFDifference.Get(Index);
}

const HDFBlend& HDFTree::GetHDFSmoothUnion(size_t Index) const
{
    AssertOrError(Index < m_HDFSmoothUnion.Size(), "Index out of range")
    return m_HDFSmoothUnion.Get(Index);
}

const HDFSmoothUnion& HDFTree::GetHDFSmoothDifference(size_t Index) const
{
    AssertOrError(Index < m_HDFSmoothDifferences.Size(), "Index out of range")
    return m_HDFSmoothDifferences.Get(Index);
}

const HDFTransform& HDFTree::GetTransform(size_t Index) const
{
    AssertOrError(Index < m_HDFTransforms.Size(), "Index out of range")
    return m_HDFTransforms.Get(Index);
}

void HDFTree::RemoveSphere(ScalarFieldRef Element)
{
    AssertOrErrorCall(Element.IsValid(), return;, "Tried to remove an invalid element")
    AssertOrErrorCall(Element.index < m_Spheres.Size(), return;, "Index out of range")
    AssertOrErrorCall((uint64_t)(Element.pool) == (uint64_t)(&m_Spheres), return;, "Tried to remove an element of the wrong type")

    m_Spheres.Remove(Element.index);
}

void HDFTree::RemoveCapsule(ScalarFieldRef Element)
{
    AssertOrErrorCall(Element.IsValid(), return;, "Tried to remove an invalid element")
    AssertOrErrorCall(Element.index < m_Capsules.Size(), return;, "Index out of range")
    AssertOrErrorCall((uint64_t)(Element.pool) == (uint64_t)(&m_Capsules), return;, "Tried to remove an element of the wrong type")

    m_Capsules.Remove(Element.index);
}

void HDFTree::RemoveBox(ScalarFieldRef Element)
{
    AssertOrErrorCall(Element.IsValid(), return;, "Tried to remove an invalid element")
    AssertOrErrorCall(Element.index < m_Boxes.Size(), return;, "Index out of range")
    AssertOrErrorCall((uint64_t)(Element.pool) == (uint64_t)(&m_Boxes), return;, "Tried to remove an element of the wrong type")

    m_Boxes.Remove(Element.index);
}

void HDFTree::RemoveTore(ScalarFieldRef Element)
{
    AssertOrErrorCall(Element.IsValid(), return;, "Tried to remove an invalid element")
    AssertOrErrorCall(Element.index < m_Tores.Size(), return;, "Index out of range")
    AssertOrErrorCall((uint64_t)(Element.pool) == (uint64_t)(&m_Tores), return;, "Tried to remove an element of the wrong type")

    m_Tores.Remove(Element.index);
}

void HDFTree::RemoveHDFUnion(ScalarFieldRef Element)
{
    AssertOrErrorCall(Element.IsValid(), return;, "Tried to remove an invalid element")
    AssertOrErrorCall(Element.index < m_HDFUnions.Size(), return;, "Index out of range")
    AssertOrErrorCall((uint64_t)(Element.pool) == (uint64_t)(&m_HDFUnions), return;, "Tried to remove an element of the wrong type")

    m_HDFUnions.Remove(Element.index);
}

void HDFTree::RemoveHDFIntersection(ScalarFieldRef Element)
{
    AssertOrErrorCall(Element.IsValid(), return;, "Tried to remove an invalid element")
    AssertOrErrorCall(Element.index < m_HDFIntersections.Size(), return;, "Index out of range")
    AssertOrErrorCall((uint64_t)(Element.pool) == (uint64_t)(&m_HDFIntersections), return;, "Tried to remove an element of the wrong type")

    m_HDFIntersections.Remove(Element.index);
}

void HDFTree::RemoveHDFDifference(ScalarFieldRef Element)
{
    AssertOrErrorCall(Element.IsValid(), return;, "Tried to remove an invalid element")
    AssertOrErrorCall(Element.index < m_HDFDifference.Size(), return;, "Index out of range")
    AssertOrErrorCall((uint64_t)(Element.pool) == (uint64_t)(&m_HDFDifference), return;, "Tried to remove an element of the wrong type")

    m_HDFDifference.Remove(Element.index);
}

void HDFTree::RemoveHDFSmoothUnion(ScalarFieldRef Element)
{
    AssertOrErrorCall(Element.IsValid(), return;, "Tried to remove an invalid element")
    AssertOrErrorCall(Element.index < m_HDFSmoothUnion.Size(), return;, "Index out of range")
    AssertOrErrorCall((uint64_t)(Element.pool) == (uint64_t)(&m_HDFSmoothUnion), return;, "Tried to remove an element of the wrong type")

    m_HDFSmoothUnion.Remove(Element.index);
}

void HDFTree::RemoveHDFSmoothDifference(ScalarFieldRef Element)
{
    AssertOrErrorCall(Element.IsValid(), return;, "Tried to remove an invalid element")
    AssertOrErrorCall(Element.index < m_HDFSmoothDifferences.Size(), return;, "Index out of range")
    AssertOrErrorCall((uint64_t)(Element.pool) == (uint64_t)(&m_HDFSmoothDifferences), return;, "Tried to remove an element of the wrong type")

    m_HDFSmoothDifferences.Remove(Element.index);
}

void HDFTree::RemoveTransform(ScalarFieldRef Element)
{
    AssertOrErrorCall(Element.IsValid(), return;, "Tried to remove an invalid element")
    AssertOrErrorCall(Element.index < m_HDFTransforms.Size(), return;, "Index out of range")
    AssertOrErrorCall((uint64_t)(Element.pool) == (uint64_t)(&m_HDFTransforms), return;, "Tried to remove an element of the wrong type")

    m_HDFTransforms.Remove(Element.index);
}

void HDFTree::Clear()
{
    m_Spheres.CLear();
    m_Capsules.CLear();
    m_Boxes.CLear();
    m_Tores.CLear();
    m_HDFUnions.CLear();
    m_HDFIntersections.CLear();
    m_HDFDifference.CLear();
    m_HDFSmoothUnion.CLear();
    m_HDFSmoothDifferences.CLear();
    m_HDFTransforms.CLear();
}

void HDFTree::ShrinkToFit()
{
    m_Spheres.ShrinkToFit();
    m_Capsules.ShrinkToFit();
    m_Boxes.ShrinkToFit();
    m_Tores.ShrinkToFit();
    m_HDFUnions.ShrinkToFit();
    m_HDFIntersections.ShrinkToFit();
    m_HDFDifference.ShrinkToFit();
    m_HDFSmoothUnion.ShrinkToFit();
    m_HDFSmoothDifferences.ShrinkToFit();
    m_HDFTransforms.ShrinkToFit();
}

void HDFTree::SetHead(ScalarFieldRef Head)
{
    if (!Head.IsValid()) return;

    m_Head = Head;
}
