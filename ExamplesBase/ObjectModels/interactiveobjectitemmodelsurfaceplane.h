/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERACTIVEOBJECTITEMMODELSURFACEPLANE_H
#define INTERACTIVEOBJECTITEMMODELSURFACEPLANE_H

#include "interactiveobjectitemmodelsurface.h"

namespace ExamplesBase {

class InteractiveSurfacePlane;
class InteractiveObjectItemModelSurfacePlanePrivate;

class InteractiveObjectItemModelSurfacePlane : public InteractiveObjectItemModelSurface
{
public:
    InteractiveObjectItemModelSurfacePlane(const Handle(InteractiveSurfacePlane) &object, QObject *parent = nullptr);
    ~InteractiveObjectItemModelSurfacePlane();

protected:
    void updateSurfaceFields();
    bool setSurfaceFieldData(const QModelIndex &index, const QVariant &value);
    QWidget *surfaceFieldEditor(const QModelIndex &index) const;

private:
    InteractiveObjectItemModelSurfacePlanePrivate *d;
};

}

#endif // INTERACTIVEOBJECTITEMMODELSURFACEPLANE_H
