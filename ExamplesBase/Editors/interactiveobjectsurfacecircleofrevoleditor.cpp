#include "interactiveobjectsurfacecircleofrevoleditor.h"

#include <AIS_InteractiveContext.hxx>
#include <AIS_TextLabel.hxx>
#include <Geom_CartesianPoint.hxx>
#include <Geom_Circle.hxx>
#include <Geom_SurfaceOfRevolution.hxx>

#include "../Objects/interactivesurfacecircleofrevol.h"

#include "dimentionlenghteditor.h"
#include "point.h"

namespace ExamplesBase {

class InteractiveObjectSurfaceCircleOfRevolEditorPrivate
{
    friend class InteractiveObjectSurfaceCircleOfRevolEditor;

    void update() {
        gp_Ax2 axes;
        Handle(Geom_Circle) circle = new Geom_Circle(axes, surface->getRadius());
        Handle(Geom_SurfaceOfRevolution) revolution =
                new Geom_SurfaceOfRevolution(circle, gp_Ax1(gp::Origin(), surface->getRevolutionDirection()));
        gp_Pnt p0 = revolution->Value(0., 0.);

        pointF->SetComponent(new Geom_CartesianPoint(p0));
        labelF->SetPosition(gp_Pnt().Transformed(surface->Transformation()));
        editorFocal->update(gp_Pnt(), p0, gp_Pln());
    }

    Handle(InteractiveSurfaceCircleOfRevol) surface;

    Handle(AIS_Point) pointF;
    Handle(AIS_TextLabel) labelF;
    Handle(DimentionLenghtEditor) editorFocal;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveObjectSurfaceCircleOfRevolEditor, InteractiveObjectEditor)

InteractiveObjectSurfaceCircleOfRevolEditor::InteractiveObjectSurfaceCircleOfRevolEditor(const Handle(InteractiveSurfaceCircleOfRevol) &surface)
    : InteractiveObjectEditor(surface)
    , d(new InteractiveObjectSurfaceCircleOfRevolEditorPrivate)
{
    d->surface = surface;

    d->pointF = new Point;
    AddChild(d->pointF);
    d->labelF = new AIS_TextLabel;
    d->labelF->SetText("R");
    d->labelF->SetZLayer(Graphic3d_ZLayerId_Topmost);
    d->editorFocal = new DimentionLenghtEditor(gp_Pnt(), gp_Pnt(1., 1., 1.), gp_Pln());
    AddChild(d->editorFocal);

    d->update();
}

InteractiveObjectSurfaceCircleOfRevolEditor::~InteractiveObjectSurfaceCircleOfRevolEditor()
{
    delete d;
}

void InteractiveObjectSurfaceCircleOfRevolEditor::SetContext(const Handle(AIS_InteractiveContext) &ctx)
{
    if (!ctx) {
        AddChild(d->labelF);
    }
    AIS_InteractiveObject::SetContext(ctx);
    if (ctx) {
        ctx->Display(d->labelF, Standard_True);
        ctx->Deactivate(d->labelF);
    }
}

void InteractiveObjectSurfaceCircleOfRevolEditor::editorChanged(const Handle(AIS_InteractiveObject) &editor)
{
    auto dimentionEditor = Handle(DimentionLenghtEditor)::DownCast(editor);
    if (dimentionEditor) {
        Standard_Real value = dimentionEditor->dimention();
        if (dimentionEditor == d->editorFocal) {
            d->surface->setRadius(value);
        }
    }
}

void InteractiveObjectSurfaceCircleOfRevolEditor::updateEditors()
{
    d->update();

    if (GetContext()) {
        GetContext()->RecomputePrsOnly(d->labelF, Standard_True);
    }
}

}
