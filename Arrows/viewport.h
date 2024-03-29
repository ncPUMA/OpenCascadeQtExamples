/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <ExamplesBase/Viewport/viewport.h>

class QTreeView;
class ViewportPrivate;

class Viewport : public ExamplesBase::Viewport
{
public:
    Viewport(QWidget *parent);
    ~Viewport();

    void setObjectsView(QTreeView *objectsView);

protected:
    void contextMenuRequest(const Handle(AIS_InteractiveObject) &object,
                            const gp_XYZ &pickedPoint,
                            QMenu &menu) final;

private:
    ViewportPrivate *const d_ptr;
};

#endif // VIEWPORT_H
