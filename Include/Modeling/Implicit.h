#pragma once

#include <optional>

#include "Math/Vector.h"
#include "Math/Box.h"
#include "Mesh.h"

class AnalyticScalarField
{
public:
    virtual ~AnalyticScalarField() = default;
    virtual double Value(const Math::Vector3d& Point) const = 0;
    virtual float Value(const Math::Vector3f& Point) const;
};

Math::Vector3d Gradiant(const AnalyticScalarField& ScalarField, const Math::Vector3d& Point);
Math::Vector3d Dichotomy(const AnalyticScalarField& ScalarField, Math::Vector3d A, Math::Vector3d B, double va, double vb, double length, double epsilon);
Math::Vector3d Normal(const AnalyticScalarField& ScalarField, const Math::Vector3d& Point);
Mesh Polygonise(const AnalyticScalarField& ScalarField, const Math::Box3d& Region, size_t Resolution, double epsilon = 1.0e-4);
void Polygonise(const AnalyticScalarField& ScalarField, const Math::Box3d& Region, size_t Resolution, Mesh& Mesh, double epsilon = 1.0e-4);




