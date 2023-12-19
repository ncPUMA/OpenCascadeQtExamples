/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interactiveobjectitemmodel.h"

#include <QDoubleSpinBox>
#include <QLineEdit>

#include <gp_Quaternion.hxx>

#include "../Objects/interactiveobject.h"

namespace ExamplesBase {

namespace {
enum Fields
{
    FieldName,
    FieldParent,
    FieldTranslationX,
    FieldTranslationY,
    FieldTranslationZ,
    FieldRotationX,
    FieldRotationY,
    FieldRotationZ,
};
}

class InteractiveObjectItemModelPrivate
{
    friend class InteractiveObjectItemModel;

    gp_XYZ translation() const {
        return object->getContextTransform().TranslationPart();
    }

    gp_XYZ rotation() const {
        gp_XYZ ret;
        gp_Quaternion quat;
        quat = object->getContextTransform().GetRotation();
        Standard_Real X, Y, Z;
        quat.GetEulerAngles(gp_Extrinsic_XYZ, X, Y, Z);
        ret.SetCoord(X, Y, Z);
        return ret * 180. / M_PI;
    }

    bool setFieldValue(Fields field, const QVariant &value) {
        if (field == FieldName) {
            object->setName(value.toString());
            return true;
        }

        if (field == FieldParent) {
            return false;
        }

        bool ok = false;
        Standard_Real vl = value.toDouble(&ok);
        if (!ok) {
            return false;
        }

        auto ctx = object->GetContext();
        if (!ctx) {
            return false;
        }

        gp_Trsf trsf = object->getContextTransform();
        if (FieldTranslationX <= field && field <= FieldTranslationZ) {
            auto translation = trsf.TranslationPart();
            translation.SetCoord(field - FieldTranslationX + 1, vl);
            trsf.SetTranslationPart(translation);
        } else if (FieldRotationX <= field && field <= FieldRotationZ) {
            gp_Quaternion quat = trsf.GetRotation();
            Standard_Real Coord[3];
            quat.GetEulerAngles(gp_Extrinsic_XYZ, Coord[0], Coord[1], Coord[2]);
            Coord[field - FieldRotationX] = vl * M_PI / 180.;
            quat.SetEulerAngles(gp_Extrinsic_XYZ, Coord[0], Coord[1], Coord[2]);
            trsf.SetRotationPart(quat);
        } else {
            return false;
        }
        object->setContextTransform(trsf);
        return true;
    }

    Handle(InteractiveObject) object;
    std::map<Fields, QStandardItem *> fields;
};

InteractiveObjectItemModel::InteractiveObjectItemModel(const Handle(InteractiveObject) &object, QObject *parent)
    : QStandardItemModel(parent)
    , d(new InteractiveObjectItemModelPrivate)
{
    d->object = object;

    setHorizontalHeaderLabels({ tr("Field"), tr("Value") });
    appendRow(createRow(tr("Object"), d->fields, FieldName));
    appendRow(createRow(tr("Parent"), d->fields, FieldParent, false));

    auto translation = createGroup(tr("Translation"));
    translation->appendRow(createRow(tr("Translation X"), d->fields, FieldTranslationX));
    translation->appendRow(createRow(tr("Translation Y"), d->fields, FieldTranslationY));
    translation->appendRow(createRow(tr("Translation Z"), d->fields, FieldTranslationZ));
    appendRow(translation);

    auto rotation = createGroup(tr("Rotation"));
    rotation->appendRow(createRow(tr("Rotation X"), d->fields, FieldRotationX));
    rotation->appendRow(createRow(tr("Rotation Y"), d->fields, FieldRotationY));
    rotation->appendRow(createRow(tr("Rotation Z"), d->fields, FieldRotationZ));
    appendRow(rotation);
}

InteractiveObjectItemModel::~InteractiveObjectItemModel()
{
    delete d;
}

void InteractiveObjectItemModel::update()
{
    d->fields[FieldName]->setText(d->object->name());
    d->fields[FieldName]->setData(d->object->name());
    QString parentValue = tr("No parent");
    auto parent = d->object->Parent();
    if (parent) {
        auto castedParent = Handle(InteractiveObject)::DownCast(parent);
        if (castedParent) {
            parentValue = castedParent->name();
        }
    }
    d->fields[FieldParent]->setText(parentValue);
    auto translation = d->translation();
    d->fields[FieldTranslationX]->setText(QString("%1").arg(translation.X(), 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldTranslationX]->setData(translation.X());
    d->fields[FieldTranslationY]->setText(QString("%1").arg(translation.Y(), 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldTranslationY]->setData(translation.Y());
    d->fields[FieldTranslationZ]->setText(QString("%1").arg(translation.Z(), 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldTranslationZ]->setData(translation.Z());
    auto rotation = d->rotation();
    d->fields[FieldRotationX]->setText(QString("%1").arg(rotation.X(), 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldRotationX]->setData(rotation.X());
    d->fields[FieldRotationY]->setText(QString("%1").arg(rotation.Y(), 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldRotationY]->setData(rotation.Y());
    d->fields[FieldRotationZ]->setText(QString("%1").arg(rotation.Z(), 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldRotationZ]->setData(rotation.Z());

    updateFields();

    emit dataChanged(index(0, 1), index(rowCount() - 1, 1), { Qt::DisplayRole, Qt::UserRole + 1 });
}

bool InteractiveObjectItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole) {
        return false;
    }

    bool ret = false;
    Fields field;
    if (findField(index, d->fields, field)) {
        ret = d->setFieldValue(field, value);
    } else {
        ret = setFieldData(index, value);
    }

    if (ret) {
        update();
    }
    return ret;
}

void InteractiveObjectItemModel::updateFields()
{

}

bool InteractiveObjectItemModel::setFieldData(const QModelIndex &index, const QVariant &value)
{
    Q_UNUSED(index);
    Q_UNUSED(value);
    return false;
}

QWidget *InteractiveObjectItemModel::fieldEditor(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return nullptr;
}

QWidget *InteractiveObjectItemModel::createFieldEditor(const QModelIndex &index) const
{
    QWidget *ret = nullptr;
    Fields field;
    if (findField(index, d->fields, field)) {
        switch (field) {
            case FieldName:
                ret = new QLineEdit;
                break;
            case FieldParent:
                break;
            case FieldTranslationX:
            case FieldTranslationY:
            case FieldTranslationZ: {
                auto spinBox = new QDoubleSpinBox;
                spinBox->setRange(-1000., 1000.);
                spinBox->setDecimals(3);
                ret = spinBox;
                break;
            }
            case FieldRotationX:
            case FieldRotationY:
            case FieldRotationZ: {
                auto spinBox = new QDoubleSpinBox;
                spinBox->setRange(-359.999, 359.999);
                spinBox->setDecimals(3);
                ret = spinBox;
                break;
            }
        }
    } else {
        ret = fieldEditor(index);
    }
    return ret;
}

}
