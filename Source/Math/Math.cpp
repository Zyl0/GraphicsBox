#include "Math/Box.h"

namespace Math
{
    namespace Box3
    {
        const size_t Edge[24] =
        {
            0,1,2,3,4,5,6,7,
            0,2,1,3,4,6,5,7,
            0,4,1,5,2,6,3,7
        };
        
        const Vector3d Normal[24] = 
        {
            Vector3d(-1.0,0.0,0.0),
            Vector3d(0.0,-1.0,0.0),
            Vector3d(0.0,0.0,-1.0),
            Vector3d(1.0,0.0,0.0),
            Vector3d(0.0, 1.0,0.0),
            Vector3d(0.0,0.0,1.0)
        };
    }
}