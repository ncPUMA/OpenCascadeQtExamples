/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactiveobjectitemmodelsurfaceparabofrevol.h"

#include <QDoubleSpinBox>

#include "../Objects/interactivesurfaceparabofrevol.h"

namespace ExamplesBase {

namespace {
enum Fields {
    FieldFocal,
};
}

class InteractiveObjectItemModelSurfaceParabOfRevolPrivate
{
    friend class InteractiveObjectItemModelSurfaceParabOfRevol;

    Handle(InteractiveSurfaceParabOfRevol) object;
    std::map<Fields, QStandardItem *> fields;
};

InteractiveObjectItemModelSurfaceParabOfRevol::InteractiveObjectItemModelSurfaceParabOfRevol(const Handle(InteractiveSurfaceParabOfRevol) &object, QObject *parent)
    : InteractiveObjectItemModelSurfaceRevolution(object, parent)
    , d(new InteractiveObjectItemModelSurfaceParabOfRevolPrivate)
{
    d->object = object;

    appendRow(createRow(tr("Focal"), d->fields, FieldFocal));
}

InteractiveObjectItemModelSurfaceParabOfRevol::~InteractiveObjectItemModelSurfaceParabOfRevol()
{
    delete d;
}

void InteractiveObjectItemModelSurfaceParabOfRevol::updateRevolutionFields()
{
    Standard_Real focal = d->object->getFocal();
    d->fields[FieldFocal]->setText(QString("%1").arg(focal, 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldFocal]->setData(focal);
}

bool InteractiveObjectItemModelSurfaceParabOfRevol::setRevolutionFieldData(const QModelIndex &index, const QVariant &value)
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
            case FieldFocal:
                d->object->setFocal(vl);
                break;
        }

        return true;
    }
    return false;
}

QWidget *InteractiveObjectItemModelSurfaceParabOfRevol::revolutionFieldEditor(const QModelIndex &index) const
{
    QWidget *ret = nullptr;
    Fields field;
    if (findField(index, d->fields, field)) {
        auto spinBox = new QDoubleSpinBox;
        spinBox->setDecimals(3);
        switch (field) {
            case FieldFocal:
                spinBox->setRange(0., 1000.);
                break;
        }
        return spinBox;
    }
    return nullptr;
}

}
