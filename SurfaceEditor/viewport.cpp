#include "viewport.h"

#include <QAbstractItemView>
#include <QDebug>
#include <QMenu>
#include <QMouseEvent>
#include <QTimer>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <V3d_View.hxx>

#include "ObjectModels/interactiveobjectitemmodel.h"
#include "ObjectModels/interactiveobjectitemmodelcreator.h"
#include "ObjectModels/interactiveobjectitemmodeldelegate.h"
#include "ObjectModels/objectstreemodel.h"
#include "Objects/interactivesurfaceplane.h"

class ObjectObserver : public InteractiveObjectObserver
{
public:
    ObjectObserver(Viewport *viewport)
        : InteractiveObjectObserver()
        , view(viewport)
    { }

    void changed() final { view->objectsChanged(); }

private:
    Viewport *view;
};

class ViewportPrivate
{
    friend class Viewport;
    friend class ObjectObserver;

    bool menuRequest(const QPoint &menuPos, const gp_XYZ &pickedPoint, const Handle(InteractiveObject) &object) {
        QMenu topMenu;

        auto addMenu = topMenu.addMenu(Viewport::tr("Add"));
        addMenu->addAction(Viewport::tr("Plane"), q_ptr, [this, object, pickedPoint](){
            auto plane = new InteractiveSurfacePlane;
            plane->setName(Viewport::tr("Plane"));
            if (object) {
                object->AddChild(plane);
            }
            addToContext(plane, pickedPoint);
        });

        if (object) {
            topMenu.addAction(Viewport::tr("Remove"), q_ptr, [this, object]() {
                if (object->Parent()) {
                    object->Parent()->RemoveChild(object);
                }
                auto ctx = q_ptr->context();
                if (ctx) {
                    ctx->Remove(object, Standard_False);
                    if (mObjectsView) {
                        auto model = static_cast<ObjectsTreeModel *>(mObjectsView->model());
                        model->removeObject(object);
                    }
                }
                auto it = objectObservers.find(object);
                if (it != objectObservers.end()) {
                    delete it->second;
                    objectObservers.erase(it);
                }
            });
            topMenu.addSeparator();
            topMenu.addAction(Viewport::tr("Transform"));
            topMenu.addAction(Viewport::tr("Change"));
        }
        return topMenu.exec(menuPos) != nullptr;
    }

    void addToContext(const Handle(InteractiveObject) &object, const gp_XYZ &translation) {
        auto ctx = q_ptr->context();
        if (ctx) {
            ctx->Display(object, Standard_False);
            gp_Trsf trsf;
            trsf.SetTranslation(translation);
            ctx->SetLocation(object, trsf);
            object->setName(QString("%1_%2").arg(object->name()).arg(++objectCounter));
            if (mObjectsView) {
                auto model = static_cast<ObjectsTreeModel *>(mObjectsView->model());
                model->addObject(object);
            }
            auto observer = new ObjectObserver(q_ptr);
            object->addObserver(*observer);
            objectObservers[object] = observer;
        }
    }

    void updateTreeViewSelection() {
        auto ctx = q_ptr->context();
        if (ctx && mObjectsView) {
            Handle(InteractiveObject) interactive;
            ctx->InitSelected();
            if (ctx->MoreSelected()) {
                interactive = Handle(InteractiveObject)::DownCast(ctx->SelectedInteractive());
            }

            if (!interactive) {
                mObjectsView->selectionModel()->clearSelection();
                return;
            }

            Handle(InteractiveObject) current;
            auto model = static_cast<ObjectsTreeModel *>(mObjectsView->model());
            auto indexes = mObjectsView->selectionModel()->selectedIndexes();
            if (!indexes.isEmpty()) {
                current = model->object(indexes.first());
            }

            if (current != interactive) {
                auto index = model->index(interactive);
                if (index.isValid()) {
                    mObjectsView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
                }
            }
        }
    }

    void updatePropertyView() {
        if (!mPropertyView) {
            return;
        }

        delete mPropertyView->model();

        InteractiveObjectItemModel *model = nullptr;
        auto ctx = q_ptr->context();
        ctx->InitSelected();
        if (ctx->MoreSelected()) {
            auto interactive = Handle(InteractiveObject)::DownCast(ctx->SelectedInteractive());
            if (interactive) {
                InteractiveObjectItemModelCreator creator;
                model = creator.createModel(interactive);
                model->update();
            }
        }
        mPropertyView->setModel(model);
    }

    Viewport *q_ptr = nullptr;
    QAbstractItemView *mObjectsView = nullptr;
    QAbstractItemView *mPropertyView = nullptr;
    int objectCounter = 0;
    std::map<Handle(InteractiveObject), ObjectObserver *> objectObservers;
    QTimer *observerCompressor = nullptr;
};

Viewport::Viewport(QWidget *parent)
    : ExamplesBase::Viewport(parent)
    , d_ptr(new ViewportPrivate)
{
    d_ptr->q_ptr = this;

    context()->IsoOnPlane(Standard_True);
    context()->SetIsoNumber(10);

    d_ptr->observerCompressor = new QTimer(this);
    d_ptr->observerCompressor->setSingleShot(true);
    d_ptr->observerCompressor->setInterval(100);
    connect(d_ptr->observerCompressor, &QTimer::timeout, this, [this]() {
        auto ctx = context();
        if (ctx) {
            if (d_ptr->mObjectsView) {
                auto model = static_cast<ObjectsTreeModel *>(d_ptr->mObjectsView->model());
                model->update(ctx);
                d_ptr->updateTreeViewSelection();
            }

            d_ptr->updatePropertyView();
        }
    });
}

Viewport::~Viewport()
{
    delete d_ptr;
}

void Viewport::setObjectsView(QAbstractItemView *objectsView)
{
    d_ptr->mObjectsView = objectsView;
    delete d_ptr->mObjectsView->model();
    auto model = new ObjectsTreeModel(d_ptr->mObjectsView);
    d_ptr->mObjectsView->setModel(model);
    auto ctx = context();
    if (ctx) {
        model->update(ctx);
    }

    connect(d_ptr->mObjectsView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this](const QItemSelection &selected, const QItemSelection &deselected) {
        Q_UNUSED(deselected);
        auto ctx = context();
        if (ctx) {
            Handle(InteractiveObject) current;
            if (!selected.indexes().isEmpty()) {
                auto model = static_cast<ObjectsTreeModel *>(d_ptr->mObjectsView->model());
                current = model->object(selected.indexes().first());
            }

            Handle(InteractiveObject) interactive;
            ctx->InitSelected();
            if (ctx->MoreSelected()) {
                interactive = Handle(InteractiveObject)::DownCast(ctx->SelectedInteractive());
            }

            if (!current) {
                ctx->ClearSelected(Standard_True);
            } else if (current != interactive) {
                ctx->SetSelected(current, Standard_True);
            }

            d_ptr->updatePropertyView();
        }
    });

    connect(d_ptr->mObjectsView, &QAbstractItemView::customContextMenuRequested,
            this, [this](const QPoint &point) {
        auto model = static_cast<ObjectsTreeModel *>(d_ptr->mObjectsView->model());
        auto index = d_ptr->mObjectsView->indexAt(point);
        if (d_ptr->menuRequest(d_ptr->mObjectsView->mapToGlobal(point),
                               gp_XYZ(),
                               model->object(index))) {
            view()->Redraw();
        }
    });
}

void Viewport::setPropertyView(QAbstractItemView *propertyView)
{
    d_ptr->mPropertyView = propertyView;
    auto delegate = new InteractiveObjectItemModelDelegate(propertyView);
    propertyView->setItemDelegateForColumn(1, delegate);
}

void Viewport::objectsChanged()
{
    d_ptr->observerCompressor->start();
}

bool Viewport::selectionChanged()
{
    d_ptr->updatePropertyView();
    d_ptr->updateTreeViewSelection();
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

        gp_XYZ translation(pickedPoint.x(), pickedPoint.y(), pickedPoint.z());
        return d_ptr->menuRequest(event->globalPos(), translation, object);
    }
    return false;
}
