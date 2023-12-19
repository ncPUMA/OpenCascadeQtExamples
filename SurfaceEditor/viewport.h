/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <ExamplesBase/Viewport/interactiveobjectsviewport.h>

class Viewport : public ExamplesBase::InteractiveObjectsViewport
{
public:
    Viewport();

protected:
    void objectsViewMenuRequest(const Handle(AIS_InteractiveObject) &obj, QMenu &menu);
    bool mouseReleased(QMouseEvent *event) final;

private:
    void menuRequest(const Handle(ExamplesBase::InteractiveObject) &object,
                     const gp_XYZ &pickedPoint,
                     QMenu &menu);
};

#endif // VIEWPORT_H
