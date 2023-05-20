#ifndef INTERACTIVEOBJECTITEMMODELSPHERE_H
#define INTERACTIVEOBJECTITEMMODELSPHERE_H

#include "interactiveobjectitemmodel.h"

namespace ExamplesBase {

class InteractiveSphere;
class InteractiveObjectItemModelSpherePrivate;

class InteractiveObjectItemModelSphere : public InteractiveObjectItemModel
{
public:
    InteractiveObjectItemModelSphere(const Handle(InteractiveSphere) &object, QObject *parent = nullptr);
    ~InteractiveObjectItemModelSphere();

protected:
    void updateFields() final;
    bool setFieldData(const QModelIndex &index, const QVariant &value) final;
    QWidget *fieldEditor(const QModelIndex &index) const final;

private:
    InteractiveObjectItemModelSpherePrivate *d;
};

}

#endif // INTERACTIVEOBJECTITEMMODELSPHERE_H
