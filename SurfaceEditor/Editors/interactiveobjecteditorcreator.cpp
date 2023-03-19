#include "interactiveobjecteditorcreator.h"

#include "../Objects/interactivesurfacecircleofrevol.h"
#include "../Objects/interactivesurfacehyperofrevol.h"
#include "../Objects/interactivesurfaceparabofrevol.h"
#include "../Objects/interactivesurfaceplane.h"

#include "interactiveobjectsurfacecircleofrevoleditor.h"
#include "interactiveobjectsurfacehyperofrevoleditor.h"
#include "interactiveobjectsurfaceparabofrevoleditor.h"
#include "interactiveobjectsurfaceplaneeditor.h"

InteractiveObjectEditorCreator::InteractiveObjectEditorCreator()
{

}

Handle(InteractiveObjectEditor) InteractiveObjectEditorCreator::create(const Handle(InteractiveObject) &object)
{
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
    return nullptr;
}
