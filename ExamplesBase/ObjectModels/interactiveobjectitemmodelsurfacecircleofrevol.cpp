#include "interactiveobjectitemmodelsurfacecircleofrevol.h"

#include <QDoubleSpinBox>

#include "../Objects/interactivesurfacecircleofrevol.h"

namespace ExamplesBase {

namespace {
enum Fields {
    FieldRadius,
};
}

class InteractiveObjectItemModelSurfaceCircleOfRevolPrivate
{
    friend class InteractiveObjectItemModelSurfaceCircleOfRevol;

    Handle(InteractiveSurfaceCircleOfRevol) object;
    std::map<Fields, QStandardItem *> fields;
};

InteractiveObjectItemModelSurfaceCircleOfRevol::InteractiveObjectItemModelSurfaceCircleOfRevol(const Handle(InteractiveSurfaceCircleOfRevol) &object, QObject *parent)
    : InteractiveObjectItemModelSurfaceRevolution(object, parent)
    , d(new InteractiveObjectItemModelSurfaceCircleOfRevolPrivate)
{
    d->object = object;

    appendRow(createRow(tr("Radius"), d->fields, FieldRadius));
}

InteractiveObjectItemModelSurfaceCircleOfRevol::~InteractiveObjectItemModelSurfaceCircleOfRevol()
{
    delete d;
}

void InteractiveObjectItemModelSurfaceCircleOfRevol::updateRevolutionFields()
{
    Standard_Real focal = d->object->getRadius();
    d->fields[FieldRadius]->setText(QString("%1").arg(focal, 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldRadius]->setData(focal);
}

bool InteractiveObjectItemModelSurfaceCircleOfRevol::setRevolutionFieldData(const QModelIndex &index, const QVariant &value)
{
    bool ret = false;
    Fields field;
    if (findField(index, d->fields, field)) {
        bool ok = false;
        Standard_Real vl = value.toDouble(&ok);
        if (!ok) {
            return false;
        }

        switch (field) {
            case FieldRadius:
                d->object->setRadius(vl);
                break;
        }

        return true;
    }
    return false;
}

QWidget *InteractiveObjectItemModelSurfaceCircleOfRevol::revolutionFieldEditor(const QModelIndex &index) const
{
    QWidget *ret = nullptr;
    Fields field;
    if (findField(index, d->fields, field)) {
        auto spinBox = new QDoubleSpinBox;
        spinBox->setDecimals(3);
        switch (field) {
            case FieldRadius:
                spinBox->setRange(0.001, 1000.);
                break;
        }
        return spinBox;
    }
    return nullptr;
}

}
