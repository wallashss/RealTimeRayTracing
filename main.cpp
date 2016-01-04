#include "mainwindow.h"
#include <QApplication>

#include <raytracing.h>

#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
