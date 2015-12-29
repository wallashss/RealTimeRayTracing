#include "mainwindow.h"
#include <QApplication>

#include <raytracing.h>

#include <iostream>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

//    int ax = 16;
//    int b = 16;
//    int count = 10;
//    for(int i = 0; i < ax ; i++)
//    {
//        for(int j =0 ; j < b ; j++)
//        {
//            int localIdx = i * b + j;

//            for(int k = 0 ; k < count ; k++)
//            {
//                std::cout << "chunk " << localIdx / count << " ";
//                std::cout << "(" << i << "," << j << ") [" << localIdx << "] " << getIndexToLocal(k, localIdx, count) << std::endl;
//            }
//        }
//    }

    return a.exec();
}
