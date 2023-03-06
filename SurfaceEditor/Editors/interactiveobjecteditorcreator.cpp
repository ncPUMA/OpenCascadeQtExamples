#include "interactiveobjecteditorcreator.h"

#include "../Objects/interactivesurfaceparabofrevol.h"
#include "../Objects/interactivesurfaceplane.h"

#include "interactiveobjectsurfaceparabofrevoleditor.h"
#include "interactiveobjectsurfaceplaneeditor.h"

InteractiveObjectEditorCreator::InteractiveObjectEditorCreator()
{

}

Handle(InteractiveObjectEditor) InteractiveObjectEditorCreator::create(const Handle(InteractiveObject) &object)
{
    auto plane = Handle(InteractiveSurfacePlane)::DownCast(object);
    if (plane) {
        return new InteractiveObjectSurfacePlaneEditor(plane);
    }
    auto parab = Handle(InteractiveSurfaceParabOfRevol)::DownCast(object);
    if (parab) {
        return new InteractiveObjectSurfaceParabOfRevolEditor(parab);
    }
    return nullptr;
}
