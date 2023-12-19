/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactiveobjectitemmodelsphere.h"

#include <QDoubleSpinBox>

#include "../Objects/interactivesphere.h"

namespace ExamplesBase {

namespace {
enum Fields {
    FieldRadius,
};
}

class InteractiveObjectItemModelSpherePrivate
{
    friend class InteractiveObjectItemModelSphere;

    Handle(InteractiveSphere) object;
    std::map<Fields, QStandardItem *> fields;
};

InteractiveObjectItemModelSphere::InteractiveObjectItemModelSphere(const Handle(InteractiveSphere) &object, QObject *parent)
    : InteractiveObjectItemModel(object, parent)
    , d(new InteractiveObjectItemModelSpherePrivate)
{
    d->object = object;

    auto sphere = createGroup(tr("Sphere"));
    sphere->appendRow(createRow(tr("Radius"), d->fields, FieldRadius));
    appendRow(sphere);
}

InteractiveObjectItemModelSphere::~InteractiveObjectItemModelSphere()
{
    delete d;
}

void InteractiveObjectItemModelSphere::updateFields()
{
    Standard_Real radius = d->object->getRadius();
    d->fields[FieldRadius]->setText(QString("%1").arg(radius, 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldRadius]->setData(radius);
}

bool InteractiveObjectItemModelSphere::setFieldData(const QModelIndex &index, const QVariant &value)
{
    bool ret = false;
    Fields field;
    if (findField(index, d->fields, field)) {
        bool ok = false;
        Standard_Real vl = value.toDouble(&ok);
        if (!ok) {
            return false;
        }

        if (field == FieldRadius) {
            d->object->setRadius(vl);
        }

        return true;
    }
    return false;
}

QWidget *InteractiveObjectItemModelSphere::fieldEditor(const QModelIndex &index) const
{
    QWidget *ret = nullptr;
    Fields field;
    if (findField(index, d->fields, field)) {
        auto spinBox = new QDoubleSpinBox;
        spinBox->setRange(1., 1000.);
        spinBox->setDecimals(3);
        return spinBox;
    }
    return nullptr;
}

}
