#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <ExamplesBase/Viewport/viewport.h>

class QTreeView;
class ViewportPrivate;

class Viewport : public ExamplesBase::Viewport
{
public:
    Viewport(QWidget *parent);
    ~Viewport();

    void setObjectsView(QTreeView *objectsView);

private:
    ViewportPrivate *const d_ptr;
};

#endif // VIEWPORT_H
