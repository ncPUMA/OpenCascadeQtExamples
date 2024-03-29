/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QApplication>

#include <ExamplesBase/interactiveobjectsmainwindow.h>

#include "viewport.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ExamplesBase::InteractiveObjectsMainWindow w;
    w.setViewport(new Viewport);
    w.setGeometry(600, 360, 1200, 720);
    w.show();
    return a.exec();
}
