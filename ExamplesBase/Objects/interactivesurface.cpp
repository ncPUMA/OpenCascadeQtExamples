/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactivesurface.h"

#include <AIS_InteractiveContext.hxx>

namespace ExamplesBase {

class InteractiveSurfacePrivate
{
    friend class InteractiveSurface;

    InteractiveSurface::SurfaceTypes surfaceType = InteractiveSurface::SurfaceTypeSurface;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveSurface, InteractiveObject)

InteractiveSurface::InteractiveSurface()
    : InteractiveObject()
    , d(new InteractiveSurfacePrivate)
{

}

InteractiveSurface::~InteractiveSurface()
{
    delete d;
}

InteractiveSurface::SurfaceTypes InteractiveSurface::surfaceType() const
{
    return d->surfaceType;
}

void InteractiveSurface::setSurfaceType(SurfaceTypes surfaceType)
{
    d->surfaceType = surfaceType;
    notify();
}

}
