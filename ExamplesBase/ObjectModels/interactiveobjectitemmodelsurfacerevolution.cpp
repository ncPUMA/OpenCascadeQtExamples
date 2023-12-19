/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactiveobjectitemmodelsurfacerevolution.h"

#include <QDoubleSpinBox>

#include "../Objects/interactivesurfacerevolution.h"

namespace ExamplesBase {

namespace {
enum Fields {
    FieldDirX,
    FieldDirY,
    FieldDirZ,
    FieldUmax,
    FieldVmax,
};
}

class InteractiveObjectItemModelSurfaceRevolutionPrivate
{
    friend class InteractiveObjectItemModelSurfaceRevolution;

    Handle(InteractiveSurfaceRevolution) object;
    std::map<Fields, QStandardItem *> fields;
};

InteractiveObjectItemModelSurfaceRevolution::InteractiveObjectItemModelSurfaceRevolution(const Handle(InteractiveSurfaceRevolution) &object, QObject *parent)
    : InteractiveObjectItemModelSurface(object, parent)
    , d(new InteractiveObjectItemModelSurfaceRevolutionPrivate)
{
    d->object = object;

    auto revol = createGroup(tr("Revolution"));
    revol->appendRow(createRow(tr("Direction X"), d->fields, FieldDirX));
    revol->appendRow(createRow(tr("Direction Y"), d->fields, FieldDirY));
    revol->appendRow(createRow(tr("Direction Z"), d->fields, FieldDirZ));
    appendRow(revol);

    auto plane = createGroup(tr("Plane"));
    plane->appendRow(createRow(tr("U max"), d->fields, FieldUmax));
    plane->appendRow(createRow(tr("V max"), d->fields, FieldVmax));
    appendRow(plane);
}

InteractiveObjectItemModelSurfaceRevolution::~InteractiveObjectItemModelSurfaceRevolution()
{
    delete d;
}

void InteractiveObjectItemModelSurfaceRevolution::updateSurfaceFields()
{
    auto dir = d->object->getRevolutionDirection();
    d->fields[FieldDirX]->setText(QString("%1").arg(dir.X(), 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldDirX]->setData(dir.X());
    d->fields[FieldDirY]->setText(QString("%1").arg(dir.Y(), 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldDirY]->setData(dir.Y());
    d->fields[FieldDirZ]->setText(QString("%1").arg(dir.Z(), 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldDirZ]->setData(dir.Z());

    Standard_Real Umax = d->object->getUmax() * 180. / M_PI;
    d->fields[FieldUmax]->setText(QString("%1").arg(Umax, 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldUmax]->setData(Umax);
    Standard_Real Vmax = d->object->getVmax();
    d->fields[FieldVmax]->setText(QString("%1").arg(Vmax, 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldVmax]->setData(Vmax);

    updateRevolutionFields();
}

bool InteractiveObjectItemModelSurfaceRevolution::setSurfaceFieldData(const QModelIndex &index, const QVariant &value)
{
    bool ret = false;
    Fields field;
    if (findField(index, d->fields, field)) {
        bool ok = false;
        Standard_Real vl = value.toDouble(&ok);
        if (!ok) {
            return false;
        }

        switch (field) {
            case FieldDirX:
            case FieldDirY:
            case FieldDirZ: {
                auto dir = d->object->getRevolutionDirection().XYZ();
                dir.SetCoord(field - FieldDirX + 1, vl);
                if (!dir.IsEqual(gp_XYZ(), Precision::Confusion())) {
                    d->object->setRevolutionDirection(dir);
                }
                break;
            }
            case FieldUmax:
                d->object->setUmax(vl * M_PI / 180.);
                break;
            case FieldVmax:
                d->object->setVmax(vl);
                break;
        }

        return true;
    }
    return setRevolutionFieldData(index, value);
}

QWidget *InteractiveObjectItemModelSurfaceRevolution::surfaceFieldEditor(const QModelIndex &index) const
{
    QWidget *ret = nullptr;
    Fields field;
    if (findField(index, d->fields, field)) {
        auto spinBox = new QDoubleSpinBox;
        spinBox->setDecimals(3);
        switch (field) {
            case FieldDirX:
            case FieldDirY:
            case FieldDirZ:
                spinBox->setRange(-1., 1.);
                break;
            case FieldUmax:
                spinBox->setRange(0.001, 360.);
                break;
            case FieldVmax:
                spinBox->setRange(0.001, 1000.);
                break;
        }
        return spinBox;
    }
    return revolutionFieldEditor(index);
}

}
