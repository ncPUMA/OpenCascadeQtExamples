#include "viewport.h"

#include <QAbstractItemView>
#include <QDebug>
#include <QMenu>
#include <QMouseEvent>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <V3d_View.hxx>

#include "ObjectModels/interactiveobjectitemmodel.h"
#include "ObjectModels/interactiveobjectitemmodelcreator.h"
#include "ObjectModels/interactiveobjectitemmodeldelegate.h"
#include "Objects/interactivesurfaceplane.h"

class ViewportPrivate
{
    friend class Viewport;

    void addToContext(const Handle(InteractiveObject) &object, const Graphic3d_Vec3d &translateion) {
        auto ctx = q_ptr->context();
        if (ctx) {
            ctx->Display(object, Standard_True);
            gp_Trsf trsf;
            trsf.SetTranslation(gp_XYZ(translateion.x(), translateion.y(), translateion.z()));
            ctx->SetLocation(object, trsf);
            object->setName(QString("%1_%2").arg(object->name()).arg(++objectCounter));
        }
    }

    Viewport *q_ptr = nullptr;
    QAbstractItemView *mPropertyView = nullptr;
    int objectCounter = 0;
};

Viewport::Viewport(QWidget *parent)
    : ExamplesBase::Viewport(parent)
    , d_ptr(new ViewportPrivate)
{
    d_ptr->q_ptr = this;

    context()->IsoOnPlane(Standard_True);
    context()->SetIsoNumber(10);
}

Viewport::~Viewport()
{
    delete d_ptr;
}

void Viewport::setPropertyView(QAbstractItemView *view)
{
    d_ptr->mPropertyView = view;
    auto delegate = new InteractiveObjectItemModelDelegate(view);
    view->setItemDelegateForColumn(1, delegate);
}

bool Viewport::selectionChanged()
{
    delete d_ptr->mPropertyView->model();

    auto ctx = context();
    ctx->InitSelected();
    if (ctx->MoreSelected()) {
        auto interactive = Handle(InteractiveObject)::DownCast(ctx->SelectedInteractive());
        if (interactive) {
            InteractiveObjectItemModelCreator creator;
            auto model = creator.createModel(interactive);
            model->update();
            d_ptr->mPropertyView->setModel(model);
        } else {
            d_ptr->mPropertyView->setModel(nullptr);
        }
    } else {
        d_ptr->mPropertyView->setModel(nullptr);
    }
    return false;
}

bool Viewport::mouseReleased(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        const Graphic3d_Vec2i aPnt(event->pos().x(), event->pos().y());

        Handle(InteractiveObject) object;
        Graphic3d_Vec3d pickedPoint, projection;
        view()->ConvertWithProj(aPnt.x(), aPnt.y(),
                                pickedPoint.x(), pickedPoint.y(), pickedPoint.z(),
                                projection.x(), projection.y(), projection.z());

        auto ctx = context();
        ctx->MainSelector()->Pick(aPnt.x(), aPnt.y(), view());
        if (ctx->MainSelector()->NbPicked()) {
            auto owner = ctx->MainSelector()->Picked(1);
            if (owner) {
                object = Handle(InteractiveObject)::DownCast(owner->Selectable());
                auto point = ctx->MainSelector()->PickedPoint(1);
                if (object) {
                    point.Transform(ctx->Location(object).Transformation().Inverted());
                    pickedPoint.SetValues(point.X(), point.Y(), point.Z());
                }
            }
        }

        QMenu topMenu;
        auto addMenu = topMenu.addMenu(tr("Add"));
        addMenu->addAction(tr("Plane"), this, [this, object, pickedPoint](){
            auto plane = new InteractiveSurfacePlane;
            plane->setName(tr("Plane"));
            if (object) {
                object->AddChild(plane);
            }
            d_ptr->addToContext(plane, pickedPoint);
        });
        return topMenu.exec(event->globalPos()) != nullptr;
    }
    return false;
}
