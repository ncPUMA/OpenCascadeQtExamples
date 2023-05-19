#ifndef INTERACTIVEOBJECTSURFACEHYPEROFREVOLEDITOR_H
#define INTERACTIVEOBJECTSURFACEHYPEROFREVOLEDITOR_H

#include "interactiveobjecteditor.h"

namespace ExamplesBase {

class InteractiveSurfaceHyperOfRevol;
class InteractiveObjectSurfaceHyperOfRevolEditorPrivate;

class InteractiveObjectSurfaceHyperOfRevolEditor : public InteractiveObjectEditor
{
    DEFINE_STANDARD_RTTIEXT(InteractiveObjectSurfaceHyperOfRevolEditor, InteractiveObjectEditor)

public:
    InteractiveObjectSurfaceHyperOfRevolEditor(const Handle(InteractiveSurfaceHyperOfRevol) &surface);
    ~InteractiveObjectSurfaceHyperOfRevolEditor();

    void SetContext(const Handle(AIS_InteractiveContext) &ctx) Standard_OVERRIDE;

protected:
    void editorChanged(const Handle(AIS_InteractiveObject) &editor) final;
    void updateEditors() final;

private:
    InteractiveObjectSurfaceHyperOfRevolEditorPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveObjectSurfaceHyperOfRevolEditor, InteractiveObjectEditor)

}

#endif // INTERACTIVEOBJECTSURFACEHYPEROFREVOLEDITOR_H
