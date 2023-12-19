/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERACTIVESURFACEREVOLUTION_H
#define INTERACTIVESURFACEREVOLUTION_H

#include "interactivesurface.h"

class Geom_Curve;
class TopoDS_Face;

namespace ExamplesBase {

class InteractiveSurfaceRevolutionPrivate;

class InteractiveSurfaceRevolution : public InteractiveSurface
{
    DEFINE_STANDARD_RTTIEXT(InteractiveSurfaceRevolution, InteractiveSurface)

public:
    InteractiveSurfaceRevolution();
    ~InteractiveSurfaceRevolution();

    gp_Dir getRevolutionDirection() const;
    Standard_Real getUmax() const;
    Standard_Real getVmax() const;

    void setRevolutionDirection(const gp_Dir &direction);
    void setUmax(Standard_Real U);
    void setVmax(Standard_Real V);

protected:
    virtual TopoDS_Shape createShape() const = 0;

    TopoDS_Face revolutionFace(const Handle(Geom_Curve) &curve) const;
    static TopoDS_Shape buildShape(const TopoDS_Face &revolutionFace, const TopoDS_Face &cap);

private:
    InteractiveSurfaceRevolutionPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveSurfaceRevolution, InteractiveSurface)

}

#endif // INTERACTIVESURFACEREVOLUTION_H
