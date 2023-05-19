#include "interactiveobjectsurfaceellipseofrevoleditor.h"

#include <AIS_InteractiveContext.hxx>
#include <AIS_Point.hxx>
#include <AIS_TextLabel.hxx>
#include <Geom_CartesianPoint.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_SurfaceOfRevolution.hxx>

#include "../Objects/interactivesurfaceellipseofrevol.h"

#include "dimentionlenghteditor.h"

namespace ExamplesBase {

class InteractiveObjectSurfaceEllipseOfRevolEditorPrivate
{
    friend class InteractiveObjectSurfaceEllipseOfRevolEditor;

    void update() {
        gp_Ax2 axes;
        Handle(Geom_Ellipse) ellipse = new Geom_Ellipse(axes, surface->majorR(), surface->minorR());
        Handle(Geom_SurfaceOfRevolution) revolution =
                new Geom_SurfaceOfRevolution(ellipse, gp_Ax1(gp::Origin(), surface->getRevolutionDirection()));
        gp_Pnt p0;// = revolution->Value(0., 0.);

        pointF->SetComponent(new Geom_CartesianPoint(ellipse->Focus1()));
        labelF->SetPosition(ellipse->Focus1().Transformed(surface->Transformation()));
        editorFocal->update(p0, ellipse->Focus1(), gp_Pln());
    }

    Handle(InteractiveSurfaceEllipseOfRevol) surface;

    Handle(AIS_Point) pointF;
    Handle(AIS_TextLabel) labelF;
    Handle(DimentionLenghtEditor) editorFocal;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveObjectSurfaceEllipseOfRevolEditor, InteractiveObjectEditor)

InteractiveObjectSurfaceEllipseOfRevolEditor::InteractiveObjectSurfaceEllipseOfRevolEditor(const Handle(InteractiveSurfaceEllipseOfRevol) &surface)
    : InteractiveObjectEditor(surface)
    , d(new InteractiveObjectSurfaceEllipseOfRevolEditorPrivate)
{
    d->surface = surface;

    d->pointF = new AIS_Point(new Geom_CartesianPoint(gp_Pnt()));
    AddChild(d->pointF);
    d->labelF = new AIS_TextLabel;
    d->labelF->SetText("F1");
    d->labelF->SetZLayer(Graphic3d_ZLayerId_Topmost);
    d->editorFocal = new DimentionLenghtEditor(gp_Pnt(), gp_Pnt(1., 1., 1.), gp_Pln());
    AddChild(d->editorFocal);;

    d->update();
}

InteractiveObjectSurfaceEllipseOfRevolEditor::~InteractiveObjectSurfaceEllipseOfRevolEditor()
{
    delete d;
}

void InteractiveObjectSurfaceEllipseOfRevolEditor::SetContext(const Handle(AIS_InteractiveContext) &ctx)
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

void InteractiveObjectSurfaceEllipseOfRevolEditor::editorChanged(const Handle(AIS_InteractiveObject) &editor)
{
    auto dimentionEditor = Handle(DimentionLenghtEditor)::DownCast(editor);
    if (dimentionEditor) {
        Standard_Real value = dimentionEditor->dimention();
        if (dimentionEditor == d->editorFocal) {
            d->surface->setFocal(value);
        }
    }
}

void InteractiveObjectSurfaceEllipseOfRevolEditor::updateEditors()
{
    d->update();

    auto ctx = GetContext();
    if (ctx) {
        ctx->RecomputePrsOnly(d->labelF, Standard_True);
    }
}

}
