#ifndef INTERACTIVEOBJECTSURFACEPLANEEDITOR_H
#define INTERACTIVEOBJECTSURFACEPLANEEDITOR_H

#include "interactiveobjecteditor.h"

class InteractiveSurfacePlane;
class InteractiveObjectSurfacePlaneEditorPrivate;

class InteractiveObjectSurfacePlaneEditor : public InteractiveObjectEditor
{
    DEFINE_STANDARD_RTTIEXT(InteractiveObjectSurfacePlaneEditor, InteractiveObjectEditor)

public:
    InteractiveObjectSurfacePlaneEditor(const Handle(InteractiveSurfacePlane) &surface);
    ~InteractiveObjectSurfacePlaneEditor();

protected:
    void editorChanged(const Handle(AIS_InteractiveObject) &editor) final;
    void updateEditors() final;

private:
    InteractiveObjectSurfacePlaneEditorPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveObjectSurfacePlaneEditor, InteractiveObjectEditor)

#endif // INTERACTIVEOBJECTSURFACEPLANEEDITOR_H
