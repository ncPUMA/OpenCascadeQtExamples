#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.setGeometry(600, 360, 1200, 720);
    w.show();
    return a.exec();
}
