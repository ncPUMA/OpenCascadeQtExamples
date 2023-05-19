#include "interactiveobjectitemmodelsurfaceellipseofrevol.h"

#include <QDoubleSpinBox>

#include "../Objects/interactivesurfaceellipseofrevol.h"

namespace ExamplesBase {

namespace {
enum Fields {
    FieldFocal,
    FieldEccentricity,
};
}

class InteractiveObjectItemModelSurfaceEllipseOfRevolPrivate
{
    friend class InteractiveObjectItemModelSurfaceEllipseOfRevol;

    Handle(InteractiveSurfaceEllipseOfRevol) object;
    std::map<Fields, QStandardItem *> fields;
};

InteractiveObjectItemModelSurfaceEllipseOfRevol::InteractiveObjectItemModelSurfaceEllipseOfRevol(const Handle(InteractiveSurfaceEllipseOfRevol) &object, QObject *parent)
    : InteractiveObjectItemModelSurfaceRevolution(object, parent)
    , d(new InteractiveObjectItemModelSurfaceEllipseOfRevolPrivate)
{
    d->object = object;

    appendRow(createRow(tr("Focal"), d->fields, FieldFocal));
    appendRow(createRow(tr("Eccentricity"), d->fields, FieldEccentricity));
}

InteractiveObjectItemModelSurfaceEllipseOfRevol::~InteractiveObjectItemModelSurfaceEllipseOfRevol()
{
    delete d;
}

void InteractiveObjectItemModelSurfaceEllipseOfRevol::updateRevolutionFields()
{
    Standard_Real focal = d->object->getFocal();
    d->fields[FieldFocal]->setText(QString("%1").arg(focal, 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldFocal]->setData(focal);
    Standard_Real eccentricity = d->object->getEccentricity();
    d->fields[FieldEccentricity]->setText(QString("%1").arg(eccentricity, 7, 'f', 3, QLatin1Char('0')));
    d->fields[FieldEccentricity]->setData(eccentricity);
}

bool InteractiveObjectItemModelSurfaceEllipseOfRevol::setRevolutionFieldData(const QModelIndex &index, const QVariant &value)
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
            case FieldFocal:
                d->object->setFocal(vl);
                break;
            case FieldEccentricity:
                d->object->setEccentricity(vl);
                break;
        }

        return true;
    }
    return false;
}

QWidget *InteractiveObjectItemModelSurfaceEllipseOfRevol::revolutionFieldEditor(const QModelIndex &index) const
{
    QWidget *ret = nullptr;
    Fields field;
    if (findField(index, d->fields, field)) {
        auto spinBox = new QDoubleSpinBox;
        spinBox->setDecimals(3);
        switch (field) {
            case FieldFocal:
                spinBox->setRange(0.001, 1000.);
                break;
            case FieldEccentricity:
                spinBox->setRange(1.001, 1000.);
                break;
        }
        return spinBox;
    }
    return nullptr;
}

}
