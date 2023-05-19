#ifndef INTERACTIVEOBJECTITEMMODELSURFACE_H
#define INTERACTIVEOBJECTITEMMODELSURFACE_H

#include "interactiveobjectitemmodel.h"

namespace ExamplesBase {

class InteractiveSurface;
class InteractiveObjectItemModelSurfacePrivate;

class InteractiveObjectItemModelSurface : public InteractiveObjectItemModel
{
public:
    InteractiveObjectItemModelSurface(const Handle(InteractiveSurface) &object, QObject *parent = nullptr);
    ~InteractiveObjectItemModelSurface();

protected:
    void updateFields() final;
    bool setFieldData(const QModelIndex &index, const QVariant &value) final;
    QWidget *fieldEditor(const QModelIndex &index) const final;

    virtual void updateSurfaceFields() = 0;
    virtual bool setSurfaceFieldData(const QModelIndex &index, const QVariant &value) = 0;
    virtual QWidget *surfaceFieldEditor(const QModelIndex &index) const = 0;

private:
    InteractiveObjectItemModelSurfacePrivate *d;
};

}

#endif // INTERACTIVEOBJECTITEMMODELSURFACE_H
