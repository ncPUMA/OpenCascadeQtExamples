#include "viewport.h"

#include <QAbstractItemView>
#include <QDebug>
#include <QMenu>
#include <QMouseEvent>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <StdSelect_BRepOwner.hxx>

#include "Surfaces/interactiveobjectitemmodel.h"
#include "Surfaces/interactiveobjectitemmodelcreator.h"
#include "Surfaces/interactiveobjectitemmodeldelegate.h"
#include "Surfaces/interactivesurfaceplane.h"

class ViewportPrivate
{
    friend class Viewport;

    void clear() {
        if (mSurface) {
            q_ptr->context()->Remove(mSurface, Standard_False);
            mSurface.reset(nullptr);
        }
    }

    Viewport *q_ptr = nullptr;
    QAbstractItemView *mPropertyView = nullptr;
    Handle(AIS_Shape) mSurface;
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
        QMenu menu;
        menu.addAction(tr("Plane"), this, [this](){
            d_ptr->clear();
            auto surface = new InteractiveSurfacePlane;
            surface->setName(tr("Surface 1"));
            d_ptr->mSurface = surface;
            context()->Display(d_ptr->mSurface, Standard_False);
        });
        return menu.exec(event->globalPos()) != nullptr;
    }
    return false;
}
