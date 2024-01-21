/*
    SPDX-FileCopyrightText: 2024 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "viewport.h"

#include <QDebug>
#include <QMenu>

#include <AIS_InteractiveContext.hxx>

#include <ExamplesBase/ModelLoader/steploader.h>
#include <ExamplesBase/Objects/interactivetexturedplane.h>
#include <ExamplesBase/utility.h>
#include <gp_QuaternionSLerp.hxx>
#include <TopoDS_Face.hxx>

class ViewportPrivate
{
    friend class Viewport;

    Handle(AIS_Shape) model;
};

Viewport::Viewport()
    : ExamplesBase::InteractiveObjectsViewport()
    , d_ptr(new ViewportPrivate)
{
    context()->IsoOnPlane(Standard_False);
    context()->SetIsoNumber(1);

    QMetaObject::invokeMethod(this, [this]() {
        const char *path = "../Models/45deg AdjMirr Adapter Left Rev1.STEP";
        ExamplesBase::StepLoader loader;
        d_ptr->model = new AIS_Shape(loader.load(path));
        d_ptr->model->SetDisplayMode(AIS_Shaded);
        context()->Display(d_ptr->model, Standard_True);
        fitInView();
    }, Qt::QueuedConnection);
}

Viewport::~Viewport()
{
    delete d_ptr;
}

void Viewport::objectsViewMenuRequest(const Handle(AIS_InteractiveObject) &obj, QMenu &menu)
{
    createContextMenu(obj, gp_XYZ(), menu);
}

void Viewport::contextMenuRequest(const Handle(AIS_InteractiveObject) &object,
                                  const gp_XYZ &pickedPoint,
                                  QMenu &menu)
{
    createContextMenu(object, pickedPoint, menu);
}

void Viewport::createContextMenu(const Handle(AIS_InteractiveObject) &object,
                                 const gp_XYZ &pickedPoint,
                                 QMenu &menu)
{
    if (object && object == d_ptr->model) {
        menu.addAction(tr("Plane"), this, [this, object, pickedPoint]() {
            const auto shape = d_ptr->model->Shape();
            const auto face = ExamplesBase::findFaceByPoint(shape, pickedPoint);
            if (face.IsNull()) {
                return;
            }

            Handle(ExamplesBase::InteractiveTexturedPlane) plane = new ExamplesBase::InteractiveTexturedPlane;
            plane->setUmax(20.);
            plane->setVmax(10.);
            plane->setTextureFileName("../Textures/barcode.png");
            addToContext(plane, pickedPoint, tr("Texture"), object);

            gp_Dir D1U;
            const auto normal = ExamplesBase::getNormal(face, pickedPoint, &D1U);
            gp_Trsf trsf;
            auto xAxis = D1U.Crossed(normal);
            auto yAxis = normal.Crossed(xAxis);
            gp_Mat mat(xAxis.XYZ(), yAxis.XYZ(), normal.XYZ());
            trsf.SetRotationPart(gp_Quaternion(mat));
            gp_Pnt pos(pickedPoint);
            pos.Translate(gp_Vec(normal) * .1);
            pos.Translate(gp_Vec(yAxis) * plane->getVmax() * -1.);
            trsf.SetTranslationPart(pos.XYZ());
            context()->SetLocation(plane, trsf);
        });
        return;
    }

    auto interactive = Handle(ExamplesBase::InteractiveObject)::DownCast(object);
    if (interactive) {
        menu.addAction(tr("Remove"), this, [this, interactive]() {
            removeFromContext(interactive);
        });
        menu.addSeparator();
        if (manipulatorAttachedObject() != interactive) {
            menu.addAction(tr("Transform"), this, [this, interactive]() {
                showManipulator(interactive);
            });
        }
        if (editorAttachedObject() != interactive) {
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
}
