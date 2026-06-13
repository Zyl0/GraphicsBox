#pragma once

#include "Implicit.h"

class SDFSphere : public AnalyticScalarField
{
private:
    Math::Vector3d center;
    double radius;
public:
    SDFSphere(double Radius = 1.0, Math::Vector3d Center = Math::Vector3d(0)) : center(Center), radius(Radius) {}

    double Value(const Math::Vector3d& v) const override;
};

class SDFCapsule : public AnalyticScalarField
{
private:
    Math::Vector3d A;
    Math::Vector3d B;
    double radius;
public:
    SDFCapsule(double Radius = 1.0, Math::Vector3d A = Math::Vector3d(0), Math::Vector3d B = Math::Vector3d(0)) :
        A(A),
        B(B),
        radius(Radius)
    {}

    double Value(const Math::Vector3d& v) const override;
};

class SDFBox : public AnalyticScalarField
{
private:
    Math::Vector3d A;
    Math::Vector3d B;
    Math::Vector3d C;
    Math::Vector3d D;
    double radius;
public:
    SDFBox(double Radius = 1.0, Math::Vector3d A = Math::Vector3d(0), Math::Vector3d B = Math::Vector3d(0)) :
        A(A),
        B(B),
        radius(Radius)
    {
        C = (A + B) / 2.;
        D = (B - A) / 2.;
    }

    double Value(const Math::Vector3d& v) const override;
};

class SDFTore : public AnalyticScalarField
{
private:
    Math::Vector3d center;
    double inerRadius;
    double outerRadius;
public:
    SDFTore(double inerRadius = 0.75, double outerRadius = 1.0, Math::Vector3d center = Math::Vector3d(0)) :
        center(center),
        inerRadius(inerRadius),
        outerRadius(outerRadius)
    {
    }

    double Value(const Math::Vector3d& v) const override;
};
