/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERACTIVEOBJECTITEMMODELCYLINDER_H
#define INTERACTIVEOBJECTITEMMODELCYLINDER_H

#include "interactiveobjectitemmodel.h"

namespace ExamplesBase {

class InteractiveCylinder;
class InteractiveObjectItemModelCylinderPrivate;

class InteractiveObjectItemModelCylinder : public InteractiveObjectItemModel
{
public:
    InteractiveObjectItemModelCylinder(const Handle(InteractiveCylinder) &object, QObject *parent = nullptr);
    ~InteractiveObjectItemModelCylinder();

protected:
    void updateFields() final;
    bool setFieldData(const QModelIndex &index, const QVariant &value) final;
    QWidget *fieldEditor(const QModelIndex &index) const final;

private:
    InteractiveObjectItemModelCylinderPrivate *d;
};

}

#endif // INTERACTIVEOBJECTITEMMODELCYLINDER_H
