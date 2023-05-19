#include <QApplication>

#include <ExamplesBase/interactiveobjectsmainwindow.h>

#include "viewport.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ExamplesBase::InteractiveObjectsMainWindow w;
    w.setGeometry(600, 360, 1200, 720);
    w.setViewport(new Viewport);
    w.show();
    return a.exec();
}
