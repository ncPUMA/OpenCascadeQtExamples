/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERACTIVEOBJECTITEMMODELSHAPE_H
#define INTERACTIVEOBJECTITEMMODELSHAPE_H

#include "interactiveobjectitemmodel.h"

#include <QWidget>

namespace ExamplesBase {

class InteractiveShape;
class InteractiveObjectItemModelShapePrivate;

class InteractiveObjectItemModelShape : public InteractiveObjectItemModel
{
    Q_OBJECT

public:
    InteractiveObjectItemModelShape(const Handle(InteractiveShape) &object, QObject *parent = nullptr);
    ~InteractiveObjectItemModelShape();

    static QString requestFilename(QWidget *parent, QString path = QString());

Q_SIGNALS:
    void newPath(const QString &path) const;

protected:
    void updateFields() final;
    bool setFieldData(const QModelIndex &index, const QVariant &value) final;
    QWidget *fieldEditor(const QModelIndex &index) const final;

private:
    InteractiveObjectItemModelShapePrivate *d;
};

}

#endif // INTERACTIVEOBJECTITEMMODELSHAPE_H
