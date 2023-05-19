#include "viewport.h"

#include <QMenu>
#include <QMouseEvent>

#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>

#include <ExamplesBase/Objects/interactivecylinder.h>

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
        menu.addAction(Viewport::tr("Cylinder"), this, [this, object, pickedPoint]() {
            auto cylinder = new ExamplesBase::InteractiveCylinder;
            addToContext(cylinder, pickedPoint, Viewport::tr("Cylinder"), object);
        });
    } else {
        menu.addAction(Viewport::tr("Remove"), this, [this, object]() {
            removeFromContext(object);
        });
        menu.addSeparator();
        if (manipulatorAttachedObject() != object) {
            menu.addAction(Viewport::tr("Transform"), this, [this, object]() {
                showManipulator(object);
            });
        }
        auto interactive = Handle(ExamplesBase::InteractiveObject)::DownCast(object);
        if (interactive && editorAttachedObject() != interactive) {
            menu.addAction(Viewport::tr("Edit"), this, [this, interactive]() {
                showEditor(interactive);
            });
        }
    }

    if (manipulatorAttachedObject()) {
        menu.addAction(Viewport::tr("End transform"), this, [this]() {
            removeManipulator();
        });
    }
    if (editorAttachedObject()) {
        menu.addAction(Viewport::tr("End edit"), this, [this]() {
            removeEditor();
        });
    }
}
