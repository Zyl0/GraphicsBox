#pragma once

#include <cfloat>

#include "Functons.h"
#include "Vector.h"
#include "Plane.h"

#include <type_traits>

namespace Math
{
    template<typename type>
    type DistancePointLine(const Point3t<type> &q, const Point3t<type> &p, const Vector3t<type> &v)
    {
        Vector3t<type> a = Cross(q - p, v);
        return sqrt(SquareMagnitude(a) / SquareMagnitude(v));
    }

    template<typename type>
    type DistanceLineLine(const Point3t<type> &p1, const Vector3t<type> &v1, const Point3t<type> &p2, const Vector3t<type> &v2)
    {
        Vector3t<type> dp = p2 - p1;

        type v12 = SquareMagnitude(v1);
        type v22 = SquareMagnitude(v2);
        type v1v2 = Dot(v1, v2);

        type det = v1v2 * v1v2 - v12 * v22;

        if constexpr(std::is_same_v<type, float>)
        {
            if(fabs(det) > FLT_MIN)
            {
                float dpv1 = Dot(dp, v1);
                float dpv2 = Dot(dp, v2);
                float t1 = (v1v2 * dpv2 -  v22 * dpv1) * det;
                float t2 = (v12  * dpv2 - v1v2 * dpv1) * det;

                return Magnitude(dp + v2 * t2 - v1 * t1);
            }
        }
        else if constexpr(std::is_same_v<type, double>)
        {
            if(fabs(det) > DBL_MIN)
            {
                double dpv1 = Dot(dp, v1);
                double dpv2 = Dot(dp, v2);
                double t1 = (v1v2 * dpv2 -  v22 * dpv1) * det;
                double t2 = (v12  * dpv2 - v1v2 * dpv1) * det;

                return Magnitude(dp + v2 * t2 - v1 * t1);
            }
        }
        else if constexpr(std::is_same_v<type, long double>)
        {
            if(fabs(det) > LDBL_MIN)
            {
                long double dpv1 = Dot(dp, v1);
                long double dpv2 = Dot(dp, v2);
                long double t1 = (v1v2 * dpv2 -  v22 * dpv1) * det;
                long double t2 = (v12  * dpv2 - v1v2 * dpv1) * det;

                return Magnitude(dp + v2 * t2 - v1 * t1);
            }
        }
        else 
        {
            if(Abs(det) > static_cast<type>(0))
            {
                type dpv1 = Dot(dp, v1);
                type dpv2 = Dot(dp, v2);
                type t1 = (v1v2 * dpv2 -  v22 * dpv1) * det;
                type t2 = (v12  * dpv2 - v1v2 * dpv1) * det;

                return Magnitude(dp + v2 * t2 - v1 * t1);
            }
        }
        
        // The lines are nearly parallel
        Vector3t<type> a = Cross(dp, v1);
        return sqrt(SquareMagnitude(a) / SquareMagnitude(v12)); 
    }

    template<typename type>
    bool IntersectionLinePlan(const Point3t<type> &p, const Vector3t<type> &v, const PlaneT<type> &f, Point3t<type> *q)
    {
        type fv = Dot(f, v);

        if constexpr(std::is_same_v<type, float>)
        {
            if(fabs(fv) > FLT_MIN)
            {
                *q = p - v * (Dot(f, p) / fv);
                return true;  
            }
        }
        else if constexpr(std::is_same_v<type, double>)
        {
            if(fabs(fv) > DBL_MIN)
            {
                *q = p - v * (Dot(f, p) / fv);
                return true; 
            }
        }
        else if constexpr(std::is_same_v<type, long double>)
        {
            if(fabs(fv) > LDBL_MIN)
            {
                *q = p - v * (Dot(f, p) / fv);
                return true; 
            }
        }
        else 
        {
            if(Abs(fv) > static_cast<type>(0))
            {
                *q = p - v * (Dot(f, p) / fv);
                return true; 
            }
        }
        
        return false; 
    }

    template<typename type>
    bool IntersectionThreePlan(const PlaneT<type> &f1, const PlaneT<type> &f2, const PlaneT<type> &f3, Point3t<type> *p)
    {
        const Vector3t<type> &n1 = f1.GetNormal();
        const Vector3t<type> &n2 = f2.GetNormal();
        const Vector3t<type> &n3 = f3.GetNormal();

        Vector3t<type> n1xn2 = Cross(n1, n2);
        type det = Dot(n1xn2, n3);
        if constexpr(std::is_same_v<type, float>)
        {
            if(fabs(det) > FLT_MIN)
            {
                *p = (Cross(n3, n2) * f1.w + Cross(n1, n3) * f2.w - n1xn2 * f3.w) / det;
                return true;  
            }
        }
        else if constexpr(std::is_same_v<type, double>)
        {
            if(fabs(det) > DBL_MIN)
            {
                *p = (Cross(n3, n2) * f1.w + Cross(n1, n3) * f2.w - n1xn2 * f3.w) / det;
                return true; 
            }
        }
        else if constexpr(std::is_same_v<type, long double>)
        {
            if(fabs(det) > LDBL_MIN)
            {
                *p = (Cross(n3, n2) * f1.w + Cross(n1, n3) * f2.w - n1xn2 * f3.w) / det;
                return true; 
            }
        }
        else 
        {
            if(Abs(det) > static_cast<type>(0))
            {
                *p = (Cross(n3, n2) * f1.w + Cross(n1, n3) * f2.w - n1xn2 * f3.w) / det;
                return true; 
            }
        }
        
        return false; 
    }

    template<typename type>
    bool IntersectionTwoPlan(const PlaneT<type> &f1, const PlaneT<type> &f2, Point3t<type> *p, Vector3t<type> *v)
    {
        const Vector3t<type> &n1 = f1.GetNormal();
        const Vector3t<type> &n2 = f2.GetNormal();

        *v = Cross(n1, n2);
        type det = SquareMagnitude(*v);
        if constexpr(std::is_same_v<type, float>)
        {
            if(fabs(det) > FLT_MIN)
            {
                *p = (Cross(*v, n2) * f1.w + Cross(n1, *v) * f2.w) / det;
                return true;  
            }
        }
        else if constexpr(std::is_same_v<type, double>)
        {
            if(fabs(det) > DBL_MIN)
            {
                *p = (Cross(*v, n2) * f1.w + Cross(n1, *v) * f2.w) / det;
                return true; 
            }
        }
        else if constexpr(std::is_same_v<type, long double>)
        {
            if(fabs(det) > LDBL_MIN)
            {
                *p = (Cross(*v, n2) * f1.w + Cross(n1, *v) * f2.w) / det;
                return true; 
            }
        }
        else 
        {
            if(Abs(det) > static_cast<type>(0))
            {
                *p = (Cross(*v, n2) * f1.w + Cross(n1, *v) * f2.w) / det;
                return true; 
            }
        }
        
        return false; 
    }
}
 