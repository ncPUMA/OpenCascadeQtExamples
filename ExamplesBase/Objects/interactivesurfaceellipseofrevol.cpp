/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactivesurfaceellipseofrevol.h"

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <Geom_Ellipse.hxx>
#include <gp_Elips.hxx>
#include <TopoDS_Face.hxx>

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

    TopoDS_Shape createShape() const {
        auto elipse = gp_Elips(gp_Ax2(), majorR(), minorR());
        auto revFace = q->revolutionFace(new Geom_Ellipse(elipse));
        revFace.Reverse();
        auto wire = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(elipse));
        TopoDS_Face capFace = BRepBuilderAPI_MakeFace(wire.Wire());
        capFace.Reverse();
        return q->buildShape(revFace, capFace);
    }

    InteractiveSurfaceEllipseOfRevol *q;
    Standard_Real focal = 100.;
    Standard_Real eccentricity = 1.2;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveSurfaceEllipseOfRevol, InteractiveSurfaceRevolution)

InteractiveSurfaceEllipseOfRevol::InteractiveSurfaceEllipseOfRevol()
    : InteractiveSurfaceRevolution()
    , d(new InteractiveSurfaceEllipseOfRevolPrivate)
{
    d->q = this;
    setUmax(M_PI);
    setVmax(M_PI);
    setRevolutionDirection(gp_Dir(1., 0., 0.));
    updateShape(d->createShape());
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
        updateShape(d->createShape());
    }
}

void InteractiveSurfaceEllipseOfRevol::setEccentricity(Standard_Real eps)
{
    if (eps > 0.) {
        d->eccentricity = eps;
        updateShape(d->createShape());
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

TopoDS_Shape InteractiveSurfaceEllipseOfRevol::createShape() const
{
    return d->createShape();
}

}
