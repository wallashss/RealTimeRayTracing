#pragma once

#include <QMainWindow>

#include <clcontextwrapper.h>

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

    ~MainWindow();

private:
    void _testOpenCL();
//     CLContextWrapper *clContext;
};
