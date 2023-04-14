#include "viewport.h"

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <V3d_View.hxx>

class ViewportPrivate
{
    friend class Viewport;

    Viewport *q_ptr = nullptr;
    Handle(AIS_Shape) shape, face;
};

Viewport::Viewport(QWidget *parent)
    : ExamplesBase::Viewport(parent)
    , d_ptr(new ViewportPrivate)
{
    d_ptr->q_ptr = this;

    d_ptr->shape = new AIS_Shape(BRepPrimAPI_MakeCylinder(100., 50.));
    context()->Display(d_ptr->shape, Standard_False);
    context()->SetDisplayMode(d_ptr->shape, AIS_Shaded, Standard_False);
    gp_Trsf trsf;
    trsf.SetTranslationPart(gp_Vec(150., 150., 150.));
    context()->SetLocation(d_ptr->shape, trsf);
    context()->Redisplay(d_ptr->shape, Standard_True);

    context()->SetSelectionModeActive(d_ptr->shape,
                                      AIS_Shape::SelectionMode(TopAbs_FACE),
                                      Standard_True,
                                      AIS_SelectionModesConcurrency_Single);
}

Viewport::~Viewport()
{
    delete d_ptr;
}

bool Viewport::selectionChanged()
{
    auto ctx = context();
    if (!ctx) {
        return false;
    }

    ctx->InitSelected();
    if (!ctx->MoreSelected()) {
        return false;
    }

    auto interactive = Handle(AIS_Shape)::DownCast(ctx->SelectedInteractive());
    if (interactive != d_ptr->shape) {
        return false;
    }

    auto owner = Handle(StdSelect_BRepOwner)::DownCast(ctx->SelectedOwner());
    if (!owner || owner->Shape().IsNull()) {
        return false;
    }

    if (d_ptr->face) {
        ctx->Remove(d_ptr->face, Standard_True);
    }

    d_ptr->face = new AIS_Shape(owner->Shape());
    d_ptr->face->SetColor(Quantity_NOC_RED);
    ctx->Display(d_ptr->face, Standard_False);
    ctx->SetDisplayMode(d_ptr->face, AIS_Shaded, Standard_False);
    ctx->SetLocation(d_ptr->face, ctx->Location(d_ptr->shape));
    ctx->Redisplay(d_ptr->face, Standard_True);
    return true;
}
