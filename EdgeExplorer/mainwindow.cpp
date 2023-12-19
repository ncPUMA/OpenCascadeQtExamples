/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mainwindow.h"

#include <QMenuBar>

#include "viewport.h"

class MainWindowPrivate
{
    friend class MainWindow;

    Viewport *mViewport = nullptr;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d_ptr(new MainWindowPrivate)
{
    d_ptr->mViewport = new Viewport(this);
    setCentralWidget(d_ptr->mViewport);
    d_ptr->mViewport->fitInView();

    auto menuBar = new QMenuBar(this);
    auto fMenu = menuBar->addMenu(tr("File"));
    fMenu->addAction(tr("Normal (v1) test"), d_ptr->mViewport, &Viewport::slNormalV1Test);
    fMenu->addAction(tr("Exit"), this, &QMainWindow::close);
    setMenuBar(menuBar);
}

MainWindow::~MainWindow()
{
    delete d_ptr;
}
