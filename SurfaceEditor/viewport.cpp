/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "viewport.h"

#include <QAbstractItemView>
#include <QMenu>
#include <QMouseEvent>

#include <AIS_InteractiveContext.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <V3d_View.hxx>

#include <ExamplesBase/ObjectModels/objectstreemodel.h>
#include <ExamplesBase/ObjectModels/interactiveobjectitemmodelshape.h>
#include <ExamplesBase/Objects/interactivecuboid.h>
#include <ExamplesBase/Objects/interactivecylinder.h>
#include <ExamplesBase/Objects/interactiveshape.h>
#include <ExamplesBase/Objects/interactivesurfacecircleofrevol.h>
#include <ExamplesBase/Objects/interactivesurfaceellipseofrevol.h>
#include <ExamplesBase/Objects/interactivesurfacehyperofrevol.h>
#include <ExamplesBase/Objects/interactivesurfaceparabofrevol.h>
#include <ExamplesBase/Objects/interactivesurfaceplane.h>

Viewport::Viewport()
    : ExamplesBase::InteractiveObjectsViewport()
{

}

void Viewport::objectsViewMenuRequest(const Handle(AIS_InteractiveObject) &obj, QMenu &menu)
{
    auto interactive = Handle(ExamplesBase::InteractiveObject)::DownCast(obj);
    menuRequest(interactive, gp_XYZ(), menu);
}

bool Viewport::mouseReleased(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        const Graphic3d_Vec2i aPnt(event->pos().x(), event->pos().y());

        Handle(ExamplesBase::InteractiveObject) object;
        Graphic3d_Vec3d pickedPoint, projection;
        view()->ConvertWithProj(aPnt.x(), aPnt.y(),
                                pickedPoint.x(), pickedPoint.y(), pickedPoint.z(),
                                projection.x(), projection.y(), projection.z());

        auto ctx = context();
        ctx->MainSelector()->Pick(aPnt.x(), aPnt.y(), view());
        if (ctx->MainSelector()->NbPicked()) {
            auto owner = ctx->MainSelector()->Picked(1);
            if (owner) {
                object = Handle(ExamplesBase::InteractiveObject)::DownCast(owner->Selectable());
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

void Viewport::menuRequest(const Handle(ExamplesBase::InteractiveObject) &object,
                           const gp_XYZ &pickedPoint,
                           QMenu &menu)
{
    auto addMenu = menu.addMenu(tr("Add"));
    addMenu->addAction(tr("Plane"), this, [this, object, pickedPoint]() {
        auto plane = new ExamplesBase::InteractiveSurfacePlane;
        addToContext(plane, pickedPoint, tr("Plane"), object);
    });
    addMenu->addAction(tr("Circle of revol"), this, [this, object, pickedPoint]() {
        auto parab = new ExamplesBase::InteractiveSurfaceCircleOfRevol;
        addToContext(parab, pickedPoint, tr("CircleOfRevol"), object);
    });
    addMenu->addAction(tr("Ellipse of revol"), this, [this, object, pickedPoint]() {
        auto ellipse = new ExamplesBase::InteractiveSurfaceEllipseOfRevol;
        addToContext(ellipse, pickedPoint, tr("EllipseOfRevol"), object);
    });
    addMenu->addAction(tr("Parabola of revol"), this, [this, object, pickedPoint]() {
        auto parab = new ExamplesBase::InteractiveSurfaceParabOfRevol;
        addToContext(parab, pickedPoint, tr("ParabOfRevol"), object);
    });
    addMenu->addAction(tr("Hyperbola of revol"), this, [this, object, pickedPoint]() {
        auto parab = new ExamplesBase::InteractiveSurfaceHyperOfRevol;
        addToContext(parab, pickedPoint, tr("HyperOfRevol"), object);
    });
    addMenu->addSeparator();
    addMenu->addAction(tr("Cuboid"), this, [this, object, pickedPoint]() {
        auto cuboid = new ExamplesBase::InteractiveCuboid;
        addToContext(cuboid, pickedPoint, tr("Cuboid"), object);
    });
    addMenu->addAction(tr("Cylinder"), this, [this, object, pickedPoint]() {
        auto cylinder = new ExamplesBase::InteractiveCylinder;
        addToContext(cylinder, pickedPoint, tr("Cylinder"), object);
    });
    addMenu->addSeparator();
    addMenu->addAction(tr("Custom shape..."), this, [this, object, pickedPoint]() {
        auto path = ExamplesBase::InteractiveObjectItemModelShape::requestFilename(this);
        if (!path.isEmpty()) {
            auto shape = new ExamplesBase::InteractiveShape;
            shape->setModelPath(path);
            addToContext(shape, pickedPoint, tr("Shape"), object);
        }
    });

    if (object) {
        menu.addAction(tr("Remove"), this, [this, object]() {
            removeFromContext(object);
        });
        menu.addSeparator();
        if (manipulatorAttachedObject() != object) {
            menu.addAction(tr("Transform"), this, [this, object]() {
                showManipulator(object);
            });
        }
        if (editorAttachedObject() != object) {
            menu.addAction(tr("Edit"), this, [this, object]() {
                showEditor(object);
            });
        }
        auto cylinder = Handle(ExamplesBase::InteractiveCylinder)::DownCast(object);
        Handle(ExamplesBase::InteractiveObject) cutted;
        auto ctx = context();
        if (ctx && cylinder) {
            AIS_ListOfInteractive list;
            ctx->ObjectsInside(list, AIS_KOI_Shape);
            auto cylTrsf = ctx->Location(cylinder).Transformation();
            auto cylShape = cylinder->Shape();
            cylShape.Location(cylTrsf);
            for (const auto &obj : list) {
                if (obj == cylinder) {
                    continue;
                }
                auto interactive = Handle(ExamplesBase::InteractiveObject)::DownCast(obj);
                if (!interactive) {
                    continue;
                }
                auto intShape = interactive->Shape();
                auto intTrsf = ctx->Location(interactive).Transformation();
                intShape.Location(intTrsf);
                TopoDS_Shape cutShape = BRepAlgoAPI_Cut(intShape, cylShape);
                if (cutShape.IsNull()) {
                    continue;
                }
                cutShape.Location(intTrsf.Inverted());
                menu.addSeparator();
                menu.addAction(tr("Cut"), this, [this, cylinder, interactive, cutShape, intTrsf]() {
                    removeFromContext(cylinder);
                    removeFromContext(interactive);
                    auto cutted = new ExamplesBase::InteractiveObject;
                    cutted->SetShape(cutShape);
                    addToContext(cutted, gp_XYZ(), tr("Cutted"), nullptr);
                    context()->SetLocation(cutted, intTrsf);
                    context()->Redisplay(cutted, Standard_True);
                });
                break;
            }
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
}
