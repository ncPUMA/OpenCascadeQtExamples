#ifndef INTERACTIVESURFACEHYPEROFREVOL_H
#define INTERACTIVESURFACEHYPEROFREVOL_H

#include "interactivesurfacerevolution.h"

class InteractiveSurfaceHyperOfRevolPrivate;

class InteractiveSurfaceHyperOfRevol : public InteractiveSurfaceRevolution
{
    DEFINE_STANDARD_RTTIEXT(InteractiveSurfaceHyperOfRevol, InteractiveSurfaceRevolution)

public:
    InteractiveSurfaceHyperOfRevol();
    ~InteractiveSurfaceHyperOfRevol();

    Standard_Real getFocal() const;
    Standard_Real getEccentricity() const;
    void setFocal(Standard_Real F);
    void setEccentricity(Standard_Real eps);

    Standard_Real majorR() const;
    Standard_Real minorR() const;

protected:
    Handle(Geom_Curve) getCurve() const Standard_OVERRIDE;

private:
    InteractiveSurfaceHyperOfRevolPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveSurfaceParabOfRevol, InteractiveSurfaceRevolution)

#endif // INTERACTIVESURFACEHYPEROFREVOL_H
