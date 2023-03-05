#include "mainwindow.h"

#include <QDockWidget>
#include <QLabel>
#include <QTreeView>

#include "viewport.h"

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
    auto objectsView = new ObjectsTree(tr("No objects"), dockObjects);
    dockObjects->setWidget(objectsView);
    addDockWidget(Qt::RightDockWidgetArea, dockObjects);
    d_ptr->mViewport->setObjectsView(objectsView);

    QDockWidget *dockProperty = new QDockWidget(tr("Surface property"), this);
    dockProperty->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    auto propertyView = new PlaceholderView(tr("Choose object"), dockProperty);
    dockProperty->setWidget(propertyView);
    addDockWidget(Qt::RightDockWidgetArea, dockProperty);
    d_ptr->mViewport->setPropertyView(propertyView);
}

MainWindow::~MainWindow()
{
    delete d_ptr;
}
