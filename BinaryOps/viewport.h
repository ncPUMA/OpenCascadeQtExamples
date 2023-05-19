#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <ExamplesBase/Viewport/interactiveobjectsviewport.h>

class ViewportPrivate;

class Viewport : public ExamplesBase::InteractiveObjectsViewport
{
public:
    Viewport();
    ~Viewport();

private:
    ViewportPrivate *const d_ptr;
};

#endif // VIEWPORT_H
