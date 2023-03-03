#include "mainwindow.h"

#include <QDockWidget>
#include <QLabel>
#include <QTreeView>

#include "viewport.h"

class SurfacePropertyView : public QTreeView
{
public:
    SurfacePropertyView(QWidget *parent = nullptr)
        : QTreeView(parent)
        , mPlaceholder(new QLabel(tr("Choose a surface"), this)) {
        mPlaceholder->setAlignment(Qt::AlignCenter);
        mPlaceholder->resize(viewport()->size());
        setAlternatingRowColors(true);
    }

    void setModel(QAbstractItemModel *model) final {
        mPlaceholder->setVisible(model == nullptr);
        QTreeView::setModel(model);
        expandAll();
        resizeColumnToContents(0);
    }

protected:
    void resizeEvent(QResizeEvent *event) final {
        QTreeView::resizeEvent(event);
        mPlaceholder->resize(viewport()->size());
    }

private:
    QLabel *mPlaceholder = nullptr;
};

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

    QDockWidget *dock = new QDockWidget(tr("Surface property"), this);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    auto propertyView = new SurfacePropertyView(dock);
    dock->setWidget(propertyView);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    d_ptr->mViewport->setPropertyView(propertyView);
}

MainWindow::~MainWindow()
{
    delete d_ptr;
}
