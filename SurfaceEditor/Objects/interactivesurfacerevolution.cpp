#include "interactivesurfacerevolution.h"

#include <AIS_InteractiveContext.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <TopoDS_Face.hxx>

class InteractiveSurfaceRevolutionPrivate
{
    friend class InteractiveSurfaceRevolution;

    void updateSurface(const opencascade::handle<Geom_Curve> &curve) {
        Handle(Geom_SurfaceOfRevolution) revolution =
                new Geom_SurfaceOfRevolution(curve, gp_Ax1(gp::Origin(), revolutionDirection));
        BRepBuilderAPI_MakeFace faceMaker(revolution, 0., Umax, 0., Vmax, Precision::Confusion());
        q->SetShape(faceMaker.Face());
        auto ctx = q->GetContext();
        if (ctx) {
            ctx->Redisplay(q, Standard_True, Standard_True);
        }
    }

    InteractiveSurfaceRevolution *q;
    gp_Dir revolutionDirection = gp::DX();
    Standard_Real Umax = M_PI * 2.;
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
    d->updateSurface(getCurve());
    notify();
}

void InteractiveSurfaceRevolution::setUmax(Standard_Real U)
{
    if (U > 0.) {
        d->Umax = U;
        d->updateSurface(getCurve());
        notify();
    }
}

void InteractiveSurfaceRevolution::setVmax(Standard_Real V)
{
    if (V > 0.) {
        d->Vmax = V;
        d->updateSurface(getCurve());
        notify();
    }
}

void InteractiveSurfaceRevolution::updateSurface(const Handle(Geom_Curve) &curve)
{
    d->updateSurface(curve);
}
