/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERACTIVESURFACECIRCLEOFREVOL_H
#define INTERACTIVESURFACECIRCLEOFREVOL_H

#include "interactivesurfacerevolution.h"

namespace ExamplesBase {

class InteractiveSurfaceCircleOfRevolPrivate;

class InteractiveSurfaceCircleOfRevol : public InteractiveSurfaceRevolution
{
    DEFINE_STANDARD_RTTIEXT(InteractiveSurfaceCircleOfRevol, InteractiveSurfaceRevolution)

public:
    InteractiveSurfaceCircleOfRevol();
    ~InteractiveSurfaceCircleOfRevol();

    Standard_Real getRadius() const;
    void setRadius(Standard_Real R);

protected:
    TopoDS_Shape createShape() const Standard_OVERRIDE;

private:
    friend class InteractiveSurfaceCircleOfRevolPrivate;
    InteractiveSurfaceCircleOfRevolPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveSurfaceCircleOfRevol, InteractiveSurfaceRevolution)

}

#endif // INTERACTIVESURFACECIRCLEOFREVOL_H
