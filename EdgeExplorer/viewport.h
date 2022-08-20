#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QWidget>

class ViewPortPrivate;

class ViewPort : public QWidget
{
    Q_OBJECT
public:
    explicit ViewPort(QWidget *parent = nullptr);
    ~ViewPort();

    void fitInView();

public Q_SLOTS:
    void slNormalV1Test();

protected:
    QPaintEngine* paintEngine() const final;
    void paintEvent(QPaintEvent *) final;
    void resizeEvent(QResizeEvent*) final;

    void mousePressEvent(QMouseEvent *event) final;
    void mouseReleaseEvent(QMouseEvent *event) final;
    void mouseMoveEvent(QMouseEvent *event) final;
    void wheelEvent(QWheelEvent *event) final;

private:
    ViewPortPrivate *const d_ptr;

};

#endif // VIEWPORT_H
