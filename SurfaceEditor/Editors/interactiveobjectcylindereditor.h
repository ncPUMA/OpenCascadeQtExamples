#ifndef INTERACTIVEOBJECTCYLINDEREDITOR_H
#define INTERACTIVEOBJECTCYLINDEREDITOR_H

#include "interactiveobjecteditor.h"

class InteractiveCylinder;
class InteractiveObjectCylinderEditorPrivate;

class InteractiveObjectCylinderEditor : public InteractiveObjectEditor
{
    DEFINE_STANDARD_RTTIEXT(InteractiveObjectCylinderEditor, InteractiveObjectEditor)

public:
    InteractiveObjectCylinderEditor(const Handle(InteractiveCylinder) &cylinder);
    ~InteractiveObjectCylinderEditor();

protected:
    void editorChanged(const Handle(AIS_InteractiveObject) &editor) final;
    void updateEditors() final;

private:
    InteractiveObjectCylinderEditorPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveObjectCylinderEditor, InteractiveObjectEditor)

#endif // INTERACTIVEOBJECTCYLINDEREDITOR_H
