/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "viewport.h"

#include <QDebug>
#include <QStandardItemModel>
#include <QTreeView>

#include <AIS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <Prs3d_ShadingAspect.hxx>

#include "xcafsteploader.h"

class ViewportPrivate
{
    friend class Viewport;

    void addToObjectsTree(const Handle(AIS_InteractiveObject) &obj,
                          const XCafStepLoader &loader,
                          Handle(AIS_InteractiveContext) &ctx,
                          QStandardItem *parent = nullptr) {
        auto name = QString::fromStdU16String(loader.name(obj).ToExtString());
        if (name.isEmpty()) {
            name = Viewport::tr("Shape %1").arg(++unnamedNo);
        }
        auto item = new QStandardItem(name);
        item->setData(QVariant::fromValue(static_cast<void *>(obj.get())));
        if (parent) {
            parent->appendRow(item);
        } else {
            objectsModel->appendRow(item);
        }
        auto chList = obj->Children();
        for (const auto &ch : chList) {
            auto child = Handle(AIS_InteractiveObject)::DownCast(ch);
            if (child) {
                addToObjectsTree(child, loader, ctx, item);
            }
        }
    }

    void load(const QString &fileName, Handle(AIS_InteractiveContext) &ctx) {
        objectsModel->clear();
        unnamedNo = 0;
        objectsModel->setHorizontalHeaderItem(0, new QStandardItem(Viewport::tr("Name")));
        XCafStepLoader loader;
        if (!loader.loadStep(fileName.toLocal8Bit().constData())) {
            qDebug() << "Error load step";
            return;
        }

        const auto objects = loader.result();
        for (const auto &obj : objects) {
            ctx->SetDisplayMode(obj, AIS_Shaded, Standard_False);
            ctx->Display(obj, Standard_True);

            if (objectsModel) {
                addToObjectsTree(obj, loader, ctx);
            }

            ctx->SetSelectionModeActive(obj,
                                        AIS_Shape::SelectionMode(TopAbs_SOLID),
                                        Standard_True,
                                        AIS_SelectionModesConcurrency_Single);
        }
        if (objectsView) {
            objectsView->expandAll();
        }
    }

    QTreeView *objectsView = nullptr;
    QStandardItemModel *objectsModel = nullptr;
    int unnamedNo = 0;
};

Viewport::Viewport(QWidget *parent)
    : ExamplesBase::Viewport(parent)
    , d_ptr(new ViewportPrivate)
{
    auto path = QStringLiteral("../Models/sample_telescope_system_full.step");
    QMetaObject::invokeMethod(this, [path, this]() {
        auto ctx = context();
        d_ptr->load(path, ctx);
    }, Qt::QueuedConnection);
}

Viewport::~Viewport()
{
    delete d_ptr;
}

void Viewport::setObjectsView(QTreeView *objectsView)
{
    d_ptr->objectsView = objectsView;
    delete d_ptr->objectsView->model();
    d_ptr->objectsModel = new QStandardItemModel(d_ptr->objectsView);
    d_ptr->objectsView->setModel(d_ptr->objectsModel);

    connect(d_ptr->objectsView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this](const QItemSelection &selected, const QItemSelection &) {
        context()->ClearSelected(Standard_True);
        auto indexes = selected.indexes();
        for (const auto &index : qAsConst(indexes)) {
            QVariant vl = index.data(Qt::UserRole + 1);
            if (vl.isNull()) {
                continue;
            }

            auto obj = vl.value<void *>();
            AIS_ListOfInteractive objects;
            context()->DisplayedObjects(objects);
            auto it = std::find_if(objects.cbegin(), objects.cend(), [obj](const Handle(AIS_InteractiveObject) &object) {
                return object.get() == obj;
            });
            if (it != objects.cend()) {
                context()->SetSelected(*it, Standard_True);
            }
        }
    });
}
