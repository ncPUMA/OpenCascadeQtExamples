/*
    SPDX-FileCopyrightText: 2024 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERACTIVETEXTUREDPLANE_H
#define INTERACTIVETEXTUREDPLANE_H

#include "interactivesurfaceplane.h"

namespace ExamplesBase {

class InteractiveTexturedPlanePrivate;

class InteractiveTexturedPlane : public InteractiveSurfacePlane
{
    DEFINE_STANDARD_RTTIEXT(InteractiveTexturedPlane, InteractiveSurfacePlane)
public:
    InteractiveTexturedPlane();
    ~InteractiveTexturedPlane();

    TCollection_AsciiString textureFileName() const;
    void setTextureFileName(const TCollection_AsciiString &fname);

private:
    InteractiveTexturedPlanePrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveTexturedPlane, InteractiveSurfacePlane)

}

#endif // INTERACTIVETEXTUREDPLANE_H
