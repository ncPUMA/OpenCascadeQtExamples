#include "interactivesurfaceellipseofrevol.h"

#include <Geom_Ellipse.hxx>

namespace ExamplesBase {

class InteractiveSurfaceEllipseOfRevolPrivate
{
    friend class InteractiveSurfaceEllipseOfRevol;

    Standard_Real majorR() const {
        return focal / eccentricity;
    }

    Standard_Real minorR() const {
        auto majR = majorR();
        return sqrt(focal * focal - majR * majR);
    }

    Handle(Geom_Curve) getCurve() const
    {
        gp_Ax2 axes;
        return new Geom_Ellipse(axes, majorR(), minorR());
    }

    Standard_Real focal = 100.;
    Standard_Real eccentricity = 1.2;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveSurfaceEllipseOfRevol, InteractiveSurfaceRevolution)

InteractiveSurfaceEllipseOfRevol::InteractiveSurfaceEllipseOfRevol()
    : InteractiveSurfaceRevolution()
    , d(new InteractiveSurfaceEllipseOfRevolPrivate)
{
    setUmax(M_PI);
    setVmax(M_PI);
    setRevolutionDirection(gp_Dir(1., 0., 0.));
    updateSurface(d->getCurve());
}

InteractiveSurfaceEllipseOfRevol::~InteractiveSurfaceEllipseOfRevol()
{
    delete d;
}

Standard_Real InteractiveSurfaceEllipseOfRevol::getFocal() const
{
    return d->focal;
}

Standard_Real InteractiveSurfaceEllipseOfRevol::getEccentricity() const
{
    return d->eccentricity;
}

void InteractiveSurfaceEllipseOfRevol::setFocal(Standard_Real F)
{
    if (F > 0.) {
        d->focal = F;
        updateSurface(d->getCurve());
    }
}

void InteractiveSurfaceEllipseOfRevol::setEccentricity(Standard_Real eps)
{
    if (eps > 0.) {
        d->eccentricity = eps;
        updateSurface(d->getCurve());
    }
}

Standard_Real InteractiveSurfaceEllipseOfRevol::majorR() const
{
    return d->majorR();
}

Standard_Real InteractiveSurfaceEllipseOfRevol::minorR() const
{
    return d->minorR();
}

Handle(Geom_Curve) InteractiveSurfaceEllipseOfRevol::getCurve() const
{
    return d->getCurve();
}

}
