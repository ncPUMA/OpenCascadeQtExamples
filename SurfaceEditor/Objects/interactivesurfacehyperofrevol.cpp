#include "interactivesurfacehyperofrevol.h"

#include <Geom_Hyperbola.hxx>

class InteractiveSurfaceHyperOfRevolPrivate
{
    friend class InteractiveSurfaceHyperOfRevol;

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
        return new Geom_Hyperbola(axes, majorR(), minorR());
    }

    Standard_Real focal = 2.;
    Standard_Real eccentricity = 1.2;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveSurfaceHyperOfRevol, InteractiveSurfaceRevolution)

InteractiveSurfaceHyperOfRevol::InteractiveSurfaceHyperOfRevol()
    : InteractiveSurfaceRevolution()
    , d(new InteractiveSurfaceHyperOfRevolPrivate)
{
    setVmax(M_PI * 2);
    setRevolutionDirection(gp_Dir(1., 0., 0.));
    updateSurface(d->getCurve());
}

InteractiveSurfaceHyperOfRevol::~InteractiveSurfaceHyperOfRevol()
{
    delete d;
}

Standard_Real InteractiveSurfaceHyperOfRevol::getFocal() const
{
    return d->focal;
}

Standard_Real InteractiveSurfaceHyperOfRevol::getEccentricity() const
{
    return d->eccentricity;
}

void InteractiveSurfaceHyperOfRevol::setFocal(Standard_Real F)
{
    if (F > 0.) {
        d->focal = F;
        updateSurface(d->getCurve());
    }
}

void InteractiveSurfaceHyperOfRevol::setEccentricity(Standard_Real eps)
{
    if (eps > 0.) {
        d->eccentricity = eps;
        updateSurface(d->getCurve());
    }
}

Standard_Real InteractiveSurfaceHyperOfRevol::majorR() const
{
    return d->majorR();
}

Standard_Real InteractiveSurfaceHyperOfRevol::minorR() const
{
    return d->minorR();
}

Handle(Geom_Curve) InteractiveSurfaceHyperOfRevol::getCurve() const
{
    return d->getCurve();
}
