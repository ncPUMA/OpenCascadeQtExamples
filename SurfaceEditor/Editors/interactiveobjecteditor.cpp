#include "interactiveobjecteditor.h"

#include <AIS_InteractiveContext.hxx>

#include "../Objects/interactiveobject.h"

class InteractiveObjectEditorPrivate
{
    friend class InteractiveObjectEditor;

    Handle(InteractiveObject) object;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveObjectEditor, AIS_InteractiveObject)

InteractiveObjectEditor::InteractiveObjectEditor(const Handle(InteractiveObject) &object)
    : AIS_InteractiveObject()
    , d(new InteractiveObjectEditorPrivate)
{
    d->object = object;

    SetMutable(Standard_True);
    SetZLayer(Graphic3d_ZLayerId_Topmost);
}

InteractiveObjectEditor::~InteractiveObjectEditor()
{
    delete d;
}

void InteractiveObjectEditor::handleEditor(const Handle(AIS_InteractiveObject) &editor)
{
    editorChanged(editor);

    auto ctx = GetContext();
    if (ctx) {
        Handle(InteractiveObject) interactive;
        ctx->InitSelected();
        if (ctx->MoreSelected()) {
            interactive = Handle(InteractiveObject)::DownCast(ctx->SelectedInteractive());
        }
        if (!interactive) {
            ctx->SetSelected(d->object, Standard_True);
        } else if (interactive && interactive != d->object) {
//            ctx->ClearSelected(Standard_True);
//            ctx->SetSelected(d->object, Standard_True);
        }
    }

    update();
}

void InteractiveObjectEditor::update()
{
    updateEditors();
    auto ctx = GetContext();
    if (ctx) {
        for (const auto &obj : Children()) {
            auto interactive = Handle(AIS_InteractiveObject)::DownCast(obj);
            if (obj) {
                ctx->Redisplay(interactive, Standard_True);
            }
        }
    }
}
