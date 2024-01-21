/*
    SPDX-FileCopyrightText: 2024 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <ExamplesBase/Viewport/interactiveobjectsviewport.h>

class ViewportPrivate;

class Viewport : public ExamplesBase::InteractiveObjectsViewport
{
public:
    Viewport();
    ~Viewport();

protected:
    void objectsViewMenuRequest(const Handle(AIS_InteractiveObject) &obj, QMenu &menu) final;

    void contextMenuRequest(const Handle(AIS_InteractiveObject) &object,
                            const gp_XYZ &pickedPoint,
                            QMenu &menu) final;

private:
    void createContextMenu(const Handle(AIS_InteractiveObject) &object,
                           const gp_XYZ &pickedPoint,
                           QMenu &menu);

private:
    ViewportPrivate *const d_ptr;
};

#endif // VIEWPORT_H
