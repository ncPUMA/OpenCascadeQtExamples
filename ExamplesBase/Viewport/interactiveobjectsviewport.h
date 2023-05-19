#ifndef INTERACTIVEOBJECTSVIEWPORT_H
#define INTERACTIVEOBJECTSVIEWPORT_H

#include "viewport.h"

class QAbstractItemView;
class gp_XYZ;
class AIS_InteractiveObject;

namespace ExamplesBase {

class InteractiveObject;
class InteractiveObjectsViewportPrivate;

class InteractiveObjectsViewport : public Viewport
{
public:
    InteractiveObjectsViewport(QWidget *parent = nullptr);
    ~InteractiveObjectsViewport();

    void setObjectsView(QAbstractItemView *objectsView);
    void setPropertyView(QAbstractItemView *propertyView);

    void objectsChanged();

    Handle(AIS_InteractiveObject) manipulatorAttachedObject() const;
    void showManipulator(const Handle(AIS_InteractiveObject) &object);
    void removeManipulator();
    Handle(InteractiveObject) editorAttachedObject() const;
    void showEditor(const Handle(InteractiveObject) &object);
    void removeEditor();

    void addToContext(const Handle(InteractiveObject) &object, const gp_XYZ &translation,
                      const QString &name, const Handle(AIS_InteractiveObject) &parent);
    void removeFromContext(const Handle(AIS_InteractiveObject) &object);

protected:
    virtual void objectsViewChanged(QAbstractItemView *objectsView);
    bool selectionChanged() override;

private:
    InteractiveObjectsViewportPrivate *const d_ptr;
};

}

#endif // INTERACTIVEOBJECTSVIEWPORT_H
