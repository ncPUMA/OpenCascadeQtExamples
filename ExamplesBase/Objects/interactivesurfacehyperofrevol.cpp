#include "interactivesurfacehyperofrevol.h"

#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <Geom_Hyperbola.hxx>
#include <TopoDS_Face.hxx>

namespace ExamplesBase {

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

    TopoDS_Shape createShape() const {
        Handle(Geom_Hyperbola) hyp = new Geom_Hyperbola(gp_Ax2(), majorR(), minorR());
        auto revFace = q->revolutionFace(hyp);
        revFace.Reverse();
        BRepAdaptor_Surface surface(revFace);
        auto p0 = surface.Value(0., q->getVmax());
        auto p1 = surface.Value(M_PI, q->getVmax());
        auto edge = BRepBuilderAPI_MakeEdge(p0, p1);
        BRepAdaptor_Curve curve(edge);
        Handle(Geom_SurfaceOfRevolution) revolution =
                new Geom_SurfaceOfRevolution(curve.Curve().Curve(), gp_Ax1(gp::Origin(), q->getRevolutionDirection()));
        auto cap = BRepBuilderAPI_MakeFace(revolution, 0., M_PI, 0., curve.LastParameter(), Precision::Confusion());
        return q->buildShape(revFace, cap);
    }

    InteractiveSurfaceHyperOfRevol *q;
    Standard_Real focal = 2.;
    Standard_Real eccentricity = 1.2;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveSurfaceHyperOfRevol, InteractiveSurfaceRevolution)

InteractiveSurfaceHyperOfRevol::InteractiveSurfaceHyperOfRevol()
    : InteractiveSurfaceRevolution()
    , d(new InteractiveSurfaceHyperOfRevolPrivate)
{
    d->q = this;
    setVmax(M_PI * 2);
    setRevolutionDirection(gp_Dir(1., 0., 0.));
    updateShape(d->createShape());
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
        updateShape(d->createShape());
    }
}

void InteractiveSurfaceHyperOfRevol::setEccentricity(Standard_Real eps)
{
    if (eps > 0.) {
        d->eccentricity = eps;
        updateShape(d->createShape());
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

TopoDS_Shape InteractiveSurfaceHyperOfRevol::createShape() const
{
    return d->createShape();
}

}
