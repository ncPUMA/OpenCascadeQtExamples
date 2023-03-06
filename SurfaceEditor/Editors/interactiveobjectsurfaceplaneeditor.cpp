#include "interactiveobjectsurfaceplaneeditor.h"

#include "../Objects/interactivesurfaceplane.h"

#include "dimentionlenghteditor.h"

class InteractiveObjectSurfacePlaneEditorPrivate
{
    friend class InteractiveObjectSurfacePlaneEditor;

    void update() {
        gp_Ax3 axes;
        Geom_Plane plane(axes);
        auto p0 = plane.Value(0., 0.);
        auto pU = plane.Value(surface->getUmax(), 0.);
        auto pV = plane.Value(0., surface->getVmax());
        editorU->update(p0, pU, plane.Pln());
        editorV->update(p0, pV, plane.Pln());
    }

    Handle(InteractiveSurfacePlane) surface;

    Handle(DimentionLenghtEditor) editorU;
    Handle(DimentionLenghtEditor) editorV;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveObjectSurfacePlaneEditor, InteractiveObjectEditor)

InteractiveObjectSurfacePlaneEditor::InteractiveObjectSurfacePlaneEditor(const Handle(InteractiveSurfacePlane) &surface)
    : InteractiveObjectEditor(surface)
    , d(new InteractiveObjectSurfacePlaneEditorPrivate)
{
    d->surface = surface;

    d->editorU = new DimentionLenghtEditor(gp_Pnt(), gp_Pnt(1., 1., 1.), gp_Pln());
    d->editorU->SetFlyout(- d->editorU->GetFlyout());
    AddChild(d->editorU);
    d->editorV = new DimentionLenghtEditor(gp_Pnt(), gp_Pnt(1., 1., 1.), gp_Pln());
    AddChild(d->editorV);
    d->update();
}

InteractiveObjectSurfacePlaneEditor::~InteractiveObjectSurfacePlaneEditor()
{
    delete d;
}

void InteractiveObjectSurfacePlaneEditor::editorChanged(const Handle(AIS_InteractiveObject) &editor)
{
    auto dimentionEditor = Handle(DimentionLenghtEditor)::DownCast(editor);
    if (!dimentionEditor) {
        return;
    }

    Standard_Real value = dimentionEditor->dimention();
    if (dimentionEditor == d->editorU) {
        d->surface->setUmax(value);
    } else if (dimentionEditor == d->editorV) {
        d->surface->setVmax(value);
    }
}

void InteractiveObjectSurfacePlaneEditor::updateEditors()
{
    d->update();
}
