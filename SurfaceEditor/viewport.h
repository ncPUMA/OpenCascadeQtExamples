#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <ExamplesBase/Viewport/interactiveobjectsviewport.h>

class Viewport : public ExamplesBase::InteractiveObjectsViewport
{
public:
    Viewport();

protected:
    void objectsViewChanged(QAbstractItemView *objectsView) final;
    bool mouseReleased(QMouseEvent *event) final;

private:
    bool menuRequest(const QPoint &menuPos,
                     const gp_XYZ &pickedPoint,
                     const Handle(ExamplesBase::InteractiveObject) &object);
};

#endif // VIEWPORT_H
