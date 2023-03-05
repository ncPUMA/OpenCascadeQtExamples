#include "interactiveobjecteditorcreator.h"

#include "../Objects/interactivesurfaceplane.h"

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
    return nullptr;
}
