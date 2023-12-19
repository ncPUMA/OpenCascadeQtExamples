/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactiveobjecteditorcreator.h"

#include "../Objects/interactivecuboid.h"
#include "../Objects/interactivecylinder.h"
#include "../Objects/interactivesphere.h"
#include "../Objects/interactivesurfacecircleofrevol.h"
#include "../Objects/interactivesurfaceellipseofrevol.h"
#include "../Objects/interactivesurfacehyperofrevol.h"
#include "../Objects/interactivesurfaceparabofrevol.h"
#include "../Objects/interactivesurfaceplane.h"

#include "interactiveobjectcuboideditor.h"
#include "interactiveobjectcylindereditor.h"
#include "interactiveobjectsphereeditor.h"
#include "interactiveobjectsurfacecircleofrevoleditor.h"
#include "interactiveobjectsurfaceellipseofrevoleditor.h"
#include "interactiveobjectsurfacehyperofrevoleditor.h"
#include "interactiveobjectsurfaceparabofrevoleditor.h"
#include "interactiveobjectsurfaceplaneeditor.h"

using namespace ExamplesBase;

InteractiveObjectEditorCreator::InteractiveObjectEditorCreator()
{

}

Handle(InteractiveObjectEditor) InteractiveObjectEditorCreator::create(const Handle(InteractiveObject) &object)
{
    auto ellipse = Handle(InteractiveSurfaceEllipseOfRevol)::DownCast(object);
    if (ellipse) {
        return new InteractiveObjectSurfaceEllipseOfRevolEditor(ellipse);
    }
    auto hyper = Handle(InteractiveSurfaceHyperOfRevol)::DownCast(object);
    if (hyper) {
        return new InteractiveObjectSurfaceHyperOfRevolEditor(hyper);
    }
    auto circle = Handle(InteractiveSurfaceCircleOfRevol)::DownCast(object);
    if (circle) {
        return new InteractiveObjectSurfaceCircleOfRevolEditor(circle);
    }
    auto parab = Handle(InteractiveSurfaceParabOfRevol)::DownCast(object);
    if (parab) {
        return new InteractiveObjectSurfaceParabOfRevolEditor(parab);
    }
    auto plane = Handle(InteractiveSurfacePlane)::DownCast(object);
    if (plane) {
        return new InteractiveObjectSurfacePlaneEditor(plane);
    }
    auto cuboid = Handle(InteractiveCuboid)::DownCast(object);
    if (cuboid) {
        return new InteractiveObjectCuboidEditor(cuboid);
    }
    auto cylinder = Handle(InteractiveCylinder)::DownCast(object);
    if (cylinder) {
        return new InteractiveObjectCylinderEditor(cylinder);
    }
    auto sphere = Handle(InteractiveSphere)::DownCast(object);
    if (sphere) {
        return new InteractiveObjectSphereEditor(sphere);
    }
    return nullptr;
}
