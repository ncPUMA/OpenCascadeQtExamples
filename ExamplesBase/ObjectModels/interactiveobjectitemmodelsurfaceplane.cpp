/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactiveobjectitemmodelsurfaceplane.h"

#include <QDoubleSpinBox>

#include "../Objects/interactivesurfaceplane.h"

namespace ExamplesBase {

namespace {
enum Fields {
    FieldUmax,
    FieldVmax,
};
}

class InteractiveObjectItemModelSurfacePlanePrivate
{
    friend class InteractiveObjectItemModelSurfacePlane;

    Handle(InteractiveSurfacePlane) object;
    std::map<Fields, QStandardItem *> fields;
};

InteractiveObjectItemModelSurfacePlane::InteractiveObjectItemModelSurfacePlane(const Handle(InteractiveSurfacePlane) &object, QObject *parent)
    : InteractiveObjectItemModelSurface(object, parent)
    , d(new InteractiveObjectItemModelSurfacePlanePrivate)
{
    d->object = object;

    auto plane = createGroup(tr("Plane"));
    plane->appendRow(createRow(tr("U max"), d->fields, FieldUmax));
    plane->appendRow(createRow(tr("V max"), d->fields, FieldVmax));
    appendRow(plane);
}

InteractiveObjectItemModelSurfacePlane::~InteractiveObjectItemModelSurfacePlane()
{
    delete d;
}

void InteractiveObjectItemModelSurfacePlane::updateSurfaceFields()
{
    Standard_Real Umax = d->object->getUmax();
    d->fields[FieldUmax]->setText(QString("%1").arg(Umax, 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldUmax]->setData(Umax);
    Standard_Real Vmax = d->object->getVmax();
    d->fields[FieldVmax]->setText(QString("%1").arg(Vmax, 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldVmax]->setData(Vmax);
}

bool InteractiveObjectItemModelSurfacePlane::setSurfaceFieldData(const QModelIndex &index, const QVariant &value)
{
    bool ret = false;
    Fields field;
    if (findField(index, d->fields, field)) {
        bool ok = false;
        Standard_Real vl = value.toDouble(&ok);
        if (!ok) {
            return false;
        }

        if (field == FieldUmax) {
            d->object->setUmax(vl);
        } else {
            d->object->setVmax(vl);
        }

        return true;
    }
    return false;
}

QWidget *InteractiveObjectItemModelSurfacePlane::surfaceFieldEditor(const QModelIndex &index) const
{
    QWidget *ret = nullptr;
    Fields field;
    if (findField(index, d->fields, field)) {
        auto spinBox = new QDoubleSpinBox;
        spinBox->setRange(-1000., 1000.);
        spinBox->setDecimals(3);
        return spinBox;
    }
    return nullptr;
}

}
