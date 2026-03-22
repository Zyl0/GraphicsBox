#include "Modules/Rendering/World/Mesh.h"

namespace Rendering::World
{
    bool frustumCullingTest(
    const Math::Matrix4f &ViewProj,
    const Math::Matrix4f &Model,
    Math::Point3f boundMin, Math::Point3f boundMax)
    {
        //todo fix

        Math::Matrix4f MVP = ViewProj * Model;
        Math::Matrix4f InverseMVP = Inverse(MVP);
        
        static const Math::Point3f frustum[8]= { 
            { -1, -1, -1 },
            { -1, -1, 1 },
            { -1, 1, -1 },
            { -1, 1, 1 },
            { 1, -1, -1 },
            { 1, -1, 1 },
            { 1, 1, -1 },
            { 1, 1, 1 },
        };

        Math::Point3f bounds[8]= { 
            { boundMin.x, boundMin.y, boundMin.z },
            { boundMin.x, boundMin.y,  boundMax.z },
            { boundMin.x, boundMax.y, boundMin.z },
            { boundMin.x, boundMax.y,  boundMax.z },
            { boundMax.x, boundMin.y, boundMin.z },
            { boundMax.x, boundMin.y,  boundMax.z },
            { boundMax.x,  boundMax.y, boundMin.z },
            { boundMax.x,  boundMax.y,  boundMax.z },
        };
        
        bool areBoundsInFrustum = true;
        {
            bool validPlans[6] = {false};
            for (size_t i = 0; i < 8; i++)
            {
                Math::Vector4f tp = MVP * Math::Vector4f(bounds[i], 1);

                validPlans[0] |= (tp.x > -tp.w);
                validPlans[1] |= (tp.x <  tp.w);
                validPlans[2] |= (tp.y > -tp.w);
                validPlans[3] |= (tp.y <  tp.w);
                validPlans[4] |= (tp.z > -tp.w);
                validPlans[5] |= (tp.z <  tp.w);        
            }
            for (size_t i = 0; i < 6; i++)
            {
                if(!validPlans[i]) 
                    areBoundsInFrustum = false;
            }
        }

        // todo fix
        bool isFrustumInBounds = true;
        {
            bool validPlans[6] = {false};
            for (size_t i = 0; i < 8; i++)
            {
                Math::Vector4f boundPosition = InverseMVP * Math::Vector4f(frustum[i], 1);
                boundPosition /= boundPosition.w;
        
                validPlans[0] |= (boundPosition.x >= boundMin.x);
                validPlans[1] |= (boundPosition.x <= boundMax.x);
                validPlans[2] |= (boundPosition.y >= boundMin.y);
                validPlans[3] |= (boundPosition.y <= boundMax.y);
                validPlans[4] |= (boundPosition.z >= boundMin.z);
                validPlans[5] |= (boundPosition.z <= boundMax.z);
            
            }
            for (size_t i = 0; i < 6; i++)
            {
                if(!validPlans[i]) 
                    isFrustumInBounds = false;
            }
        }
        return areBoundsInFrustum && isFrustumInBounds;
    }
}