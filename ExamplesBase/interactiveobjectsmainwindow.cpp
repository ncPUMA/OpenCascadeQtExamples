#include "interactiveobjectsmainwindow.h"

#include <QDockWidget>
#include <QLabel>
#include <QTreeView>

#include "Viewport/interactiveobjectsviewport.h"

namespace ExamplesBase {

class PlaceholderView : public QTreeView
{
public:
    PlaceholderView(const QString &placeholderText, QWidget *parent = nullptr)
        : QTreeView(parent)
        , mPlaceholder(new QLabel(placeholderText, this)) {
        mPlaceholder->setAlignment(Qt::AlignCenter);
        mPlaceholder->resize(viewport()->size());
        mPlaceholder->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(mPlaceholder, &QWidget::customContextMenuRequested,
                this, &QWidget::customContextMenuRequested);
        setAlternatingRowColors(true);
    }

    void setModel(QAbstractItemModel *model) override {
        setPlaceholderVisible(model == nullptr);
        QTreeView::setModel(model);
        expandAll();
        resizeColumnToContents(0);
    }

protected:
    void resizeEvent(QResizeEvent *event) final {
        QTreeView::resizeEvent(event);
        mPlaceholder->resize(viewport()->size());
    }

    void setPlaceholderVisible(bool visible) {
        mPlaceholder->setVisible(visible);
    }

private:
    QLabel *mPlaceholder = nullptr;
};

class ObjectsTree : public PlaceholderView
{
public:
    ObjectsTree(const QString &placeholderText, QWidget *parent = nullptr)
        : PlaceholderView(placeholderText, parent) {
        setContextMenuPolicy(Qt::CustomContextMenu);
    }

    void setModel(QAbstractItemModel *model) override {
        PlaceholderView::setModel(model);
        setPlaceholderVisible(model->rowCount() == 0);
        connect(model, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex &index){
            expand(index);
            setPlaceholderVisible(this->model()->rowCount() == 0);
        });
        connect(model, &QAbstractItemModel::rowsRemoved, this, [this](){
            setPlaceholderVisible(this->model()->rowCount() == 0);
        });
        connect(model, &QAbstractItemModel::modelReset, this, [this](){
            setPlaceholderVisible(this->model()->rowCount() == 0);
            expandAll();
        });
    }
};

class InteractiveObjectsMainWindowPrivate
{
    friend class InteractiveObjectsMainWindow;

    InteractiveObjectsViewport *mViewport = nullptr;
    ObjectsTree *mObjectsTree;
    PlaceholderView *mPropertyView;
};

InteractiveObjectsMainWindow::InteractiveObjectsMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d_ptr(new InteractiveObjectsMainWindowPrivate)
{
    QDockWidget *dockObjects = new QDockWidget(tr("Interactive objects"), this);
    dockObjects->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    d_ptr->mObjectsTree = new ObjectsTree(tr("No objects"), dockObjects);
    dockObjects->setWidget(d_ptr->mObjectsTree);
    addDockWidget(Qt::RightDockWidgetArea, dockObjects);

    QDockWidget *dockProperty = new QDockWidget(tr("Object property"), this);
    dockProperty->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    d_ptr->mPropertyView = new PlaceholderView(tr("Choose object"), dockProperty);
    dockProperty->setWidget(d_ptr->mPropertyView);
    addDockWidget(Qt::RightDockWidgetArea, dockProperty);
}

InteractiveObjectsMainWindow::~InteractiveObjectsMainWindow()
{
    delete d_ptr;
}

void InteractiveObjectsMainWindow::setViewport(InteractiveObjectsViewport *viewport)
{
    viewport->setObjectsView(d_ptr->mObjectsTree);
    viewport->setPropertyView(d_ptr->mPropertyView);

    setCentralWidget(viewport);
    d_ptr->mViewport = viewport;
}

}
