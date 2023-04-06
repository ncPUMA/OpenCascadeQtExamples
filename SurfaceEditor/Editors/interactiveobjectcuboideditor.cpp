#include "interactiveobjectcuboideditor.h"

#include "../Objects/interactivecuboid.h"

#include "dimentionlenghteditor.h"

class InteractiveObjectCuboidEditorPrivate
{
    friend class InteractiveObjectCuboidEditor;

    void update() {
        gp_Pnt p0;
        gp_Pnt pW(cuboid->getWidth(), 0., 0.);
        gp_Pnt pH(0., cuboid->getHeight(), 0.);
        gp_Pnt pD(0., 0., cuboid->getDepth());
        editorWidth->update(p0, pW, gp_Pln(p0, gp_Dir(0., 0., 1.)));
        editorHeight->update(p0, pH, gp_Pln(p0, gp_Dir(0., 0., 1.)));
        editorDepth->update(p0, pD, gp_Pln(p0, gp_Dir(1., 0., 0.)));
    }

    Handle(InteractiveCuboid) cuboid;

    Handle(DimentionLenghtEditor) editorWidth;
    Handle(DimentionLenghtEditor) editorHeight;
    Handle(DimentionLenghtEditor) editorDepth;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveObjectCuboidEditor, InteractiveObjectEditor)

InteractiveObjectCuboidEditor::InteractiveObjectCuboidEditor(const Handle(InteractiveCuboid) &cuboid)
    : InteractiveObjectEditor(cuboid)
    , d(new InteractiveObjectCuboidEditorPrivate)
{
    d->cuboid = cuboid;

    d->editorWidth = new DimentionLenghtEditor(gp_Pnt(), gp_Pnt(1., 1., 1.), gp_Pln());
    d->editorWidth->SetFlyout(- d->editorWidth->GetFlyout());
    AddChild(d->editorWidth);
    d->editorHeight = new DimentionLenghtEditor(gp_Pnt(), gp_Pnt(1., 1., 1.), gp_Pln());
    AddChild(d->editorHeight);
    d->editorDepth = new DimentionLenghtEditor(gp_Pnt(), gp_Pnt(1., 1., 1.), gp_Pln());
    AddChild(d->editorDepth);
    d->update();
}

InteractiveObjectCuboidEditor::~InteractiveObjectCuboidEditor()
{
    delete d;
}

void InteractiveObjectCuboidEditor::editorChanged(const Handle(AIS_InteractiveObject) &editor)
{
    auto dimentionEditor = Handle(DimentionLenghtEditor)::DownCast(editor);
    if (!dimentionEditor) {
        return;
    }

    Standard_Real value = dimentionEditor->dimention();
    if (dimentionEditor == d->editorWidth) {
        d->cuboid->setWidth(value);
    } else if (dimentionEditor == d->editorHeight) {
        d->cuboid->setHeight(value);
    } else if (dimentionEditor == d->editorDepth) {
        d->cuboid->setDepth(value);
    }
}

void InteractiveObjectCuboidEditor::updateEditors()
{
    d->update();
}
