/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactiveobjectsurfaceparabofrevoleditor.h"

#include <AIS_InteractiveContext.hxx>
#include <AIS_Point.hxx>
#include <AIS_TextLabel.hxx>
#include <Geom_CartesianPoint.hxx>
#include <Geom_Parabola.hxx>
#include <Geom_SurfaceOfRevolution.hxx>

#include "../Objects/interactivesurfaceparabofrevol.h"

#include "dimentionlenghteditor.h"

namespace ExamplesBase {

class InteractiveObjectSurfaceParabOfRevolEditorPrivate
{
    friend class InteractiveObjectSurfaceParabOfRevolEditor;

    void update() {
        gp_Ax2 axes;
        Handle(Geom_Parabola) parabola = new Geom_Parabola(axes, surface->getFocal());
        Handle(Geom_SurfaceOfRevolution) revolution =
                new Geom_SurfaceOfRevolution(parabola, gp_Ax1(gp::Origin(), surface->getRevolutionDirection()));
        gp_Pnt p0 = revolution->Value(0., 0.);

        pointF->SetComponent(new Geom_CartesianPoint(parabola->Focus()));
        labelF->SetPosition(parabola->Focus().Transformed(surface->Transformation()));
        editorFocal->update(p0, parabola->Focus(), gp_Pln());
    }

    Handle(InteractiveSurfaceParabOfRevol) surface;

    Handle(AIS_Point) pointF;
    Handle(AIS_TextLabel) labelF;
    Handle(DimentionLenghtEditor) editorFocal;
};

IMPLEMENT_STANDARD_RTTIEXT(InteractiveObjectSurfaceParabOfRevolEditor, InteractiveObjectEditor)

InteractiveObjectSurfaceParabOfRevolEditor::InteractiveObjectSurfaceParabOfRevolEditor(const Handle(InteractiveSurfaceParabOfRevol) &surface)
    : InteractiveObjectEditor(surface)
    , d(new InteractiveObjectSurfaceParabOfRevolEditorPrivate)
{
    d->surface = surface;

    d->pointF = new AIS_Point(new Geom_CartesianPoint(gp_Pnt()));
    AddChild(d->pointF);
    d->labelF = new AIS_TextLabel;
    d->labelF->SetText("F");
    d->labelF->SetZLayer(Graphic3d_ZLayerId_Topmost);
    d->editorFocal = new DimentionLenghtEditor(gp_Pnt(), gp_Pnt(1., 1., 1.), gp_Pln());
    AddChild(d->editorFocal);

    d->update();
}

InteractiveObjectSurfaceParabOfRevolEditor::~InteractiveObjectSurfaceParabOfRevolEditor()
{
    delete d;
}

void InteractiveObjectSurfaceParabOfRevolEditor::SetContext(const Handle(AIS_InteractiveContext) &ctx)
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

void InteractiveObjectSurfaceParabOfRevolEditor::editorChanged(const Handle(AIS_InteractiveObject) &editor)
{
    auto dimentionEditor = Handle(DimentionLenghtEditor)::DownCast(editor);
    if (dimentionEditor) {
        Standard_Real value = dimentionEditor->dimention();
        if (dimentionEditor == d->editorFocal) {
            d->surface->setFocal(value);
        }
    }
}

void InteractiveObjectSurfaceParabOfRevolEditor::updateEditors()
{
    d->update();

    if (GetContext()) {
        GetContext()->RecomputePrsOnly(d->labelF, Standard_True);
    }
}

}
