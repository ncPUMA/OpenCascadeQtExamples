#ifndef INTERACTIVESURFACEELLIPSEOFREVOL_H
#define INTERACTIVESURFACEELLIPSEOFREVOL_H

#include "interactivesurfacerevolution.h"

namespace ExamplesBase {

class InteractiveSurfaceEllipseOfRevolPrivate;

class InteractiveSurfaceEllipseOfRevol : public InteractiveSurfaceRevolution
{
    DEFINE_STANDARD_RTTIEXT(InteractiveSurfaceEllipseOfRevol, InteractiveSurfaceRevolution)

public:
    InteractiveSurfaceEllipseOfRevol();
    ~InteractiveSurfaceEllipseOfRevol();

    Standard_Real getFocal() const;
    Standard_Real getEccentricity() const;
    void setFocal(Standard_Real F);
    void setEccentricity(Standard_Real eps);

    Standard_Real majorR() const;
    Standard_Real minorR() const;

protected:
    Handle(Geom_Curve) getCurve() const Standard_OVERRIDE;

private:
    InteractiveSurfaceEllipseOfRevolPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveSurfaceEllipseOfRevol, InteractiveSurfaceRevolution)

}

#endif // INTERACTIVESURFACEELLIPSEOFREVOL_H
