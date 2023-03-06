#include "interactiveobjectitemmodelcreator.h"

#include "interactiveobjectitemmodelshape.h"
#include "interactiveobjectitemmodelsurfaceparabofrevol.h"
#include "interactiveobjectitemmodelsurfaceplane.h"

#include "../Objects/interactiveshape.h"
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
