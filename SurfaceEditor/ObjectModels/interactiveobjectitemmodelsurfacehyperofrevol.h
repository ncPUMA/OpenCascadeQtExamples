#ifndef INTERACTIVEOBJECTITEMMODELSURFACEHYPEROFREVOL_H
#define INTERACTIVEOBJECTITEMMODELSURFACEHYPEROFREVOL_H

#include "interactiveobjectitemmodelsurfacerevolution.h"

class InteractiveSurfaceHyperOfRevol;
class InteractiveObjectItemModelSurfaceHyperOfRevolPrivate;

class InteractiveObjectItemModelSurfaceHyperOfRevol : public InteractiveObjectItemModelSurfaceRevolution
{
public:
    InteractiveObjectItemModelSurfaceHyperOfRevol(const Handle(InteractiveSurfaceHyperOfRevol) &object, QObject *parent = nullptr);
    ~InteractiveObjectItemModelSurfaceHyperOfRevol();

protected:
    void updateRevolutionFields() final;
    bool setRevolutionFieldData(const QModelIndex &index, const QVariant &value) final;
    QWidget *revolutionFieldEditor(const QModelIndex &index) const final;

private:
    InteractiveObjectItemModelSurfaceHyperOfRevolPrivate *d;
};

#endif // INTERACTIVEOBJECTITEMMODELSURFACEHYPEROFREVOL_H
