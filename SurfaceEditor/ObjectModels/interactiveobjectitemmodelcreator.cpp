#include "interactiveobjectitemmodelcreator.h"

#include "interactiveobjectitemmodelcuboid.h"
#include "interactiveobjectitemmodelcylinder.h"
#include "interactiveobjectitemmodelshape.h"
#include "interactiveobjectitemmodelsurfacecircleofrevol.h"
#include "interactiveobjectitemmodelsurfaceellipseofrevol.h"
#include "interactiveobjectitemmodelsurfacehyperofrevol.h"
#include "interactiveobjectitemmodelsurfaceparabofrevol.h"
#include "interactiveobjectitemmodelsurfaceplane.h"

#include "../Objects/interactivecuboid.h"
#include "../Objects/interactivecylinder.h"
#include "../Objects/interactiveshape.h"
#include "../Objects/interactivesurfacecircleofrevol.h"
#include "../Objects/interactivesurfaceellipseofrevol.h"
#include "../Objects/interactivesurfacehyperofrevol.h"
#include "../Objects/interactivesurfaceparabofrevol.h"
#include "../Objects/interactivesurfaceplane.h"

InteractiveObjectItemModelCreator::InteractiveObjectItemModelCreator()
{

}

InteractiveObjectItemModel *InteractiveObjectItemModelCreator::createModel(const Handle(InteractiveObject) &interactive)
{
    if (interactive) {
        auto plane = Handle(InteractiveSurfacePlane)::DownCast(interactive);
        if (plane) {
            return new InteractiveObjectItemModelSurfacePlane(plane);
        }

        auto circle = Handle(InteractiveSurfaceCircleOfRevol)::DownCast(interactive);
        if (circle) {
            return new InteractiveObjectItemModelSurfaceCircleOfRevol(circle);
        }

        auto ellipse = Handle(InteractiveSurfaceEllipseOfRevol)::DownCast(interactive);
        if (ellipse) {
            return new InteractiveObjectItemModelSurfaceEllipseOfRevol(ellipse);
        }

        auto hyper = Handle(InteractiveSurfaceHyperOfRevol)::DownCast(interactive);
        if (hyper) {
            return new InteractiveObjectItemModelSurfaceHyperOfRevol(hyper);
        }

        auto parab = Handle(InteractiveSurfaceParabOfRevol)::DownCast(interactive);
        if (parab) {
            return new InteractiveObjectItemModelSurfaceParabOfRevol(parab);
        }

        auto cuboid = Handle(InteractiveCuboid)::DownCast(interactive);
        if (cuboid) {
            return new InteractiveObjectItemModelCuboid(cuboid);
        }

        auto cylinder = Handle(InteractiveCylinder)::DownCast(interactive);
        if (cylinder) {
            return new InteractiveObjectItemModelCylinder(cylinder);
        }

        auto shape = Handle(InteractiveShape)::DownCast(interactive);
        if (shape) {
            return new InteractiveObjectItemModelShape(shape);
        }

        return new InteractiveObjectItemModel(interactive);
    }
    return nullptr;
}
