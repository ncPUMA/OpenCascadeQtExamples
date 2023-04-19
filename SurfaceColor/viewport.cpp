#include "viewport.h"

#include <QDebug>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <TopoDS.hxx>
#include <V3d_View.hxx>

#include <ExamplesBase/ModelLoader/abstractmodelloader.h>
#include <ExamplesBase/ModelLoader/modelloaderfactorymethod.h>

#include "coloredshape.h"

class ViewportPrivate
{
    friend class Viewport;

    Viewport *q_ptr = nullptr;
    Handle(ColoredShape) shape;
};

Viewport::Viewport(QWidget *parent)
    : ExamplesBase::Viewport(parent)
    , d_ptr(new ViewportPrivate)
{
    d_ptr->q_ptr = this;

    Handle(AIS_Trihedron) trihedron;
    auto path = QStringLiteral("../Models/sample_telescope_mirrors.step");
    ExamplesBase::ModelLoaderFactoryMethod factory;
    auto &loader = factory.loaderByFName(path);
    auto topo_shape = loader.load(path.toLatin1().constData());
    if (topo_shape.IsNull()) {
        return;
    }

    d_ptr->shape = new ColoredShape(topo_shape);
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
    if (!owner || owner->Shape().IsNull() || owner->Shape().ShapeType() != TopAbs_FACE) {
        return false;
    }

    d_ptr->shape->resetColors();
    d_ptr->shape->setFaceColor(TopoDS::Face(owner->Shape()), Quantity_NOC_RED);
    ctx->RecomputePrsOnly(d_ptr->shape, Standard_True);
    return true;
}
