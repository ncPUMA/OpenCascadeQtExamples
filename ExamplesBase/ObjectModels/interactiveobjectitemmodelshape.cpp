/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactiveobjectitemmodelshape.h"

#include <QAction>
#include <QFileDialog>
#include <QLineEdit>

#include "../ModelLoader/modelloaderfactorymethod.h"

#include "../Objects/interactiveshape.h"

namespace ExamplesBase {

class InteractiveObjectItemModelShapePrivate
{
    friend class InteractiveObjectItemModelShape;

    Handle(InteractiveShape) shape;
    QStandardItem *shapeItem = nullptr;
    QBrush foreground;
};

InteractiveObjectItemModelShape::InteractiveObjectItemModelShape(const Handle(InteractiveShape) &object, QObject *parent)
    : InteractiveObjectItemModel(object, parent)
    , d(new InteractiveObjectItemModelShapePrivate)
{
    d->shape = object;

    QList<QStandardItem *> items = { new QStandardItem(tr("Shape path")), new QStandardItem };
    items[0]->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    d->shapeItem = items[1];
    d->foreground = d->shapeItem->foreground();
    appendRow(items);
}

InteractiveObjectItemModelShape::~InteractiveObjectItemModelShape()
{
    delete d;
}

QString InteractiveObjectItemModelShape::requestFilename(QWidget *parent, QString path)
{
    ExamplesBase::ModelLoaderFactoryMethod factory;
    return QFileDialog::getOpenFileName(parent,
                                        tr("Choose model file"),
                                        QString(),
                                        factory.supportedFilters());
}

void InteractiveObjectItemModelShape::updateFields()
{
    d->shapeItem->setData(d->shape->modelPath());
    d->shapeItem->setText(d->shape->modelPath());
    d->shapeItem->setForeground(d->shape->isValid() ? d->foreground : QBrush(Qt::red));
}

bool InteractiveObjectItemModelShape::setFieldData(const QModelIndex &index, const QVariant &value)
{
    auto item = itemFromIndex(index);
    if (item == d->shapeItem) {
        d->shape->setModelPath(value.toString());
        return true;
    }
    return false;
}

QWidget *InteractiveObjectItemModelShape::fieldEditor(const QModelIndex &index) const
{
    auto item = itemFromIndex(index);
    if (item == d->shapeItem) {
        auto le = new QLineEdit;
        auto action = le->addAction(QIcon::fromTheme(QStringLiteral("document-open")), QLineEdit::TrailingPosition);
        connect(this, &InteractiveObjectItemModelShape::newPath, le, &QLineEdit::setText);
        connect(action, &QAction::triggered, this, [this, le]() {
            auto path = requestFilename(le, d->shape->modelPath());
            if (!path.isEmpty()) {
                emit newPath(path);
            }
        });
        return le;
    }
    return nullptr;
}

}
