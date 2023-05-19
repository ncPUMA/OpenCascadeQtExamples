#include "interactiveobjectsurfacehyperofrevoleditor.h"

#include <AIS_InteractiveContext.hxx>
#include <AIS_Point.hxx>
#include <AIS_TextLabel.hxx>
#include <Geom_CartesianPoint.hxx>
#include <Geom_Hyperbola.hxx>
#include <Geom_SurfaceOfRevolution.hxx>

#include "../Objects/interactivesurfacehyperofrevol.h"

#include "dimentionlenghteditor.h"

namespace ExamplesBase {

class InteractiveObjectSurfaceHyperOfRevolEditorPrivate
{
    friend class InteractiveObjectSurfaceHyperOfRevolEditor;

    void update() {
        gp_Ax2 axes;
        Handle(Geom_Hyperbola) hyperbola = new Geom_Hyperbola(axes, surface->majorR(), surface->minorR());
        Handle(Geom_SurfaceOfRevolution) revolution =
                new Geom_SurfaceOfRevolution(hyperbola, gp_Ax1(gp::Origin(), surface->getRevolutionDirection()));
        gp_Pnt p0;// = revolution->Value(0., 0.);

        pointF->SetComponent(new Geom_CartesianPoint(hyperbola->Focus1()));
        labelF->SetPosition(hyperbola->Focus1().Transformed(surface->Transformation()));
        editorFocal->update(p0, hyperbola->Focus1(), gp_Pln());
    }

    Handle(InteractiveSurfaceHyperOfRevol) surface;

    Handle(AIS_Point) pointF;
    Handle(AIS_TextLabel) labelF;
    Handle(DimentionLenghtEditor) editorFocal;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveObjectSurfaceHyperOfRevolEditor, InteractiveObjectEditor)

InteractiveObjectSurfaceHyperOfRevolEditor::InteractiveObjectSurfaceHyperOfRevolEditor(const Handle(InteractiveSurfaceHyperOfRevol) &surface)
    : InteractiveObjectEditor(surface)
    , d(new InteractiveObjectSurfaceHyperOfRevolEditorPrivate)
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

InteractiveObjectSurfaceHyperOfRevolEditor::~InteractiveObjectSurfaceHyperOfRevolEditor()
{
    delete d;
}

void InteractiveObjectSurfaceHyperOfRevolEditor::SetContext(const Handle(AIS_InteractiveContext) &ctx)
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

void InteractiveObjectSurfaceHyperOfRevolEditor::editorChanged(const Handle(AIS_InteractiveObject) &editor)
{
    auto dimentionEditor = Handle(DimentionLenghtEditor)::DownCast(editor);
    if (dimentionEditor) {
        Standard_Real value = dimentionEditor->dimention();
        if (dimentionEditor == d->editorFocal) {
            d->surface->setFocal(value);
        }
    }
}

void InteractiveObjectSurfaceHyperOfRevolEditor::updateEditors()
{
    d->update();

    auto ctx = GetContext();
    if (ctx) {
        ctx->RecomputePrsOnly(d->labelF, Standard_True);
    }
}

}
