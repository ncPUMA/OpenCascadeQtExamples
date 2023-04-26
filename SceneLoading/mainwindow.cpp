#include "mainwindow.h"

#include <QDockWidget>
#include <QTreeView>

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

    QDockWidget *dockObjects = new QDockWidget(tr("Interactive objects"), this);
    dockObjects->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    auto objectsView = new QTreeView(dockObjects);
    dockObjects->setWidget(objectsView);
    addDockWidget(Qt::RightDockWidgetArea, dockObjects);
    d_ptr->mViewport->setObjectsView(objectsView);
}

MainWindow::~MainWindow()
{
    delete d_ptr;
}
