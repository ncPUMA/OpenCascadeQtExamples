/*
    SPDX-FileCopyrightText: 2023 Ilya Pominov <ncpumanc@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EB_VIEWPORT_H
#define EB_VIEWPORT_H

#include "../ExamplesBase_global.h"

#include <QWidget>

#include <Standard_Handle.hxx>

class QMenu;

class AIS_InteractiveContext;
class AIS_InteractiveObject;
class AIS_Shape;
class AIS_Trihedron;
class gp_XYZ;
class V3d_View;

namespace ExamplesBase {

class ViewportPrivate;

class EXAMPLESBASE_EXPORT Viewport : public QWidget
{
public:
    explicit Viewport(QWidget *parent = nullptr);
    ~Viewport();

    bool loadModel(const QString &path,
                   Handle(AIS_Shape) &shape,
                   Handle(AIS_Trihedron) trihedron = nullptr) const;

    void fitInView();

protected:
    Handle(AIS_InteractiveContext) context() const;
    Handle(V3d_View) view() const;
    Standard_Integer depthOffLayer() const;

    virtual bool selectionChanged();
    virtual bool mousePressed(QMouseEvent *event);
    virtual bool mouseReleased(QMouseEvent *event);
    virtual bool mouseMoved(QMouseEvent *event);
    virtual void contextMenuRequest(const Handle(AIS_InteractiveObject) &object,
                                    const gp_XYZ &pickedPoint,
                                    QMenu &menu);

    QPaintEngine* paintEngine() const final;
    void paintEvent(QPaintEvent *) final;
    void resizeEvent(QResizeEvent*) final;

    void mousePressEvent(QMouseEvent *event) final;
    void mouseReleaseEvent(QMouseEvent *event) final;
    void mouseMoveEvent(QMouseEvent *event) final;
    void wheelEvent(QWheelEvent *event) final;

private:
    ViewportPrivate *const d_ptr;

    friend class ViewportPrivate;
};

}

#endif // EB_VIEWPORT_H
