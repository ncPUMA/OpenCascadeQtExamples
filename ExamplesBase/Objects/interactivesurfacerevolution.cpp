#include "interactivesurfacerevolution.h"

#include <AIS_InteractiveContext.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepPrim_Builder.hxx>
#include <Geom_SurfaceOfRevolution.hxx>

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
    updateShape(createShape());
}

void InteractiveSurfaceRevolution::setUmax(Standard_Real U)
{
    if (U > 0.) {
        d->Umax = U;
        updateShape(createShape());
    }
}

void InteractiveSurfaceRevolution::setVmax(Standard_Real V)
{
    if (V > 0.) {
        d->Vmax = V;
        updateShape(createShape());
    }
}

TopoDS_Face InteractiveSurfaceRevolution::revolutionFace(const Handle(Geom_Curve) &curve) const
{
    Handle(Geom_SurfaceOfRevolution) revolution =
            new Geom_SurfaceOfRevolution(curve, gp_Ax1(gp::Origin(), d->revolutionDirection));
    return BRepBuilderAPI_MakeFace(revolution, 0., d->Umax, 0., d->Vmax, Precision::Confusion());
}

TopoDS_Shape InteractiveSurfaceRevolution::buildShape(const TopoDS_Face &revolutionFace, const TopoDS_Face &cap)
{
    BRepPrim_Builder builder;
    TopoDS_Shell shell;
    builder.MakeShell(shell);
    builder.AddShellFace(shell, revolutionFace);
    builder.AddShellFace(shell, cap);
    shell.Closed(BRep_Tool::IsClosed(shell));
    builder.CompleteShell(shell);
    return BRepBuilderAPI_MakeSolid(shell);
}

}
