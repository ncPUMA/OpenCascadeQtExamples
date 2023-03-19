#ifndef INTERACTIVEOBJECTITEMMODELSURFACEELLIPSEOFREVOL_H
#define INTERACTIVEOBJECTITEMMODELSURFACEELLIPSEOFREVOL_H

#include "interactiveobjectitemmodelsurfacerevolution.h"

class InteractiveSurfaceEllipseOfRevol;
class InteractiveObjectItemModelSurfaceEllipseOfRevolPrivate;

class InteractiveObjectItemModelSurfaceEllipseOfRevol : public InteractiveObjectItemModelSurfaceRevolution
{
public:
    InteractiveObjectItemModelSurfaceEllipseOfRevol(const Handle(InteractiveSurfaceEllipseOfRevol) &object, QObject *parent = nullptr);
    ~InteractiveObjectItemModelSurfaceEllipseOfRevol();

protected:
    void updateRevolutionFields() final;
    bool setRevolutionFieldData(const QModelIndex &index, const QVariant &value) final;
    QWidget *revolutionFieldEditor(const QModelIndex &index) const final;

private:
    InteractiveObjectItemModelSurfaceEllipseOfRevolPrivate *d;
};

#endif // INTERACTIVEOBJECTITEMMODELSURFACEELLIPSEOFREVOL_H
