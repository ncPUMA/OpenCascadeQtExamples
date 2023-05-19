#ifndef INTERACTIVEOBJECTSURFACECIRCLEOFREVOLEDITOR_H
#define INTERACTIVEOBJECTSURFACECIRCLEOFREVOLEDITOR_H

#include "interactiveobjecteditor.h"

namespace ExamplesBase {

class InteractiveSurfaceCircleOfRevol;
class InteractiveObjectSurfaceCircleOfRevolEditorPrivate;

class InteractiveObjectSurfaceCircleOfRevolEditor : public InteractiveObjectEditor
{
    DEFINE_STANDARD_RTTIEXT(InteractiveObjectSurfaceCircleOfRevolEditor, InteractiveObjectEditor)

public:
    InteractiveObjectSurfaceCircleOfRevolEditor(const Handle(InteractiveSurfaceCircleOfRevol) &surface);
    ~InteractiveObjectSurfaceCircleOfRevolEditor();

    void SetContext(const Handle(AIS_InteractiveContext) &ctx) Standard_OVERRIDE;

protected:
    void editorChanged(const Handle(AIS_InteractiveObject) &editor) final;
    void updateEditors() final;

private:
    InteractiveObjectSurfaceCircleOfRevolEditorPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveObjectSurfaceCircleOfRevolEditor, InteractiveObjectEditor)

}

#endif // INTERACTIVEOBJECTSURFACECIRCLEOFREVOLEDITOR_H
