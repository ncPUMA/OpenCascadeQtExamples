#include "interactivesurfacecircleofrevol.h"

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <Geom_Circle.hxx>
#include <gp_Circ.hxx>
#include <TopoDS_Face.hxx>

namespace ExamplesBase {

class InteractiveSurfaceCircleOfRevolPrivate
{
    friend class InteractiveSurfaceCircleOfRevol;

    TopoDS_Shape createShape() const {
        auto circle = gp_Circ(gp_Ax2(), radius);
        auto revFace = q->revolutionFace(new Geom_Circle(circle));
        revFace.Reverse();
        auto wire = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(circle));
        auto capFace = BRepBuilderAPI_MakeFace(wire.Wire());
        return q->buildShape(revFace, capFace);
    }

    InteractiveSurfaceCircleOfRevol *q;
    Standard_Real radius = 50.;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveSurfaceCircleOfRevol, InteractiveSurfaceRevolution)

InteractiveSurfaceCircleOfRevol::InteractiveSurfaceCircleOfRevol()
    : InteractiveSurfaceRevolution()
    , d(new InteractiveSurfaceCircleOfRevolPrivate)
{
    d->q = this;
    setUmax(M_PI);
    setVmax(M_PI);
    updateShape(d->createShape());
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
        updateShape(d->createShape());
        notify();
    }
}

TopoDS_Shape InteractiveSurfaceCircleOfRevol::createShape() const
{
    return d->createShape();
}

}
