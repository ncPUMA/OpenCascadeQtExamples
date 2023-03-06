#ifndef INTERACTIVEOBJECTITEMMODELSURFACEPARABOFREVOL_H
#define INTERACTIVEOBJECTITEMMODELSURFACEPARABOFREVOL_H

#include "interactiveobjectitemmodelsurface.h"

class InteractiveSurfaceParabOfRevol;
class InteractiveObjectItemModelSurfaceParabOfRevolPrivate;

class InteractiveObjectItemModelSurfaceParabOfRevol : public InteractiveObjectItemModelSurface
{
public:
    InteractiveObjectItemModelSurfaceParabOfRevol(const Handle(InteractiveSurfaceParabOfRevol) &object, QObject *parent = nullptr);
    ~InteractiveObjectItemModelSurfaceParabOfRevol();

protected:
    void updateSurfaceFields();
    bool setSurfaceFieldData(const QModelIndex &index, const QVariant &value);
    QWidget *surfaceFieldEditor(const QModelIndex &index) const;

private:
    InteractiveObjectItemModelSurfaceParabOfRevolPrivate *d;
};

#endif // INTERACTIVEOBJECTITEMMODELSURFACEPARABOFREVOL_H
