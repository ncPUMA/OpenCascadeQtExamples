#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <ExamplesBase/Viewport/viewport.h>

class QMenu;
class AIS_InteractiveObject;
class gp_XYZ;
class ViewportPrivate;

class Viewport : public ExamplesBase::Viewport
{
public:
    Viewport(QWidget *parent);
    ~Viewport();

protected:
    bool mouseReleased(QMouseEvent *event);

private:
    void menuRequest(const Handle(AIS_InteractiveObject) &object,
                     const gp_XYZ &pickedPoint,
                     QMenu &menu);

private:
    ViewportPrivate *const d;
};

#endif // VIEWPORT_H
