#include "interactiveobjectitemmodelcreator.h"

#include "interactiveobjectitemmodelsurfaceplane.h"

#include "interactivesurfaceplane.h"

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

        return new InteractiveObjectItemModel(interactive);
    }
    return nullptr;
}
