#ifndef INTERACTIVEOBJECTITEMMODELSURFACECIRCLEOFREVOL_H
#define INTERACTIVEOBJECTITEMMODELSURFACECIRCLEOFREVOL_H

#include "interactiveobjectitemmodelsurfacerevolution.h"

namespace ExamplesBase {

class InteractiveSurfaceCircleOfRevol;
class InteractiveObjectItemModelSurfaceCircleOfRevolPrivate;

class InteractiveObjectItemModelSurfaceCircleOfRevol : public InteractiveObjectItemModelSurfaceRevolution
{
public:
    InteractiveObjectItemModelSurfaceCircleOfRevol(const Handle(InteractiveSurfaceCircleOfRevol) &object, QObject *parent = nullptr);
    ~InteractiveObjectItemModelSurfaceCircleOfRevol();

protected:
    void updateRevolutionFields() final;
    bool setRevolutionFieldData(const QModelIndex &index, const QVariant &value) final;
    QWidget *revolutionFieldEditor(const QModelIndex &index) const final;

private:
    InteractiveObjectItemModelSurfaceCircleOfRevolPrivate *d;
};

}

#endif // INTERACTIVEOBJECTITEMMODELSURFACECIRCLEOFREVOL_H
