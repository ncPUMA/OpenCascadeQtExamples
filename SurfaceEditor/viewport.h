#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <ExamplesBase/Viewport/viewport.h>

class QAbstractItemView;
class ViewportPrivate;

class Viewport : public ExamplesBase::Viewport
{
public:
    Viewport(QWidget *parent);
    ~Viewport();

    void setPropertyView(QAbstractItemView *view);

protected:
    bool selectionChanged() final;
    bool mouseReleased(QMouseEvent *event) final;

private:
    ViewportPrivate *const d_ptr;

    friend class ViewportPrivate;
};

#endif // VIEWPORT_H
