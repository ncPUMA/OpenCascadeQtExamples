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

protected:
    QPaintEngine* paintEngine() const final;
    void paintEvent(QPaintEvent *) final;
    void resizeEvent(QResizeEvent*) final;

    void mousePressEvent(QMouseEvent *event) final;
    void mouseReleaseEvent(QMouseEvent *event) final;
    void mouseMoveEvent(QMouseEvent *event) final;
    void wheelEvent(QWheelEvent *event) final;

signals:

private:
    ViewPortPrivate *const d_ptr;

};

#endif // VIEWPORT_H
