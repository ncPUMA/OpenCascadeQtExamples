#include "viewport.h"

class ViewportPrivate
{
    friend class Viewport;
};

Viewport::Viewport(QWidget *parent)
    : ExamplesBase::Viewport(parent)
    , d_ptr(new ViewportPrivate)
{

}

Viewport::~Viewport()
{
    delete d_ptr;
}

void Viewport::objectsChanged()
{

}
