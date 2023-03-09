#ifndef INTERACTIVEOBJECTITEMMODELSURFACEREVOLUTION_H
#define INTERACTIVEOBJECTITEMMODELSURFACEREVOLUTION_H

#include "interactiveobjectitemmodelsurface.h"

class InteractiveSurfaceRevolution;
class InteractiveObjectItemModelSurfaceRevolutionPrivate;

class InteractiveObjectItemModelSurfaceRevolution : public InteractiveObjectItemModelSurface
{
public:
    InteractiveObjectItemModelSurfaceRevolution(const Handle(InteractiveSurfaceRevolution) &object, QObject *parent = nullptr);
    ~InteractiveObjectItemModelSurfaceRevolution();

protected:
    void updateSurfaceFields() final;
    bool setSurfaceFieldData(const QModelIndex &index, const QVariant &value) final;
    QWidget *surfaceFieldEditor(const QModelIndex &index) const final;

    virtual void updateRevolutionFields() = 0;
    virtual bool setRevolutionFieldData(const QModelIndex &index, const QVariant &value) = 0;
    virtual QWidget *revolutionFieldEditor(const QModelIndex &index) const = 0;

private:
    InteractiveObjectItemModelSurfaceRevolutionPrivate *d;
};

#endif // INTERACTIVEOBJECTITEMMODELSURFACEREVOLUTION_H
