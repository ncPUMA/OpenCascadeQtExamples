#ifndef INTERACTIVEOBJECTEDITOR_H
#define INTERACTIVEOBJECTEDITOR_H

#include <AIS_InteractiveObject.hxx>

class DimentionLenghtEditor;
class InteractiveObject;
class InteractiveObjectEditorPrivate;

class InteractiveObjectEditor : public AIS_InteractiveObject
{
    DEFINE_STANDARD_RTTIEXT(InteractiveObjectEditor, AIS_InteractiveObject)

public:
    InteractiveObjectEditor(const Handle(InteractiveObject) &object);
    ~InteractiveObjectEditor();

    void ComputeSelection(const Handle(SelectMgr_Selection) &,
                          const Standard_Integer) final { };

    void handleEditor(const Handle(AIS_InteractiveObject) &editor);
    void update();

protected:
    void Compute(const Handle(PrsMgr_PresentationManager) &,
                 const Handle(Prs3d_Presentation) &,
                 const Standard_Integer) final { };

    virtual void editorChanged(const Handle(AIS_InteractiveObject) &editor) = 0;
    virtual void updateEditors() = 0;

private:
    InteractiveObjectEditorPrivate *d;
};

DEFINE_STANDARD_HANDLE(InteractiveObjectEditor, AIS_InteractiveObject)

#endif // INTERACTIVEOBJECTEDITOR_H
