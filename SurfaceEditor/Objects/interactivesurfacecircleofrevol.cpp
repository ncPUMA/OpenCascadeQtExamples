#include "interactivesurfacecircleofrevol.h"

#include <Geom_Circle.hxx>

class InteractiveSurfaceCircleOfRevolPrivate
{
    friend class InteractiveSurfaceCircleOfRevol;

    Handle(Geom_Curve) getCurve() const
    {
        gp_Ax2 axes;
        return new Geom_Circle(axes, radius);
    }

    Standard_Real radius = 50.;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveSurfaceCircleOfRevol, InteractiveSurfaceRevolution)

InteractiveSurfaceCircleOfRevol::InteractiveSurfaceCircleOfRevol()
    : InteractiveSurfaceRevolution()
    , d(new InteractiveSurfaceCircleOfRevolPrivate)
{
    setUmax(M_PI);
    setVmax(M_PI);
    updateSurface(d->getCurve());
}

InteractiveSurfaceCircleOfRevol::~InteractiveSurfaceCircleOfRevol()
{
    delete d;
}

Standard_Real InteractiveSurfaceCircleOfRevol::getRadius() const
{
    return d->radius;
}

void InteractiveSurfaceCircleOfRevol::setRadius(Standard_Real R)
{
    if (R > 0.) {
        d->radius = R;
        updateSurface(d->getCurve());
        notify();
    }
}

Handle(Geom_Curve) InteractiveSurfaceCircleOfRevol::getCurve() const
{
    return d->getCurve();
}
