/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERACTIVEOBJECTSMAINWINDOW_H
#define INTERACTIVEOBJECTSMAINWINDOW_H

#include <QMainWindow>

namespace ExamplesBase {

class InteractiveObjectsViewport;
class InteractiveObjectsMainWindowPrivate;

class InteractiveObjectsMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    InteractiveObjectsMainWindow(QWidget *parent = nullptr);
    ~InteractiveObjectsMainWindow();

    void setViewport(InteractiveObjectsViewport *viewport);

private:
    InteractiveObjectsMainWindowPrivate *const d_ptr;
};

}

#endif // INTERACTIVEOBJECTSMAINWINDOW_H
