#ifndef INTERACTIVEOBJECTSURFACEPARABOFREVOLEDITOR_H
#define INTERACTIVEOBJECTSURFACEPARABOFREVOLEDITOR_H

#include "interactiveobjecteditor.h"

class InteractiveSurfaceParabOfRevol;
class InteractiveObjectSurfaceParabOfRevolEditorPrivate;

class InteractiveObjectSurfaceParabOfRevolEditor : public InteractiveObjectEditor
{
    DEFINE_STANDARD_RTTIEXT(InteractiveObjectSurfaceParabOfRevolEditor, InteractiveObjectEditor)

public:
    InteractiveObjectSurfaceParabOfRevolEditor(const Handle(InteractiveSurfaceParabOfRevol) &surface);
    ~InteractiveObjectSurfaceParabOfRevolEditor();

    void SetContext(const Handle(AIS_InteractiveContext) &ctx) Standard_OVERRIDE;

protected:
    void editorChanged(const Handle(AIS_InteractiveObject) &editor) final;
    void updateEditors() final;

private:
    InteractiveObjectSurfaceParabOfRevolEditorPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveObjectSurfaceParabOfRevolEditor, InteractiveObjectEditor)

#endif // INTERACTIVEOBJECTSURFACEPARABOFREVOLEDITOR_H
