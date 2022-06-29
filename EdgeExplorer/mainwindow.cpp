#include "mainwindow.h"

#include "viewport.h"

class MainWindowPrivate
{
    friend class MainWindow;

    ViewPort *mViewport = nullptr;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d_ptr(new MainWindowPrivate)
{
    d_ptr->mViewport = new ViewPort(this);
    setCentralWidget(d_ptr->mViewport);
    d_ptr->mViewport->fitInView();
}

MainWindow::~MainWindow()
{
    delete d_ptr;
}
