#include "interactivesurfaceparabofrevol.h"

#include <AIS_InteractiveContext.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <Geom_Parabola.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <TopoDS_Face.hxx>

class InteractiveSurfaceParabOfRevolPrivate
{
    friend class InteractiveSurfaceParabOfRevol;

    void updateSurface() {
        gp_Ax2 axes;
        Handle(Geom_Parabola) parabola = new Geom_Parabola(axes, focal);
        Handle(Geom_SurfaceOfRevolution) revolution =
                new Geom_SurfaceOfRevolution(parabola, gp_Ax1(gp::Origin(), revolutionDirection));
        BRepBuilderAPI_MakeFace faceMaker(revolution, 0., Umax, 0., Vmax, Precision::Confusion());
        q->SetShape(faceMaker.Face());
        auto ctx = q->GetContext();
        if (ctx) {
            ctx->Redisplay(q, Standard_True, Standard_True);
        }
    }

    InteractiveSurfaceParabOfRevol *q;
    Standard_Real focal = 50.;
    gp_Dir revolutionDirection = gp::DX();
    Standard_Real Umax = M_PI * 2.;
    Standard_Real Vmax = 200.;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveSurfaceParabOfRevol, InteractiveSurface)

InteractiveSurfaceParabOfRevol::InteractiveSurfaceParabOfRevol()
    : InteractiveSurface()
    , d(new InteractiveSurfaceParabOfRevolPrivate)
{
    d->q = this;
    d->updateSurface();
}

InteractiveSurfaceParabOfRevol::~InteractiveSurfaceParabOfRevol()
{
    delete d;
}

Standard_Real InteractiveSurfaceParabOfRevol::getFocal() const
{
    return d->focal;
}

gp_Dir InteractiveSurfaceParabOfRevol::getRevolutionDirection() const
{
    return d->revolutionDirection;
}

Standard_Real InteractiveSurfaceParabOfRevol::getUmax() const
{
    return d->Umax;
}

Standard_Real InteractiveSurfaceParabOfRevol::getVmax() const
{
    return d->Vmax;
}

void InteractiveSurfaceParabOfRevol::setFocal(Standard_Real F)
{
    if (F >= 0.) {
        d->focal = F;
        d->updateSurface();
        notify();
    }
}

void InteractiveSurfaceParabOfRevol::setRevolutionDirection(const gp_Dir &direction)
{
    d->revolutionDirection = direction;
    d->updateSurface();
    notify();
}

void InteractiveSurfaceParabOfRevol::setUmax(Standard_Real U)
{
    if (U > 0.) {
        d->Umax = U;
        d->updateSurface();
        notify();
    }
}

void InteractiveSurfaceParabOfRevol::setVmax(Standard_Real V)
{
    if (V > 0.) {
        d->Vmax = V;
        d->updateSurface();
        notify();
    }
}
