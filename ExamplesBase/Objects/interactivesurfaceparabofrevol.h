#ifndef INTERACTIVESURFACEPARABOFREVOL_H
#define INTERACTIVESURFACEPARABOFREVOL_H

#include "interactivesurfacerevolution.h"

namespace ExamplesBase {

class InteractiveSurfaceParabOfRevolPrivate;

class InteractiveSurfaceParabOfRevol : public InteractiveSurfaceRevolution
{
    DEFINE_STANDARD_RTTIEXT(InteractiveSurfaceParabOfRevol, InteractiveSurfaceRevolution)

public:
    InteractiveSurfaceParabOfRevol();
    ~InteractiveSurfaceParabOfRevol();

    Standard_Real getFocal() const;
    void setFocal(Standard_Real F);

protected:
    Handle(Geom_Curve) getCurve() const Standard_OVERRIDE;

private:
    InteractiveSurfaceParabOfRevolPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveSurfaceParabOfRevol, InteractiveSurfaceRevolution)

}

#endif // INTERACTIVESURFACEPARABOFREVOL_H
