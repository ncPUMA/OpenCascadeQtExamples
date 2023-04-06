#include "interactiveobjectitemmodelcuboid.h"

#include <QDoubleSpinBox>

#include "../Objects/interactivecuboid.h"

namespace {
enum Fields {
    FieldWidth,
    FieldHeight,
    FieldDepth,
};
}

class InteractiveObjectItemModelCuboidPrivate
{
    friend class InteractiveObjectItemModelCuboid;

    Handle(InteractiveCuboid) object;
    std::map<Fields, QStandardItem *> fields;
};

InteractiveObjectItemModelCuboid::InteractiveObjectItemModelCuboid(const Handle(InteractiveCuboid) &object, QObject *parent)
    : InteractiveObjectItemModel(object, parent)
    , d(new InteractiveObjectItemModelCuboidPrivate)
{
    d->object = object;

    auto cuboid = createGroup(tr("Cuboid"));
    cuboid->appendRow(createRow(tr("Width"), d->fields, FieldWidth));
    cuboid->appendRow(createRow(tr("Height"), d->fields, FieldHeight));
    cuboid->appendRow(createRow(tr("Depth"), d->fields, FieldDepth));
    appendRow(cuboid);
}

InteractiveObjectItemModelCuboid::~InteractiveObjectItemModelCuboid()
{
    delete d;
}

void InteractiveObjectItemModelCuboid::updateFields()
{
    Standard_Real width = d->object->getWidth();
    d->fields[FieldWidth]->setText(QString("%1").arg(width, 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldWidth]->setData(width);
    Standard_Real height = d->object->getHeight();
    d->fields[FieldHeight]->setText(QString("%1").arg(height, 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldHeight]->setData(height);
    Standard_Real depth = d->object->getDepth();
    d->fields[FieldDepth]->setText(QString("%1").arg(depth, 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldDepth]->setData(depth);
}

bool InteractiveObjectItemModelCuboid::setFieldData(const QModelIndex &index, const QVariant &value)
{
    bool ret = false;
    Fields field;
    if (findField(index, d->fields, field)) {
        bool ok = false;
        Standard_Real vl = value.toDouble(&ok);
        if (!ok) {
            return false;
        }

        if (field == FieldWidth) {
            d->object->setWidth(vl);
        } else if (field == FieldHeight) {
            d->object->setHeight(vl);
        } else {
            d->object->setDepth(vl);
        }

        return true;
    }
    return false;
}

QWidget *InteractiveObjectItemModelCuboid::fieldEditor(const QModelIndex &index) const
{
    QWidget *ret = nullptr;
    Fields field;
    if (findField(index, d->fields, field)) {
        auto spinBox = new QDoubleSpinBox;
        spinBox->setRange(1., 1000.);
        spinBox->setDecimals(3);
        return spinBox;
    }
    return nullptr;
}
