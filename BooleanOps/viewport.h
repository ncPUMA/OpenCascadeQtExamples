/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

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
    bool mouseReleased(QMouseEvent *event) final;

private:
    void menuRequest(const Handle(AIS_Shape) &object,
                     const gp_XYZ &pickedPoint,
                     QMenu &menu);

private:
    ViewportPrivate *const d_ptr;
};

#endif // VIEWPORT_H
