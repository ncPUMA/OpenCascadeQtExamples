#include "interactiveobjectsviewport.h"

#include <QAbstractItemView>
#include <QMenu>
#include <QTimer>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Manipulator.hxx>
#include <AIS_Shape.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <V3d_View.hxx>

#include "../Editors/interactiveobjecteditor.h"
#include "../Editors/interactiveobjecteditorcreator.h"
#include "../ObjectModels/interactiveobjectitemmodel.h"
#include "../ObjectModels/interactiveobjectitemmodelcreator.h"
#include "../ObjectModels/interactiveobjectitemmodeldelegate.h"
#include "../ObjectModels/objectstreemodel.h"
#include "../Objects/interactivecylinder.h"
#include "../Objects/interactiveshape.h"

namespace ExamplesBase {

class ObjectObserver : public InteractiveObjectObserver
{
public:
    ObjectObserver(InteractiveObjectsViewport *viewport)
        : InteractiveObjectObserver()
        , view(viewport)
    { }

    void changed() final { view->objectsChanged(); }

private:
    InteractiveObjectsViewport *view;
};

class ObjectManipulator : public AIS_Manipulator
{
public:
    ObjectManipulator(InteractiveObjectsViewport *viewport)
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
    InteractiveObjectsViewport *view;
};

class InteractiveObjectsViewportPrivate
{
    friend class InteractiveObjectsViewport;

    void addToContext(const Handle(AIS_InteractiveObject) &object, const gp_XYZ &translation,
                      const QString &name, const Handle(AIS_InteractiveObject) &parent)
    {
        if (parent) {
            parent->AddChild(object);
        }

        ctx->Display(object, Standard_False);
        gp_Trsf trsf;
        trsf.SetTranslation(translation);
        ctx->SetLocation(object, trsf);
        auto interactive = Handle(InteractiveObject)::DownCast(object);
        if (interactive) {
            interactive->setName(QString("%1_%2").arg(name).arg(++objectCounter));
            if (mObjectsView) {
                auto model = static_cast<ExamplesBase::ObjectsTreeModel *>(mObjectsView->model());
                model->addObject(interactive);
            }
            auto observer = new ObjectObserver(q_ptr);
            interactive->addObserver(*observer);
            objectObservers[interactive] = observer;
        }
    }

    void removeFromContext(const Handle(AIS_InteractiveObject) &object) {
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
        auto interactive = Handle(InteractiveObject)::DownCast(object);
        if (interactive) {
            if (mObjectsView) {
                auto model = static_cast<ExamplesBase::ObjectsTreeModel *>(mObjectsView->model());
                model->removeObject(interactive);
            }

            auto it = objectObservers.find(interactive);
            if (it != objectObservers.end()) {
                delete it->second;
                objectObservers.erase(it);
            }
        }
    }

    void updateTreeViewSelection() {
        if (mObjectsView) {
            Handle(ExamplesBase::InteractiveObject) interactive;
            ctx->InitSelected();
            if (ctx->MoreSelected()) {
                interactive = Handle(ExamplesBase::InteractiveObject)::DownCast(ctx->SelectedInteractive());
            }

            if (!interactive) {
                mObjectsView->selectionModel()->clearSelection();
                return;
            }

            Handle(ExamplesBase::InteractiveObject) current;
            auto model = static_cast<ExamplesBase::ObjectsTreeModel *>(mObjectsView->model());
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
        ctx->InitSelected();
        if (ctx->MoreSelected()) {
            auto interactive = Handle(ExamplesBase::InteractiveObject)::DownCast(ctx->SelectedInteractive());
            if (interactive) {
                InteractiveObjectItemModelCreator creator;
                model = creator.createModel(interactive);
                model->update();
            }
        }
        mPropertyView->setModel(model);
    }

    void showManipulator(const Handle(AIS_InteractiveObject) &object) {
        removeEditor();

        if (manipulator->IsAttached()) {
            manipulator->Detach();
        }

        auto ax = gp_Ax2().Transformed(object->Transformation());
        Bnd_Box bndBox;
        auto shapeObj = Handle(AIS_Shape)::DownCast(object);
        if (shapeObj) {
            bndBox = shapeObj->BoundingBox();
        } else {
            object->BoundingBox(bndBox);
        }
        bndBox = bndBox.Transformed(object->Transformation());
        ax.SetLocation(bndBox.CornerMin().XYZ() + (bndBox.CornerMax().XYZ() - bndBox.CornerMin().XYZ()) / 2.);
        manipulator->SetPosition(ax);

        AIS_Manipulator::OptionsForAttach options;
        options.SetAdjustPosition(Standard_False);
        options.SetAdjustSize(Standard_False);
        options.SetEnableModes(Standard_True);
        manipulator->Attach(object, options);
        if (!ctx->IsDisplayed(manipulator)) {
            ctx->Display(manipulator, Standard_False);
        } else {
            ctx->Redisplay(manipulator, Standard_False);
        }
    }

    void removeManipulator() {
        manipulator->Detach();
        ctx->Remove(manipulator, Standard_True);
    }

    void showEditor(const Handle(InteractiveObject) &object) {
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
    }

    void removeEditor() {
        if (editor) {
            auto parent = editor->Parent();
            if (parent) {
                parent->RemoveChild(editor);
            }

            ctx->Remove(editor, Standard_True);
            auto interactive = Handle(AIS_InteractiveObject)::DownCast(parent);
            if (interactive) {
                ctx->Activate(interactive);
            }
        }
    }

    InteractiveObjectsViewport *q_ptr = nullptr;
    Handle(AIS_InteractiveContext) ctx;
    QAbstractItemView *mObjectsView = nullptr;
    QAbstractItemView *mPropertyView = nullptr;
    int objectCounter = 0;
    std::map<Handle(ExamplesBase::InteractiveObject), ObjectObserver *> objectObservers;
    QTimer *observerCompressor = nullptr;
    Handle(AIS_Manipulator) manipulator;
    Handle(ExamplesBase::InteractiveObjectEditor) editor;
};

InteractiveObjectsViewport::InteractiveObjectsViewport(QWidget *parent)
    : Viewport(parent)
    , d_ptr(new InteractiveObjectsViewportPrivate)
{
    d_ptr->q_ptr = this;
    d_ptr->ctx = context();

    context()->IsoOnPlane(Standard_True);
    context()->SetIsoNumber(10);

    d_ptr->observerCompressor = new QTimer(this);
    d_ptr->observerCompressor->setSingleShot(true);
    d_ptr->observerCompressor->setInterval(50);
    connect(d_ptr->observerCompressor, &QTimer::timeout, this, [this]() {
        auto ctx = context();
        if (ctx) {
            if (d_ptr->mObjectsView) {
                auto model = static_cast<ExamplesBase::ObjectsTreeModel *>(d_ptr->mObjectsView->model());
                model->update(ctx);
                d_ptr->updateTreeViewSelection();
            }

            d_ptr->updatePropertyView();

            if (d_ptr->manipulator->IsAttached()) {
                auto object = Handle(ExamplesBase::InteractiveObject)::DownCast(d_ptr->manipulator->Object());
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

InteractiveObjectsViewport::~InteractiveObjectsViewport()
{
    delete d_ptr;
}

void InteractiveObjectsViewport::setObjectsView(QAbstractItemView *objectsView)
{
    d_ptr->mObjectsView = objectsView;
    delete d_ptr->mObjectsView->model();
    auto model = new ExamplesBase::ObjectsTreeModel(d_ptr->mObjectsView);
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
            Handle(ExamplesBase::InteractiveObject) current;
            if (!selected.indexes().isEmpty()) {
                auto model = static_cast<ExamplesBase::ObjectsTreeModel *>(d_ptr->mObjectsView->model());
                current = model->object(selected.indexes().first());
            }

            Handle(ExamplesBase::InteractiveObject) interactive;
            ctx->InitSelected();
            if (ctx->MoreSelected()) {
                interactive = Handle(ExamplesBase::InteractiveObject)::DownCast(ctx->SelectedInteractive());
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

    connect(objectsView, &QAbstractItemView::customContextMenuRequested,
            this, [this, objectsView](const QPoint &point) {
        auto model = static_cast<ExamplesBase::ObjectsTreeModel *>(objectsView->model());
        auto index = objectsView->indexAt(point);
        QMenu menu;
        objectsViewMenuRequest(model->object(index), menu);
        if (menu.exec(objectsView->mapToGlobal(point)) != nullptr) {
            view()->Redraw();
        }
    });
}

void InteractiveObjectsViewport::setPropertyView(QAbstractItemView *propertyView)
{
    d_ptr->mPropertyView = propertyView;
    auto delegate = new InteractiveObjectItemModelDelegate(propertyView);
    propertyView->setItemDelegateForColumn(1, delegate);
}

void InteractiveObjectsViewport::objectsChanged()
{
    if (!d_ptr->observerCompressor->isActive()) {
        d_ptr->observerCompressor->start();
    }
}

Handle(AIS_InteractiveObject) InteractiveObjectsViewport::manipulatorAttachedObject() const
{
    if (d_ptr->manipulator->IsAttached()) {
        return d_ptr->manipulator->Object();
    }
    return nullptr;
}

void InteractiveObjectsViewport::showManipulator(const Handle(AIS_InteractiveObject) &object)
{
    d_ptr->showManipulator(object);
}

void ExamplesBase::InteractiveObjectsViewport::removeManipulator()
{
    d_ptr->removeManipulator();
}

Handle(InteractiveObject) InteractiveObjectsViewport::editorAttachedObject() const
{
    Handle(InteractiveObject) obj;
    if (d_ptr->editor && d_ptr->editor->Parent()) {
        return Handle(InteractiveObject)::DownCast(d_ptr->editor->Parent());
    }
    return nullptr;
}

void InteractiveObjectsViewport::showEditor(const Handle(InteractiveObject) &object)
{
    d_ptr->showEditor(object);
}

void InteractiveObjectsViewport::removeEditor()
{
    d_ptr->removeEditor();
}

void InteractiveObjectsViewport::objectsViewMenuRequest(const Handle(AIS_InteractiveObject) &obj, QMenu &menu)
{
    Q_UNUSED(obj);
    Q_UNUSED(menu);
}

void InteractiveObjectsViewport::addToContext(const Handle(AIS_InteractiveObject) &object, const gp_XYZ &translation,
                                              const QString &name, const Handle(AIS_InteractiveObject) &parent)
{
    d_ptr->addToContext(object, translation, name, parent);
}

void InteractiveObjectsViewport::removeFromContext(const Handle(AIS_InteractiveObject) &object)
{
    d_ptr->removeFromContext(object);
}

bool InteractiveObjectsViewport::selectionChanged()
{
    d_ptr->updatePropertyView();
    d_ptr->updateTreeViewSelection();
    return false;
}

}
