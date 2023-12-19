/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactivesurfaceparabofrevol.h"

#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <Geom_Parabola.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <TopoDS_Face.hxx>

namespace ExamplesBase {

class InteractiveSurfaceParabOfRevolPrivate
{
    friend class InteractiveSurfaceParabOfRevol;

    TopoDS_Shape createShape() const {
        Handle(Geom_Parabola) parabola = new Geom_Parabola(gp_Ax2(), focal);
        auto revFace = q->revolutionFace(parabola);
        BRepAdaptor_Surface surface(revFace);
        auto p0 = surface.Value(0., q->getVmax());
        auto p1 = surface.Value(M_PI, q->getVmax());
        auto edge = BRepBuilderAPI_MakeEdge(p0, p1);
        BRepAdaptor_Curve curve(edge);
        Handle(Geom_SurfaceOfRevolution) revolution =
                new Geom_SurfaceOfRevolution(curve.Curve().Curve(), gp_Ax1(gp::Origin(), q->getRevolutionDirection()));
        TopoDS_Face cap = BRepBuilderAPI_MakeFace(revolution, 0., M_PI, 0., curve.LastParameter(), Precision::Confusion());
        return q->buildShape(revFace, cap);
    }

    InteractiveSurfaceParabOfRevol *q;
    Standard_Real focal = 50.;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveSurfaceParabOfRevol, InteractiveSurfaceRevolution)

InteractiveSurfaceParabOfRevol::InteractiveSurfaceParabOfRevol()
    : InteractiveSurfaceRevolution()
    , d(new InteractiveSurfaceParabOfRevolPrivate)
{
    d->q = this;
    updateShape(d->createShape());
}

InteractiveSurfaceParabOfRevol::~InteractiveSurfaceParabOfRevol()
{
    delete d;
}

Standard_Real InteractiveSurfaceParabOfRevol::getFocal() const
{
    return d->focal;
}

void InteractiveSurfaceParabOfRevol::setFocal(Standard_Real F)
{
    if (F >= 0.) {
        d->focal = F;
        updateShape(d->createShape());
    }
}

TopoDS_Shape InteractiveSurfaceParabOfRevol::createShape() const
{
    return d->createShape();
}

}
