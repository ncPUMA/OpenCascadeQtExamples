#include "viewport.h"

#include <QAbstractItemView>
#include <QDebug>
#include <QMenu>
#include <QMouseEvent>
#include <QTimer>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Manipulator.hxx>
#include <AIS_Shape.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <V3d_View.hxx>

#include "Editors/interactiveobjecteditor.h"
#include "Editors/interactiveobjecteditorcreator.h"
#include "ObjectModels/interactiveobjectitemmodelshape.h"
#include "ObjectModels/interactiveobjectitemmodelcreator.h"
#include "ObjectModels/interactiveobjectitemmodeldelegate.h"
#include "ObjectModels/objectstreemodel.h"
#include "Objects/interactiveshape.h"
#include "Objects/interactivesurfacecircleofrevol.h"
#include "Objects/interactivesurfaceellipseofrevol.h"
#include "Objects/interactivesurfacehyperofrevol.h"
#include "Objects/interactivesurfaceparabofrevol.h"
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

class ObjectManipulator : public AIS_Manipulator
{
public:
    ObjectManipulator(Viewport *viewport)
        : AIS_Manipulator()
        , view(viewport)
    { }

    Standard_Boolean ProcessDragging (const Handle(AIS_InteractiveContext)& theCtx,
                                      const Handle(V3d_View)& theView,
                                      const Handle(SelectMgr_EntityOwner)& theOwner,
                                      const Graphic3d_Vec2i& theDragFrom,
                                      const Graphic3d_Vec2i& theDragTo,
                                      const AIS_DragAction theAction) Standard_OVERRIDE {
        auto ret = AIS_Manipulator::ProcessDragging(theCtx, theView, theOwner,
                                                    theDragFrom, theDragTo, theAction);
        if (IsAttached()) {
            theCtx->RecomputeSelectionOnly(Object());
            view->objectsChanged();
        }
        return ret;
    }

private:
    Viewport *view;
};

class ViewportPrivate
{
    friend class Viewport;
    friend class ObjectObserver;

    bool menuRequest(const QPoint &menuPos, const gp_XYZ &pickedPoint, const Handle(InteractiveObject) &object) {
        auto ctx = q_ptr->context();
        if (!ctx) {
            return false;
        }

        QMenu topMenu;
        auto addMenu = topMenu.addMenu(Viewport::tr("Add"));
        addMenu->addAction(Viewport::tr("Plane"), q_ptr, [this, object, pickedPoint]() {
            auto plane = new InteractiveSurfacePlane;
            addToContext(plane, pickedPoint, Viewport::tr("Plane"), object);
        });
        addMenu->addAction(Viewport::tr("Circle of revol"), q_ptr, [this, object, pickedPoint]() {
            auto parab = new InteractiveSurfaceCircleOfRevol;
            addToContext(parab, pickedPoint, Viewport::tr("CircleOfRevol"), object);
        });
        addMenu->addAction(Viewport::tr("Ellipse of revol"), q_ptr, [this, object, pickedPoint]() {
            auto ellipse = new InteractiveSurfaceEllipseOfRevol;
            addToContext(ellipse, pickedPoint, Viewport::tr("EllipseOfRevol"), object);
        });
        addMenu->addAction(Viewport::tr("Parabola of revol"), q_ptr, [this, object, pickedPoint]() {
            auto parab = new InteractiveSurfaceParabOfRevol;
            addToContext(parab, pickedPoint, Viewport::tr("ParabOfRevol"), object);
        });
        addMenu->addAction(Viewport::tr("Hyperbola of revol"), q_ptr, [this, object, pickedPoint]() {
            auto parab = new InteractiveSurfaceHyperOfRevol;
            addToContext(parab, pickedPoint, Viewport::tr("HyperOfRevol"), object);
        });
        addMenu->addSeparator();
        addMenu->addAction(Viewport::tr("Custom shape..."), q_ptr, [this, object, pickedPoint]() {
            auto path = InteractiveObjectItemModelShape::requestFilename(q_ptr);
            if (!path.isEmpty()) {
                auto shape = new InteractiveShape;
                shape->setModelPath(path);
                addToContext(shape, pickedPoint, Viewport::tr("Shape"), object);
            }
        });

        if (object) {
            topMenu.addAction(Viewport::tr("Remove"), q_ptr, [this, object, ctx]() {
                if (manipulator->IsAttached() && manipulator->Object() == object) {
                    manipulator->Detach();
                    ctx->Remove(manipulator, Standard_False);
                }

                if (editor && editor->Parent() && editor->Parent() == object) {
                    object->RemoveChild(editor);
                    ctx->Remove(editor, Standard_True);
                    editor = nullptr;
                }

                if (object->Parent()) {
                    object->Parent()->RemoveChild(object);
                }

                ctx->Remove(object, Standard_False);
                if (mObjectsView) {
                    auto model = static_cast<ObjectsTreeModel *>(mObjectsView->model());
                    model->removeObject(object);
                }

                auto it = objectObservers.find(object);
                if (it != objectObservers.end()) {
                    delete it->second;
                    objectObservers.erase(it);
                }
            });
            topMenu.addSeparator();
            if (!manipulator->IsAttached() || manipulator->Object() != object) {
                topMenu.addAction(Viewport::tr("Transform"), q_ptr, [this, object, ctx]() {
                    removeEditor();

                    if (manipulator->IsAttached()) {
                        manipulator->Detach();
                    }

                    auto ax = gp_Ax2().Transformed(object->Transformation());
                    auto bndBox = object->BoundingBox().Transformed(object->Transformation());
                    ax.SetLocation(bndBox.CornerMin().XYZ() + (bndBox.CornerMax().XYZ() - bndBox.CornerMin().XYZ()) / 2.);
                    manipulator->SetPosition(ax);

                    AIS_Manipulator::OptionsForAttach options;
                    options.SetAdjustPosition(Standard_False);
                    options.SetAdjustSize(Standard_False);
                    options.SetEnableModes(Standard_True);
                    manipulator->Attach(object, options);
                    if (!ctx->IsDisplayed(manipulator)) {
                        ctx->Display(manipulator, Standard_False);
                    }
                    ctx->Redisplay(manipulator, Standard_False);
                });
            }
            if (!editor || !editor->Parent() || editor->Parent() != object) {
                topMenu.addAction(Viewport::tr("Edit"), q_ptr, [this, object, ctx]() {
                    if (ctx->IsDisplayed(manipulator)) {
                        manipulator->Detach();
                        ctx->Remove(manipulator, Standard_True);
                    }
                    removeEditor();

                    ctx->Deactivate(object);
                    InteractiveObjectEditorCreator creator;
                    editor = creator.create(object);
                    if (editor) {
                        object->AddChild(editor);
                        ctx->Display(editor, Standard_False);
                    }
                });
            }
        }

        if (ctx->IsDisplayed(manipulator)) {
            topMenu.addAction(Viewport::tr("End transform"), q_ptr, [this, ctx]() {
                manipulator->Detach();
                ctx->Remove(manipulator, Standard_True);
            });
        }
        if (ctx->IsDisplayed(editor)) {
            topMenu.addAction(Viewport::tr("End edit"), q_ptr, [this]() {
                removeEditor();
            });
        }
        return topMenu.exec(menuPos) != nullptr;
    }

    void addToContext(const Handle(InteractiveObject) &object, const gp_XYZ &translation,
                      const QString &name, const Handle(InteractiveObject) &parent) {
        if (parent) {
            object->AddChild(object);
        }
        auto ctx = q_ptr->context();
        if (ctx) {
            ctx->Display(object, Standard_False);
            gp_Trsf trsf;
            trsf.SetTranslation(translation);
            ctx->SetLocation(object, trsf);
            object->setName(QString("%1_%2").arg(name).arg(++objectCounter));
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

    void removeEditor() {
        if (editor) {
            auto parent = editor->Parent();
            if (parent) {
                parent->RemoveChild(editor);
            }
            auto ctx = q_ptr->context();
            if (ctx) {
                ctx->Remove(editor, Standard_True);
                auto interactive = Handle(AIS_InteractiveObject)::DownCast(parent);
                if (interactive) {
                    ctx->Activate(interactive);
                }
            }
        }
    }

    Viewport *q_ptr = nullptr;
    QAbstractItemView *mObjectsView = nullptr;
    QAbstractItemView *mPropertyView = nullptr;
    int objectCounter = 0;
    std::map<Handle(InteractiveObject), ObjectObserver *> objectObservers;
    QTimer *observerCompressor = nullptr;
    Handle(AIS_Manipulator) manipulator;
    Handle(InteractiveObjectEditor) editor;
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
    d_ptr->observerCompressor->setInterval(50);
    connect(d_ptr->observerCompressor, &QTimer::timeout, this, [this]() {
        auto ctx = context();
        if (ctx) {
            if (d_ptr->mObjectsView) {
                auto model = static_cast<ObjectsTreeModel *>(d_ptr->mObjectsView->model());
                model->update(ctx);
                d_ptr->updateTreeViewSelection();
            }

            d_ptr->updatePropertyView();

            if (d_ptr->manipulator->IsAttached()) {
                auto object = Handle(InteractiveObject)::DownCast(d_ptr->manipulator->Object());
                if (object) {
                    auto ax = gp_Ax2().Transformed(object->Transformation());
                    auto bndBox = object->BoundingBox().Transformed(object->Transformation());
                    ax.SetLocation(bndBox.CornerMin().XYZ() + (bndBox.CornerMax().XYZ() - bndBox.CornerMin().XYZ()) / 2.);
                    d_ptr->manipulator->SetPosition(ax);
                    ctx->Redisplay(d_ptr->manipulator, Standard_True);
                }
            }

            if (d_ptr->editor) {
                d_ptr->editor->update();
            }
        }
    });

    d_ptr->manipulator = new ObjectManipulator(this);
    d_ptr->manipulator->SetPart(0, AIS_MM_Scaling, Standard_False);
    d_ptr->manipulator->SetPart(1, AIS_MM_Scaling, Standard_False);
    d_ptr->manipulator->SetPart(2, AIS_MM_Scaling, Standard_False);
    d_ptr->manipulator->SetModeActivationOnDetection(Standard_True);
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

            auto emptyShapeCurrent = Handle(InteractiveShape)::DownCast(current);
            if (emptyShapeCurrent && !emptyShapeCurrent->isValid()) {
                if (!d_ptr->mPropertyView) {
                    return;
                }

                delete d_ptr->mPropertyView->model();

                InteractiveObjectItemModel *model = nullptr;
                InteractiveObjectItemModelCreator creator;
                model = creator.createModel(current);
                model->update();
                d_ptr->mPropertyView->setModel(model);
            }
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
    if (!d_ptr->observerCompressor->isActive()) {
        d_ptr->observerCompressor->start();
    }
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
