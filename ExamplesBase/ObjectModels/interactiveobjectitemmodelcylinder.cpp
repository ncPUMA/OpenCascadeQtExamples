#include "interactiveobjectitemmodelcylinder.h"

#include <QDoubleSpinBox>

#include "../Objects/interactivecylinder.h"

namespace ExamplesBase {

namespace {
enum Fields {
    FieldRadius,
    FieldLenght,
};
}

class InteractiveObjectItemModelCylinderPrivate
{
    friend class InteractiveObjectItemModelCylinder;

    Handle(InteractiveCylinder) object;
    std::map<Fields, QStandardItem *> fields;
};

InteractiveObjectItemModelCylinder::InteractiveObjectItemModelCylinder(const Handle(InteractiveCylinder) &object, QObject *parent)
    : InteractiveObjectItemModel(object, parent)
    , d(new InteractiveObjectItemModelCylinderPrivate)
{
    d->object = object;

    auto cylinder = createGroup(tr("Cylinder"));
    cylinder->appendRow(createRow(tr("Radius"), d->fields, FieldRadius));
    cylinder->appendRow(createRow(tr("Lenght"), d->fields, FieldLenght));
    appendRow(cylinder);
}

InteractiveObjectItemModelCylinder::~InteractiveObjectItemModelCylinder()
{
    delete d;
}

void InteractiveObjectItemModelCylinder::updateFields()
{
    Standard_Real radius = d->object->getRadius();
    d->fields[FieldRadius]->setText(QString("%1").arg(radius, 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldRadius]->setData(radius);
    Standard_Real lenght = d->object->getLenght();
    d->fields[FieldLenght]->setText(QString("%1").arg(lenght, 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldLenght]->setData(lenght);
}

bool InteractiveObjectItemModelCylinder::setFieldData(const QModelIndex &index, const QVariant &value)
{
    bool ret = false;
    Fields field;
    if (findField(index, d->fields, field)) {
        bool ok = false;
        Standard_Real vl = value.toDouble(&ok);
        if (!ok) {
            return false;
        }

        if (field == FieldRadius) {
            d->object->setRadius(vl);
        } else {
            d->object->setLenght(vl);
        }

        return true;
    }
    return false;
}

QWidget *InteractiveObjectItemModelCylinder::fieldEditor(const QModelIndex &index) const
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

}
