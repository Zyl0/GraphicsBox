#include "GLTF/SceneLoader.h"

namespace  GLTF
{
    Transform::Transform(const Transform& Other): 
        Type(Other.Type), Value(Math::WorldTransformF())
    {
        switch (Type)
        {
        case Properties:
            Value.asProperties = Other.Value.asProperties;
            break;
            
        case Matrix:
            Value.asMatrix = Other.Value.asMatrix;
            break;
            
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
        }
    }

    Transform::Transform(Transform&& Other) noexcept: 
        Type(Other.Type), Value(Math::WorldTransformF())
    {
        switch (Type)
        {
        case Properties:
            Value.asProperties = std::move(Other.Value.asProperties);
            break;
            
        case Matrix:
            Value.asMatrix = std::move(Other.Value.asMatrix);
            break;
            
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
        }
    }

    Transform& Transform::operator=(const Transform& Other)
    {
        if (this == &Other)
            return *this;
        
        Type = Other.Type;
        
        switch (Type)
        {
        case Properties:
            Value.asProperties = Other.Value.asProperties;
            break;
            
        case Matrix:
            Value.asMatrix = Other.Value.asMatrix;
            break;
            
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
        }
            
        return *this;
    }

    Transform& Transform::operator=(Transform&& Other) noexcept
    {
        if (this == &Other)
            return *this;
        
        Type = Other.Type;
            
        switch (Type)
        {
        case Properties:
            Value.asProperties = std::move(Other.Value.asProperties);
            break;
            
        case Matrix:
            Value.asMatrix = std::move(Other.Value.asMatrix);
            break;
            
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
        }
            
        return *this;
    }

}
