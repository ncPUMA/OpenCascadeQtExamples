#ifndef INTERACTIVESURFACEHYPEROFREVOL_H
#define INTERACTIVESURFACEHYPEROFREVOL_H

#include "interactivesurfacerevolution.h"

namespace ExamplesBase {

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
    TopoDS_Shape createShape() const Standard_OVERRIDE;

private:
    friend class InteractiveSurfaceHyperOfRevolPrivate;
    InteractiveSurfaceHyperOfRevolPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveSurfaceHyperOfRevol, InteractiveSurfaceRevolution)

}

#endif // INTERACTIVESURFACEHYPEROFREVOL_H
