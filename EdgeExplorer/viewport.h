#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <ExamplesBase/Viewport/viewport.h>

class ViewportPrivate;

class Viewport : public ExamplesBase::Viewport
{
    Q_OBJECT
public:
    Viewport(QWidget *parent);
    ~Viewport();

    void debugCurve() const;

public Q_SLOTS:
    void slNormalV1Test();

protected:
    bool selectionChanged() final;
    bool mousePressed(QMouseEvent *event) final;
    bool mouseReleased(QMouseEvent *event) final;
    bool mouseMoved(QMouseEvent *event) final;

private:
    ViewportPrivate *const d_ptr;
};

#endif // VIEWPORT_H
