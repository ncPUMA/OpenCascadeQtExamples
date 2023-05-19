#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <ExamplesBase/Viewport/viewport.h>

class ViewportPrivate;

class Viewport : public ExamplesBase::Viewport
{
public:
    Viewport(QWidget *parent);
    ~Viewport();

    void objectsChanged();

private:
    ViewportPrivate *const d_ptr;
};

#endif // VIEWPORT_H
