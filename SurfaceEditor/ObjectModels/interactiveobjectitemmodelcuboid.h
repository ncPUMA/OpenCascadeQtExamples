#ifndef INTERACTIVEOBJECTITEMMODELCUBOID_H
#define INTERACTIVEOBJECTITEMMODELCUBOID_H

#include "interactiveobjectitemmodel.h"

class InteractiveCuboid;
class InteractiveObjectItemModelCuboidPrivate;

class InteractiveObjectItemModelCuboid : public InteractiveObjectItemModel
{
public:
    InteractiveObjectItemModelCuboid(const Handle(InteractiveCuboid) &object, QObject *parent = nullptr);
    ~InteractiveObjectItemModelCuboid();

protected:
    void updateFields() final;
    bool setFieldData(const QModelIndex &index, const QVariant &value) final;
    QWidget *fieldEditor(const QModelIndex &index) const final;

private:
    InteractiveObjectItemModelCuboidPrivate *d;
};

#endif // INTERACTIVEOBJECTITEMMODELCUBOID_H
