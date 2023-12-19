/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERACTIVEOBJECTITEMMODELSURFACEPARABOFREVOL_H
#define INTERACTIVEOBJECTITEMMODELSURFACEPARABOFREVOL_H

#include "interactiveobjectitemmodelsurfacerevolution.h"

namespace ExamplesBase {

class InteractiveSurfaceParabOfRevol;
class InteractiveObjectItemModelSurfaceParabOfRevolPrivate;

class InteractiveObjectItemModelSurfaceParabOfRevol : public InteractiveObjectItemModelSurfaceRevolution
{
public:
    InteractiveObjectItemModelSurfaceParabOfRevol(const Handle(InteractiveSurfaceParabOfRevol) &object, QObject *parent = nullptr);
    ~InteractiveObjectItemModelSurfaceParabOfRevol();

protected:
    void updateRevolutionFields() final;
    bool setRevolutionFieldData(const QModelIndex &index, const QVariant &value) final;
    QWidget *revolutionFieldEditor(const QModelIndex &index) const final;

private:
    InteractiveObjectItemModelSurfaceParabOfRevolPrivate *d;
};

}

#endif // INTERACTIVEOBJECTITEMMODELSURFACEPARABOFREVOL_H
