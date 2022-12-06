#include "mainwindow.h"

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
}

MainWindow::~MainWindow()
{
    delete d_ptr;
}
