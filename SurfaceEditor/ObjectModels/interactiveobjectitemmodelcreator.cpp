#include "interactiveobjectitemmodelcreator.h"

#include "interactiveobjectitemmodelshape.h"
#include "interactiveobjectitemmodelsurfacehyperofrevol.h"
#include "interactiveobjectitemmodelsurfaceparabofrevol.h"
#include "interactiveobjectitemmodelsurfaceplane.h"

#include "../Objects/interactiveshape.h"
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

        auto hyper = Handle(InteractiveSurfaceHyperOfRevol)::DownCast(interactive);
        if (hyper) {
            return new InteractiveObjectItemModelSurfaceHyperOfRevol(hyper);
        }

        auto parab = Handle(InteractiveSurfaceParabOfRevol)::DownCast(interactive);
        if (parab) {
            return new InteractiveObjectItemModelSurfaceParabOfRevol(parab);
        }

        auto shape = Handle(InteractiveShape)::DownCast(interactive);
        if (shape) {
            return new InteractiveObjectItemModelShape(shape);
        }

        return new InteractiveObjectItemModel(interactive);
    }
    return nullptr;
}
