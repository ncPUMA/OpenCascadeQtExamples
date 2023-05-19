#include "viewport.h"

class ViewportPrivate
{
    friend class Viewport;
};

Viewport::Viewport()
    : ExamplesBase::InteractiveObjectsViewport()
    , d_ptr(new ViewportPrivate)
{

}

Viewport::~Viewport()
{
    delete d_ptr;
}
