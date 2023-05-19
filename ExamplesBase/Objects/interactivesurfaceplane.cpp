#include "interactivesurfaceplane.h"

#include <AIS_InteractiveContext.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <Geom_Plane.hxx>
#include <TopoDS_Face.hxx>

namespace ExamplesBase {

class InteractiveSurfacePlanePrivate
{
    friend class InteractiveSurfacePlane;

    TopoDS_Shape plane() const {
        gp_Ax3 axes;
        Handle(Geom_Plane) plane = new Geom_Plane(axes);
        BRepBuilderAPI_MakeFace faceMaker(plane, 0., Umax, 0., Vmax, Precision::Confusion());
        return faceMaker.Face();
    }

    InteractiveSurfacePlane *q;
    Standard_Real Umax = 100.;
    Standard_Real Vmax = 200.;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveSurfacePlane, InteractiveSurfacee)

InteractiveSurfacePlane::InteractiveSurfacePlane()
    : InteractiveSurface()
    , d(new InteractiveSurfacePlanePrivate)
{
    d->q = this;
    updateShape(d->plane());
}

InteractiveSurfacePlane::~InteractiveSurfacePlane()
{
    delete d;
}

Standard_Real InteractiveSurfacePlane::getUmax() const
{
    return d->Umax;
}

Standard_Real InteractiveSurfacePlane::getVmax() const
{
    return d->Vmax;
}

void InteractiveSurfacePlane::setUmax(Standard_Real U)
{
    if (U > 0.) {
        d->Umax = U;
        updateShape(d->plane());
    }
}

void InteractiveSurfacePlane::setVmax(Standard_Real V)
{
    if (V > 0.) {
        d->Vmax = V;
        updateShape(d->plane());
    }
}

}
