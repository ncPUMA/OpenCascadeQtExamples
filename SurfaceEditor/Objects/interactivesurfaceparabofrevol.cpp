#include "interactivesurfaceparabofrevol.h"

#include <Geom_Parabola.hxx>

class InteractiveSurfaceParabOfRevolPrivate
{
    friend class InteractiveSurfaceParabOfRevol;

    Handle(Geom_Curve) getCurve() const
    {
        gp_Ax2 axes;
        return new Geom_Parabola(axes, focal);
    }

    Standard_Real focal = 50.;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveSurfaceParabOfRevol, InteractiveSurfaceRevolution)

InteractiveSurfaceParabOfRevol::InteractiveSurfaceParabOfRevol()
    : InteractiveSurfaceRevolution()
    , d(new InteractiveSurfaceParabOfRevolPrivate)
{
    updateSurface(d->getCurve());
}

InteractiveSurfaceParabOfRevol::~InteractiveSurfaceParabOfRevol()
{
    delete d;
}

Standard_Real InteractiveSurfaceParabOfRevol::getFocal() const
{
    return d->focal;
}

void InteractiveSurfaceParabOfRevol::setFocal(Standard_Real F)
{
    if (F >= 0.) {
        d->focal = F;
        updateSurface(d->getCurve());
    }
}

Handle(Geom_Curve) InteractiveSurfaceParabOfRevol::getCurve() const
{
    return d->getCurve();
}
