#include "interactivesurfacerevolution.h"

#include <AIS_InteractiveContext.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <TopoDS_Face.hxx>

namespace ExamplesBase {

class InteractiveSurfaceRevolutionPrivate
{
    friend class InteractiveSurfaceRevolution;

    InteractiveSurfaceRevolution *q;
    gp_Dir revolutionDirection = gp::DX();
    Standard_Real Umax = M_PI * 2;
    Standard_Real Vmax = 200.;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveSurfaceRevolution, InteractiveSurface)

InteractiveSurfaceRevolution::InteractiveSurfaceRevolution()
    : InteractiveSurface()
    , d(new InteractiveSurfaceRevolutionPrivate)
{
    d->q = this;
}

InteractiveSurfaceRevolution::~InteractiveSurfaceRevolution()
{
    delete d;
}

gp_Dir InteractiveSurfaceRevolution::getRevolutionDirection() const
{
    return d->revolutionDirection;
}

Standard_Real InteractiveSurfaceRevolution::getUmax() const
{
    return d->Umax;
}

Standard_Real InteractiveSurfaceRevolution::getVmax() const
{
    return d->Vmax;
}

void InteractiveSurfaceRevolution::setRevolutionDirection(const gp_Dir &direction)
{
    d->revolutionDirection = direction;
    updateSurface(getCurve());
}

void InteractiveSurfaceRevolution::setUmax(Standard_Real U)
{
    if (U > 0.) {
        d->Umax = U;
        updateSurface(getCurve());
    }
}

void InteractiveSurfaceRevolution::setVmax(Standard_Real V)
{
    if (V > 0.) {
        d->Vmax = V;
        updateSurface(getCurve());
    }
}

void InteractiveSurfaceRevolution::updateSurface(const Handle(Geom_Curve) &curve)
{
    Handle(Geom_SurfaceOfRevolution) revolution =
            new Geom_SurfaceOfRevolution(curve, gp_Ax1(gp::Origin(), d->revolutionDirection));
    BRepBuilderAPI_MakeFace faceMaker(revolution, 0., d->Umax, 0., d->Vmax, Precision::Confusion());
    updateShape(faceMaker.Face());
}

}
