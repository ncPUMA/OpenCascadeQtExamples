#ifndef INTERACTIVESURFACECIRCLEOFREVOL_H
#define INTERACTIVESURFACECIRCLEOFREVOL_H

#include "interactivesurfacerevolution.h"

namespace ExamplesBase {

class InteractiveSurfaceCircleOfRevolPrivate;

class InteractiveSurfaceCircleOfRevol : public InteractiveSurfaceRevolution
{
    DEFINE_STANDARD_RTTIEXT(InteractiveSurfaceCircleOfRevol, InteractiveSurfaceRevolution)

public:
    InteractiveSurfaceCircleOfRevol();
    ~InteractiveSurfaceCircleOfRevol();

    Standard_Real getRadius() const;
    void setRadius(Standard_Real R);

protected:
    Handle(Geom_Curve) getCurve() const Standard_OVERRIDE;

private:
    InteractiveSurfaceCircleOfRevolPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveSurfaceCircleOfRevol, InteractiveSurfaceRevolution)

}

#endif // INTERACTIVESURFACECIRCLEOFREVOL_H
