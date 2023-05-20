#include "viewport.h"

#include <QDebug>
#include <QMenu>
#include <QMouseEvent>

#include <AIS_InteractiveContext.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <V3d_View.hxx>

#include <ExamplesBase/Objects/interactivecylinder.h>
#include <ExamplesBase/Objects/interactivesphere.h>

class ViewportPrivate
{
    friend class Viewport;
};

Viewport::Viewport()
    : ExamplesBase::InteractiveObjectsViewport()
    , d_ptr(new ViewportPrivate)
{

}

Viewport::~Viewport()
{
    delete d_ptr;
}

bool Viewport::mouseReleased(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        const Graphic3d_Vec2i aPnt(event->pos().x(), event->pos().y());

        Handle(AIS_Shape) object;
        Graphic3d_Vec3d pickedPoint, projection;
        view()->ConvertWithProj(aPnt.x(), aPnt.y(),
                                pickedPoint.x(), pickedPoint.y(), pickedPoint.z(),
                                projection.x(), projection.y(), projection.z());

        auto ctx = context();
        ctx->MainSelector()->Pick(aPnt.x(), aPnt.y(), view());
        if (ctx->MainSelector()->NbPicked()) {
            auto owner = ctx->MainSelector()->Picked(1);
            if (owner) {
                object = Handle(AIS_Shape)::DownCast(owner->Selectable());
                auto point = ctx->MainSelector()->PickedPoint(1);
                if (object) {
                    point.Transform(ctx->Location(object).Transformation().Inverted());
                    pickedPoint.SetValues(point.X(), point.Y(), point.Z());
                }
            }
        }

        gp_XYZ translation(pickedPoint.x(), pickedPoint.y(), pickedPoint.z());
        QMenu menu;
        menuRequest(object, translation, menu);
        return menu.exec(event->globalPos()) != nullptr;
    }
    return false;
}

void Viewport::menuRequest(const Handle(AIS_Shape) &object,
                           const gp_XYZ &pickedPoint,
                           QMenu &menu)
{
    if (!object) {
        menu.addAction(tr("Cylinder"), this, [this, object, pickedPoint]() {
            auto cylinder = new ExamplesBase::InteractiveCylinder;
            addToContext(cylinder, pickedPoint, tr("Cylinder"), object);
        });
        menu.addAction(tr("Sphere"), this, [this, object, pickedPoint]() {
            auto cylinder = new ExamplesBase::InteractiveSphere;
            addToContext(cylinder, pickedPoint, tr("Sphere"), object);
        });
    } else {
        menu.addAction(tr("Remove"), this, [this, object]() {
            removeFromContext(object);
        });
        menu.addSeparator();
        if (manipulatorAttachedObject() != object) {
            menu.addAction(tr("Transform"), this, [this, object]() {
                showManipulator(object);
            });
        }
        auto interactive = Handle(ExamplesBase::InteractiveObject)::DownCast(object);
        if (interactive && editorAttachedObject() != interactive) {
            menu.addAction(tr("Edit"), this, [this, interactive]() {
                showEditor(interactive);
            });
        }
    }

    if (manipulatorAttachedObject()) {
        menu.addAction(tr("End transform"), this, [this]() {
            removeManipulator();
        });
    }
    if (editorAttachedObject()) {
        menu.addAction(tr("End edit"), this, [this]() {
            removeEditor();
        });
    }

    if (object) {
        menu.addSeparator();
        menu.addAction(tr("Try END operation"), this, [this, object]() {
            auto ctx = context();
            TopoDS_Shape currentShape = object->Shape();
            currentShape.Location(ctx->Location(object));
            AIS_ListOfInteractive list;
            ctx->ObjectsInside(list, AIS_KOI_Shape);
            for (const auto &obj : list) {
                auto target = Handle(AIS_Shape)::DownCast(obj);
                if (!target || target == object) {
                    continue;
                }

                auto targetShape = target->Shape();
                targetShape.Location(ctx->Location(target));
                TopoDS_Shape res = BRepAlgoAPI_Common(currentShape, targetShape);
                if (res.IsNull()) {
                    qDebug() << "err";
                    continue;
                }

                removeFromContext(object);
                removeFromContext(target);
                addToContext(new AIS_Shape(res), gp_XYZ(), tr(""), nullptr);
                return;
            }
        });
    }
}
